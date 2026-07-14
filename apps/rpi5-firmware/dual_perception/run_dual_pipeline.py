#
# Unified lane segmentation + TSR detection on one camera and one Hailo VDevice.
#

import argparse
import os
import sys

import cv2
import numpy as np

from hailo_lib import Camera
from hailo_lib.CameraCarla import CARLACamera
from hailo_lib.DualInference import DualInference
from hailo_lib.PostProcessor import PostProcessor as SegmentationPostProcessor
from hailo_lib.DetectionPostProcessor import DetectionPostProcessor
from hailo_lib.NamedPipeWriter import NamedPipeWriter
from hailo_lib.NamedPipeDetWriter import NamedPipeDetWriter

MODEL_FAMILY = "yolov8n_seg"
CAM_HEIGHT = 640
CAM_WIDTH = 640
MODEL_HEIGHT = 640
MODEL_WIDTH = 640
LANE_CLASS_ID = 0
DEFAULT_MODELS_DIR = os.path.join(os.path.dirname(__file__), "trained_models")


def _default_hef(name):
	return os.path.join(DEFAULT_MODELS_DIR, name)


def load_labels(path):
	if not path or not os.path.exists(path):
		return []
	with open(path, "r", encoding="utf-8") as handle:
		return [line.strip() for line in handle if line.strip()]


def merge_lane_masks(masks):
	if masks is None:
		return None
	if isinstance(masks, list):
		if len(masks) == 0:
			return None
		try:
			return np.any(np.stack(masks, axis=0), axis=0).astype(np.uint8)
		except Exception:
			return np.asarray(masks[0])
	return np.asarray(masks)


def detections_to_pipe_payload(boxes, scores, classes):
	dets = []
	for box, score, cls_id in zip(boxes, scores, classes):
		x1, y1, x2, y2 = box.astype(int)
		w = max(0, x2 - x1)
		h = max(0, y2 - y1)
		dets.append((int(cls_id), float(score), x1, y1, w, h))
	return dets


def write_frame(display_proc, frame):
	if display_proc is None or getattr(display_proc, "stdin", None) is None:
		return False
	try:
		display_proc.stdin.write(frame.tobytes())
		display_proc.stdin.flush()
		return True
	except BrokenPipeError:
		return False


def draw_overlay(frame, lane_mask, dets, labels, lane_score):
	cv2.putText(
		display,
		f"lane={lane_score:.2f} dets={len(dets)}",
		(8, 24),
		cv2.FONT_HERSHEY_SIMPLEX,
		0.6,
		(255, 255, 255),
		2,
	)
	return display


def _parse_args():
	parser = argparse.ArgumentParser(
		description="Run lane segmentation and TSR detection from one camera on one Hailo device",
	)
	parser.add_argument(
		"--seg-hef",
		default=_default_hef("yolov8s_seg.hef"),
		help="Lane segmentation HEF path",
	)
	parser.add_argument(
		"--det-hef",
		default=_default_hef("../tsr/models/yolov8s.hef"),
		help="TSR detection HEF path",
	)
	parser.add_argument(
		"--seg-model-name",
		default="yolov8s_seg",
		help="Segmentation model name prefix used by the decoder",
	)
	parser.add_argument(
		"--labels",
		default=os.path.normpath(os.path.join(os.path.dirname(__file__), "../tsr/models/labels.txt")),
		help="TSR labels.txt path",
	)
	parser.add_argument(
		"--lane-pipe",
		default=None,
		help="Named pipe for lane masks (e.g. /tmp/lane_mask_pipe)",
	)
	parser.add_argument(
		"--tsr-pipe",
		default="/root/tsr/NamedPipeTsr",
		help="Named pipe for TSR detections",
	)
	parser.add_argument("--lane-conf", type=float, default=0.57, help="Lane confidence threshold")
	parser.add_argument("--det-conf", type=float, default=0.35, help="Detection confidence threshold")
	parser.add_argument("--lane-iou", type=float, default=0.5, help="Lane NMS IoU threshold")
	parser.add_argument("--det-iou", type=float, default=0.5, help="Detection NMS IoU threshold")
	parser.add_argument(
		"--use-carla-camera",
		action="store_true",
		help="Use CARLA TCP camera input instead of Raspberry Pi camera",
	)
	parser.add_argument("--carla-port", type=int, default=5005, help="CARLA camera TCP port")
	parser.add_argument("--no-display", action="store_true", help="Disable Wayland display output")
	parser.add_argument("--debug-every", type=int, default=30, help="Print status every N frames")
	return parser.parse_args()

def draw_lanes(display, lane_mask):
	if lane_mask is not None and lane_mask.size > 0:
		mask_bin = (lane_mask > 0).astype(np.uint8)
		if mask_bin.shape[:2] != display.shape[:2]:
			mask_bin = cv2.resize(
				mask_bin,
				(display.shape[1], display.shape[0]),
				interpolation=cv2.INTER_NEAREST,
            )
	overlay = np.zeros_like(display)
	overlay[mask_bin > 0] = (0, 255, 0)
	display = cv2.addWeighted(display, 0.65, overlay, 0.35, 0)
	return display

