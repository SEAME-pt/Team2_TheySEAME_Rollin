import cv2
import numpy as np


def apply_sigmoid(tensor):
	tensor = tensor.astype(np.float32)
	# Numerically stable sigmoid to avoid overflow for large |tensor|.
	return np.where(
		tensor >= 0,
		1.0 / (1.0 + np.exp(-tensor)),
		np.exp(tensor) / (1.0 + np.exp(tensor)),
	)

def apply_threshold(tensor, threshold):
	tensor = (tensor > threshold).astype(np.float32)
	return tensor

def _to_float_tensor(tensor):
	tensor = np.asarray(tensor)
	if tensor.dtype == np.uint8:
		return (tensor.astype(np.float32) - 128.0) / 128.0
	return tensor.astype(np.float32)

def _sigmoid(x):
	x = x.astype(np.float32)
	return np.where(x >= 0, 1.0 / (1.0 + np.exp(-x)), np.exp(x) / (1.0 + np.exp(x)))

def _softmax(x, axis=-1):
	x = x - np.max(x, axis=axis, keepdims=True)
	exp_x = np.exp(x)
	return exp_x / np.sum(exp_x, axis=axis, keepdims=True)


class BaseSegmentationAdapter:
	"""Adapter interface for different YOLO segmentation output layouts."""

	def select_outputs(self, infer_results):
		raise NotImplementedError


class YoloV8LikeSegmentationAdapter(BaseSegmentationAdapter):
	"""
	Heuristic adapter for YOLOv8/v11/v26 segmentation heads.
	Expected per-scale outputs: boxes, classes, coeffs and one prototype tensor.
	"""

	def __init__(self, proto_channels=32):
		self.proto_channels = proto_channels

	def _find_proto(self, tensors):
		for tensor in tensors:
			_, h, w, c = tensor.shape
			if c == self.proto_channels and h == w and h >= 80:
				return tensor
		return None

	def select_outputs(self, infer_results):
		tensors = [np.asarray(t) for t in infer_results.values() if np.asarray(t).ndim == 4]
		if not tensors:
			return None, {}

		proto = self._find_proto(tensors)
		if proto is None:
			return None, {}

		head_groups = {}
		for tensor in tensors:
			if tensor is proto:
				continue
			_, h, w, c = tensor.shape
			spatial_key = (h, w)
			head_groups.setdefault(spatial_key, []).append(tensor)

		head_by_scale = {}
		for spatial_key, group in head_groups.items():
			if len(group) < 3:
				continue

			coeff_candidate = None
			boxes_candidate = None
			classes_candidate = None

			for tensor in group:
				channels = tensor.shape[-1]
				if channels == self.proto_channels and coeff_candidate is None:
					coeff_candidate = tensor

			for tensor in group:
				if tensor is coeff_candidate:
					continue
				channels = tensor.shape[-1]
				if channels % 4 == 0 and channels > 4:
					if boxes_candidate is None or channels > boxes_candidate.shape[-1]:
						boxes_candidate = tensor

			for tensor in group:
				if tensor is coeff_candidate or tensor is boxes_candidate:
					continue
				if classes_candidate is None:
					classes_candidate = tensor

			if coeff_candidate is None or boxes_candidate is None or classes_candidate is None:
				continue

			head_by_scale[spatial_key] = {
				"boxes": boxes_candidate,
				"classes": classes_candidate,
				"coeffs": coeff_candidate,
			}

		return proto, head_by_scale


