"""
YOLOv8 Segmentation Post-Processor for Hailo-8
=============================================

Description:
------------
This module implements a post-processing pipeline for YOLOv8 segmentation
models executed on Hailo-8 hardware. It decodes the raw outputs produced
by the HEF model inference (via hailort) into meaningful detections,
including bounding boxes, class predictions, confidence scores, and
segmentation masks.

Features:
---------
- Dequantization of Hailo output tensors
- Distribution Focal Loss (DFL) bounding box decoding
- Confidence thresholding and Non-Maximum Suppression (NMS)
- Segmentation mask reconstruction using prototype masks
- Optional rendering and streaming of segmentation overlays

Inputs:
-------
- outputs (dict):
	Dictionary of tensors returned by `HEF.infer()`
- quant_params (dict):
	Dictionary mapping tensor names to (scale, zero_point)

Outputs:
--------
Dictionary containing:
- boxes   : ndarray of shape (N, 4)
- scores  : ndarray of shape (N,)
- classes : ndarray of shape (N,)
- scales  : ndarray of shape (N,)
- masks   : list of segmentation masks
- mask    : first mask (for convenience)

Author: Joao Barbot

Date: <2026-05-04>

"""

import numpy as np
import cv2


# ---------------------------
# Utils
# ---------------------------

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


def class_aware_nms(boxes, scores, classes, iou_th=0.5):
		if len(boxes) == 0:
				return []

		keep_indices = []
		for class_id in np.unique(classes):
				class_indices = np.where(classes == class_id)[0]
				class_keep = nms(boxes[class_indices], scores[class_indices], iou_th)
				keep_indices.extend(class_indices[class_keep].tolist())

		if len(keep_indices) == 0:
				return []

		keep_indices = np.asarray(keep_indices, dtype=np.int32)
		order = scores[keep_indices].argsort()[::-1]
		return keep_indices[order].tolist()

# ---------------------------
# Main Decoder
# ---------------------------

class PostProcessor:

		def __init__(self, input_size=(640, 640), strides=(8, 16, 32), model_name="self.model_name"):
				self.input_size = input_size
				self.strides = strides
				self.model_name = model_name

		def decode(self, outputs, quant_params, conf_th=0.3, iou_th=0.5):

				# ---------------------------
				# Dequantize all tensors
				# ---------------------------
				def dq(name):
						scale, zp = quant_params[name]
						return dequant(outputs[name], scale, zp)[0]
				proto = dq(f"{self.model_name}/conv48")
				scales = [
						(f"{self.model_name}/conv44", f"{self.model_name}/conv45", f"{self.model_name}/conv46", 8),
						(f"{self.model_name}/conv60", f"{self.model_name}/conv61", f"{self.model_name}/conv62", 16),
						(f"{self.model_name}/conv73", f"{self.model_name}/conv74", f"{self.model_name}/conv75", 32),
				]
				print(scales)
				all_boxes = []
				all_scores = []
				all_classes = []
				all_coeffs = []
				all_scales = []

				for box_name, cls_name, coeff_name, stride in scales:

						boxes = dq(box_name)
						classes = dq(cls_name)
						coeffs = dq(coeff_name)

						H, W, _ = classes.shape

						# ---- decode bbox (DFL)
						# boxes shape: (H, W, 4 * bins) where bins is typically 16. Each set of 4 values corresponds to the DFL distribution for l, t, r, b.
						bins = boxes.shape[-1] // 4
						# Reshape to (H, W, 4, bins) for easier processing
						boxes = boxes.reshape(H, W, 4, bins)

						# Softmax to get probabilities, then compute expected values for distances
						prob = softmax(boxes, axis=-1)
						bins_range = np.arange(bins, dtype=np.float32)

						distances = np.sum(prob * bins_range, axis=-1)

						# ---- grid
						ys, xs = np.meshgrid(np.arange(H), np.arange(W), indexing="ij")

						cx = (xs + 0.5) * stride
						cy = (ys + 0.5) * stride

						l, t, r, b = distances[..., 0], distances[..., 1], distances[..., 2], distances[..., 3]

						x1 = cx - l * stride
						y1 = cy - t * stride
						x2 = cx + r * stride
						y2 = cy + b * stride

						# ---- class scores
						cls_prob = sigmoid(classes)
						scores = np.max(cls_prob, axis=-1)
						class_ids = np.argmax(cls_prob, axis=-1)

						mask = scores > conf_th

						all_boxes.append(np.stack([x1, y1, x2, y2], axis=-1)[mask])
						all_scores.append(scores[mask])
						all_classes.append(class_ids[mask])
						all_coeffs.append(coeffs[mask])
						all_scales.extend([stride] * int(np.count_nonzero(mask)))

				if len(all_boxes) == 0:
						return {
								"boxes": np.empty((0, 4), dtype=np.float32),
								"score": 0.0,
								"scale": None,
								"classes": np.empty((0,), dtype=np.int32),
								"mask": None,
								"masks": [],
						}

				boxes = np.concatenate(all_boxes)
				scores = np.concatenate(all_scores)
				classes = np.concatenate(all_classes)
				coeffs = np.concatenate(all_coeffs)
				scales = np.asarray(all_scales, dtype=np.int32)

				# ---------------------------
				# NMS
				# ---------------------------
				keep = class_aware_nms(boxes, scores, classes, iou_th)

				boxes = boxes[keep]
				scores = scores[keep]
				classes = classes[keep]
				coeffs = coeffs[keep]
				scales = scales[keep]

				# ---------------------------
				# Masks
				# ---------------------------
				proto = proto.astype(np.float32)

				masks = []
				for i in range(len(boxes)):
						m = np.tensordot(proto, coeffs[i], axes=([2], [0]))
						m = sigmoid(m)

						m = cv2.resize(m, self.input_size[::-1])
						m = (m > 0.5).astype(np.uint8)

						# crop to box
						x1, y1, x2, y2 = boxes[i].astype(int)
						cropped = np.zeros_like(m)
						cropped[y1:y2, x1:x2] = m[y1:y2, x1:x2]

						masks.append(cropped)

				return {
						"boxes": boxes,
						"scores": scores,
						"scales": scales,
						"classes": classes,
						"mask": masks[0] if masks else None,
						"masks": masks,
				}

		def render_segmentation_mask(self, mask, base_frame=None, color=(0, 255, 0), alpha=0.8):
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
				return cv2.addWeighted(base, alpha, overlay, alpha, 0)

		def write_segmentation_mask_to_display(self, display_proc, mask, base_frame, color=(0, 255, 0), alpha=0.65):
				if display_proc is None or getattr(display_proc, "stdin", None) is None:
						return False

				frame_to_write = self.render_segmentation_mask(mask, base_frame=base_frame, color=color, alpha=alpha)
				try:
						display_proc.stdin.write(frame_to_write.tobytes())
						display_proc.stdin.flush()
						return True
				except BrokenPipeError:
						return False
