import numpy as np


def sigmoid(x):
	return 1.0 / (1.0 + np.exp(-x))


def softmax(x, axis=-1):
	x = x - np.max(x, axis=axis, keepdims=True)
	e = np.exp(x)
	return e / np.sum(e, axis=axis, keepdims=True)


def dequant(tensor, scale, zero_point):
	return (tensor.astype(np.float32) - zero_point) * scale


def nms(boxes, scores, iou_th=0.5):
	if len(boxes) == 0:
		return []

	x1, y1, x2, y2 = boxes.T
	areas = (x2 - x1) * (y2 - y1)
	order = scores.argsort()[::-1]

	keep = []
	while order.size > 0:
		i = order[0]
		keep.append(i)

		xx1 = np.maximum(x1[i], x1[order[1:]])
		yy1 = np.maximum(y1[i], y1[order[1:]])
		xx2 = np.minimum(x2[i], x2[order[1:]])
		yy2 = np.minimum(y2[i], y2[order[1:]])

		w = np.maximum(0, xx2 - xx1)
		h = np.maximum(0, yy2 - yy1)
		inter = w * h
		iou = inter / (areas[i] + areas[order[1:]] - inter + 1e-6)

		order = order[1:][iou < iou_th]

	return keep


class DetectionPostProcessor:
	def __init__(self, input_size=(640, 640), strides=(8, 16, 32)):
		self.input_size = input_size
		self.strides = strides

	def _group_outputs(self, outputs, quant_params):
		grouped = {}
		for name, tensor in outputs.items():
			scale, zp = quant_params[name]
			dq = dequant(tensor, scale, zp)[0]
			h, w, c = dq.shape
			grouped.setdefault((h, w), []).append((name, dq))
		return grouped

	def _split_box_cls(self, tensors):
		tensors = sorted(tensors, key=lambda item: item[1].shape[-1])
		_, cls_tensor = tensors[0]
		_, box_tensor = tensors[-1]
		return box_tensor, cls_tensor

	def decode(self, outputs, quant_params, conf_th=0.3, iou_th=0.5):
		grouped = self._group_outputs(outputs, quant_params)

		all_boxes = []
		all_scores = []
		all_classes = []

		for (h, w), tensors in grouped.items():
			if len(tensors) < 2:
				continue

			box_tensor, cls_tensor = self._split_box_cls(tensors)

			bins = box_tensor.shape[-1] // 4
			if bins <= 0:
				continue

			box = box_tensor.reshape(h, w, 4, bins)
			prob = softmax(box, axis=-1)
			bins_range = np.arange(bins, dtype=np.float32)
			distances = np.sum(prob * bins_range, axis=-1)

			stride = None
			for candidate in self.strides:
				if self.input_size[0] // candidate == h:
					stride = candidate
					break
			if stride is None:
				stride = self.input_size[0] // h

			ys, xs = np.meshgrid(np.arange(h), np.arange(w), indexing="ij")
			cx = (xs + 0.5) * stride
			cy = (ys + 0.5) * stride

			l = distances[..., 0]
			t = distances[..., 1]
			r = distances[..., 2]
			b = distances[..., 3]

			x1 = cx - l * stride
			y1 = cy - t * stride
			x2 = cx + r * stride
			y2 = cy + b * stride

			cls_prob = sigmoid(cls_tensor)
			scores = np.max(cls_prob, axis=-1)
			class_ids = np.argmax(cls_prob, axis=-1)

			mask = scores > conf_th
			if np.count_nonzero(mask) == 0:
				continue

			all_boxes.append(np.stack([x1, y1, x2, y2], axis=-1)[mask])
			all_scores.append(scores[mask])
			all_classes.append(class_ids[mask])

		if len(all_boxes) == 0:
			return {
				"boxes": np.empty((0, 4), dtype=np.float32),
				"scores": np.empty((0,), dtype=np.float32),
				"classes": np.empty((0,), dtype=np.int32),
			}

		boxes = np.concatenate(all_boxes)
		scores = np.concatenate(all_scores)
		classes = np.concatenate(all_classes)

		keep = nms(boxes, scores, iou_th)
		return {
			"boxes": boxes[keep],
			"scores": scores[keep],
			"classes": classes[keep],
		}