class PostProcessor:
	"""Unified post-processing API with mode and model-family selection."""

	MODE_LANE_SEGMENTATION = "lane_segmentation"

	def __init__(self, input_size=(640, 640), model_family="yolov8"):
		self.input_size = input_size
		self.adapters = {}
		self.register_adapter("yolov8", YoloV8LikeSegmentationAdapter())
		self.register_adapter("yolov11", YoloV8LikeSegmentationAdapter())
		self.register_adapter("yolov26", YoloV8LikeSegmentationAdapter())
		self.set_model_family(model_family)

	def register_adapter(self, model_family, adapter):
		self.adapters[model_family] = adapter

	def set_model_family(self, model_family):
		if model_family not in self.adapters:
			raise ValueError(f"Unsupported model family: {model_family}")
		self.model_family = model_family
		self.adapter = self.adapters[model_family]

	def process(self, infer_results, mode=MODE_LANE_SEGMENTATION, lane_class_id=None, confidence_threshold=0.35):
		if mode != self.MODE_LANE_SEGMENTATION:
			raise ValueError(f"Unsupported post-processing mode: {mode}")
		return self._decode_lane_segmentation(
			infer_results,
			lane_class_id=lane_class_id,
			confidence_threshold=confidence_threshold,
		)

	def _decode_lane_segmentation(self, infer_results, lane_class_id=None, confidence_threshold=0.35):
		proto, heads = self.adapter.select_outputs(infer_results)
		if proto is None or not heads:
			return {
				"mode": self.MODE_LANE_SEGMENTATION,
				"mask": None,
				"score": None,
				"scale": None,
				"class_ids": None,
				"class_scores": None,
			}

		selected_candidate = None
		selected_score = confidence_threshold
		selected_scale = None
		selected_class_scores = None
		selected_class_ids = None

		for scale_key in sorted(heads.keys(), reverse=True):
			scale_heads = heads[scale_key]
			if not {"boxes", "classes", "coeffs"}.issubset(scale_heads):
				continue

			boxes = _to_float_tensor(scale_heads["boxes"])[0]
			classes = _to_float_tensor(scale_heads["classes"])[0]
			coeffs = _to_float_tensor(scale_heads["coeffs"])[0]
			height, width = classes.shape[:2]
			stride_y = self.input_size[0] / float(height)
			stride_x = self.input_size[1] / float(width)
			bin_count = boxes.shape[-1] // 4
			if bin_count == 0:
				continue
			bin_values = np.arange(bin_count, dtype=np.float32)

			class_probabilities = _sigmoid(classes)
			if lane_class_id is None:
				class_scores = np.max(class_probabilities, axis=-1)
			else:
				if lane_class_id >= class_probabilities.shape[-1]:
					continue
				class_scores = class_probabilities[:, :, lane_class_id]

			best_flat_index = int(np.argmax(class_scores))
			best_y, best_x = np.unravel_index(best_flat_index, class_scores.shape)
			best_score = float(class_scores[best_y, best_x])

			if best_score < selected_score:
				continue

			box_distribution = boxes[best_y, best_x].reshape(4, bin_count)
			box_probabilities = _softmax(box_distribution, axis=-1)
			distances = np.sum(box_probabilities * bin_values[None, :], axis=-1)
			left, top, right, bottom = distances

			center_x = (best_x + 0.5) * stride_x
			center_y = (best_y + 0.5) * stride_y
			box = np.array([
				center_x - left * stride_x,
				center_y - top * stride_y,
				center_x + right * stride_x,
				center_y + bottom * stride_y,
			], dtype=np.float32)

			selected_candidate = {
				"box": box,
				"coeffs": coeffs[best_y, best_x],
				"score": best_score,
				"scale": scale_key,
			}
			selected_score = best_score
			selected_scale = scale_key
			selected_class_scores = class_scores
			selected_class_ids = np.argmax(class_probabilities, axis=-1).astype(np.uint8)

		if selected_candidate is None:
			return {
				"mode": self.MODE_LANE_SEGMENTATION,
				"mask": None,
				"score": None,
				"scale": None,
				"class_ids": None,
				"class_scores": None,
			}

		mask_logits = np.tensordot(_to_float_tensor(proto)[0], selected_candidate["coeffs"], axes=([2], [0]))
		mask_prob = _sigmoid(mask_logits)
		mask_prob = cv2.resize(mask_prob, (self.input_size[1], self.input_size[0]), interpolation=cv2.INTER_LINEAR)
		mask = (mask_prob > 0.5).astype(np.uint8)

		box = selected_candidate["box"]
		box[0] = np.clip(box[0], 0, self.input_size[1] - 1)
		box[1] = np.clip(box[1], 0, self.input_size[0] - 1)
		box[2] = np.clip(box[2], 0, self.input_size[1] - 1)
		box[3] = np.clip(box[3], 0, self.input_size[0] - 1)

		x1, y1, x2, y2 = box.astype(np.int32)
		print(x1, x2, y1, y2)
		if x2 > x1 and y2 > y1:
			cropped_mask = np.zeros_like(mask)
			cropped_mask[y1:y2 + 1, x1:x2 + 1] = mask[y1:y2 + 1, x1:x2 + 1]
		else:
			cropped_mask = mask

		class_score_map = cv2.resize(
			selected_class_scores.astype(np.float32),
			(self.input_size[1], self.input_size[0]),
			interpolation=cv2.INTER_LINEAR,
		)
		class_id_map = cv2.resize(
			selected_class_ids,
			(self.input_size[1], self.input_size[0]),
			interpolation=cv2.INTER_NEAREST,
		)

		return {
			"mode": self.MODE_LANE_SEGMENTATION,
			"mask": cropped_mask,
			"score": float(selected_candidate["score"]),
			"scale": selected_scale,
			"class_ids": class_id_map,
			"class_scores": class_score_map,
		}

	def render_segmentation_mask(self, mask, base_frame=None, color=(0, 255, 0), alpha=0.65):
		if mask is None:
			if base_frame is None:
				return np.zeros((self.input_size[0], self.input_size[1], 3), dtype=np.uint8)
			return base_frame

		if base_frame is None:
			base = np.zeros((self.input_size[0], self.input_size[1], 3), dtype=np.uint8)
		else:
			base = base_frame.copy()

		binary = (mask > 0).astype(np.uint8)
		overlay = np.zeros_like(base)
		overlay[binary > 0] = color
		return cv2.addWeighted(base, 1.0, overlay, alpha, 0)

	def write_segmentation_mask_to_display(self, display_proc, mask, base_frame=None, color=(0, 255, 0), alpha=0.65):
		if display_proc is None or display_proc.stdin is None:
			return False

		frame_to_write = self.render_segmentation_mask(mask, base_frame=base_frame, color=color, alpha=alpha)
		try:
			display_proc.stdin.write(frame_to_write.tobytes())
			display_proc.stdin.flush()
			return True
		except BrokenPipeError:
			return False