def lane_segmentation(seg_post, seg_results, infer_engine, args, frame):
	lane_result = seg_post.decode(
		seg_results,
		quant_params=infer_engine.get_seg_quant_params(),
		conf_th=args.lane_conf,
		iou_th=args.lane_iou,
	)
	lane_mask = merge_lane_masks(lane_result.get("masks"))
	scores = lane_result.get("scores", [])
	classes = lane_result.get("classes", [])
	if scores is None or (hasattr(scores, "size") and scores.size == 0):
		lane_score = 0.0
	else:
		try:
			if classes is not None and len(scores) > 0:
				lane_scores = np.asarray(scores)[np.asarray(classes) == LANE_CLASS_ID]
				lane_score = float(np.max(lane_scores)) if lane_scores.size > 0 else float(scores[0])
			else:
				lane_score = float(scores[0])
		except Exception:
			lane_score = float(scores)
	return draw_lanes(frame, lane_mask)

def draw_detections(display, dets, labels):
	for cls_id, score, x, y, w, h in dets:
		x2, y2 = x + w, y + h
		cv2.rectangle(display, (x, y), (x2, y2), (0, 140, 255), 2)
		label = labels[cls_id] if 0 <= cls_id < len(labels) else str(cls_id)
		cv2.putText(
			display,
			f"{label} {score:.2f}",
			(x, max(y - 6, 12)),
			cv2.FONT_HERSHEY_SIMPLEX,
			0.5,
			(0, 140, 255),
			1,
			cv2.LINE_AA,
		)
	return display

def detections(det_post, det_results, infer_engine, args, frame, labels):
	det_result = det_post.decode(
		det_results,
		quant_params=infer_engine.get_det_quant_params(),
		conf_th=args.det_conf,
		iou_th=args.det_iou,
    )
	dets = detections_to_pipe_payload(
		det_result.get("boxes", []),
		det_result.get("scores", []),
		det_result.get("classes", []),
	)
	if tsr_pipe is not None:
		tsr_pipe.write_detections(dets)
	return draw_detections(frame, dets, labels)

if __name__ == "__main__":
	args = _parse_args()
	camera = None
	lane_pipe = None
	tsr_pipe = None
	frame_index = 0

	try:
		labels = load_labels(args.labels)
		if args.use_carla_camera:
			camera = CARLACamera(
				CAM_HEIGHT,
				CAM_WIDTH,
				MODEL_HEIGHT,
				MODEL_WIDTH,
				port=args.carla_port,
			)
			print(f"Using CARLA camera on port {args.carla_port}")
		else:
			camera = Camera(CAM_HEIGHT, CAM_WIDTH, MODEL_HEIGHT, MODEL_WIDTH)
			print("Using Raspberry Pi camera")

		if args.no_display and camera.display_proc is not None:
			camera.terminate_display()
			camera.display_proc = None

		if args.tsr_pipe:
			tsr_pipe = NamedPipeDetWriter(args.tsr_pipe)
			if tsr_pipe.pipe_fd is None:
				print(f"Warning: TSR pipe unavailable at {args.tsr_pipe}")

		infer_engine = DualInference(camera, args.seg_hef, args.det_hef)
        # Lane Post Processor
		seg_post = SegmentationPostProcessor(
			input_size=(MODEL_HEIGHT, MODEL_WIDTH),
			strides=(8, 16, 32),
			model_name=MODEL_FAMILY,
		)
        # TSR Post Processor
		det_post = DetectionPostProcessor(input_size=(MODEL_HEIGHT, MODEL_WIDTH))

		lane_seg_b = True
		det_b = False
		print(f"Lane HEF: {args.seg_hef}")
		print(f"TSR HEF:  {args.det_hef}")

		for frame, seg_results, det_results in infer_engine.run_inference():
			frame_index += 1
			display = frame.copy()

			if lane_seg_b == True:
				display = lane_segmentation(seg_post, seg_results, infer_engine, args, display)
			if det_b == True:
				display = detections(det_post, det_results, infer_engine, args, display, labels)

            # Debug
			#if frame_index % args.debug_every == 0:
			#	if not dets:
			#		print(f"frame={frame_index} lane={lane_score:.2f} det=none", flush=True)
			#	else:
			#		cls_id, score, x, y, w, h = dets[0]
			#		label = labels[cls_id] if 0 <= cls_id < len(labels) else str(cls_id)
			#		print(
			#			f"frame={frame_index} lane={lane_score:.2f} "
			#			f"det={label} {score:.2f} box=({x},{y},{w},{h})",
			#			flush=True,
			#		)

			if not args.no_display:
				if not write_frame(camera.display_proc, display):
					break

	except Exception as exc:
		print(f"FATAL ERROR: {exc}", file=sys.stderr)
		raise
	finally:
		if tsr_pipe is not None:
			tsr_pipe.close()
		if camera is not None:
			camera.terminate_camera()
			if getattr(camera, "display_proc", None) is not None:
				camera.terminate_display()