def polyfit_lines(tensor):
	tensor = np.asarray(tensor)
	if tensor.ndim == 4:
		if tensor.shape[0] == 0:
			return np.empty((0, 1, 2), dtype=np.int32)
		lane_mask = tensor[0, :, :, 0] > 0
	elif tensor.ndim == 2:
		lane_mask = tensor > 0
	else:
		print("Unexpected tensor shape for lane mask:", tensor.shape)
		return np.empty((0, 1, 2), dtype=np.int32)

	y_coords, x_coords = np.where(lane_mask)
	if x_coords.size < 3:
		return np.empty((0, 1, 2), dtype=np.int32)

	coefficients = np.polyfit(y_coords, x_coords, 2)
	y_values = np.linspace(0, lane_mask.shape[0] - 1, 100)
	x_values = coefficients[0] * y_values**2 + coefficients[1] * y_values + coefficients[2]
	x_values = np.clip(x_values, 0, lane_mask.shape[1] - 1)

	points = np.column_stack((x_values, y_values)).astype(np.int32)
	return points.reshape(-1, 1, 2)

def select_yolov8_seg_outputs(infer_results):
	return YoloV8LikeSegmentationAdapter().select_outputs(infer_results)

def decode_lane_mask(infer_results, lane_class_id=None, confidence_threshold=0.35, input_size=(640, 640)):
	post_processor = PostProcessor(input_size=input_size, model_family="yolov8")
	result = post_processor.process(
		infer_results,
		mode=PostProcessor.MODE_LANE_SEGMENTATION,
		lane_class_id=lane_class_id,
		confidence_threshold=confidence_threshold,
	)
	return result["mask"], result["score"], result["scale"]

