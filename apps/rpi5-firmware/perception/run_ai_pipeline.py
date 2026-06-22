#
# Script to handle AI and camera pipelines
#

import cv2
import numpy as np
import argparse
import struct
import sys
from hailo_lib import PostProcessor
from hailo_lib import Inference
from hailo_lib import Camera
from hailo_lib.CameraCarla import CARLACamera
from hailo_lib.NamedPipeWriter import NamedPipeWriter
from detector import build_lane_result, create_trapezoid_roi

CAM_HEIGHT = 640
CAM_WIDTH = 640
MODEL_HEIGHT = 640
MODEL_WIDTH = 640
MODEL_NAME = "yolov8s_seg"
LANE_CLASS_ID = 0
LANE_CONFIDENCE_THRESHOLD = 0.57
MODEL_FAMILY = "yolov8"
DEBUG_EVERY_N_FRAMES = 10
IOU_THRESHOLD = 0.5
ENABLE_POLYFIT = True
ENABLE_LANE_ROI = True
ENABLE_LANE_ROI_VISUALIZATION = True

# Lane Dection Parameters
MIN_LANE_AREA = 500

def _parse_args():
	parser = argparse.ArgumentParser(description="Run AI pipeline with either Pi camera or CARLA camera")
	parser.add_argument(
		"--stream-masks",
		action="store_true",
		help="Write binary mask frames to stdout for consumption by another program",
	)
	parser.add_argument(
		"--named-pipe",
		type=str,
		default=None,
		help="Write lane masks to a named pipe (FIFO) for consumption by another program (e.g., /tmp/lane_mask_pipe)",
	)
	parser.add_argument(
		"--use-carla-camera",
		action="store_true",
		help="Use CARLA TCP camera input instead of Raspberry Pi camera",
	)
	parser.add_argument(
		"--carla-port",
		type=int,
		default=5005,
		help="TCP port used by CARLA camera stream (default: 5005)",
	)
	return parser.parse_args()

if __name__ == '__main__':
	args = _parse_args()
	camera = None
	pipe_writer = None
	printed_tensor_info = False
	frame_index = 0
	stream_masks = getattr(args, "stream_masks", False)
	try:
		if args.use_carla_camera:
			camera = CARLACamera(
				CAM_HEIGHT,
				CAM_WIDTH,
				MODEL_HEIGHT,
				MODEL_WIDTH,
				port=args.carla_port,
			)
			print(f"Using CARLA camera on port {args.carla_port}", file=sys.stderr if stream_masks else sys.stdout)
		else:
			camera = Camera(CAM_HEIGHT, CAM_WIDTH, MODEL_HEIGHT, MODEL_WIDTH)
			print("Using Raspberry Pi camera", file=sys.stderr if stream_masks else sys.stdout)

		# Initialize named pipe writer if requested
		if args.named_pipe:
			pipe_writer = NamedPipeWriter(args.named_pipe)
			pipe_available = pipe_writer.pipe_fd is not None
			if not pipe_available:
				print(f"Warning: Named pipe not available at {args.named_pipe}", file=sys.stderr if stream_masks else sys.stdout)

		infer_engine = Inference(camera, "/root/trained_models/yolov8s_40e.hef")
		post_processor = PostProcessor(input_size=(MODEL_HEIGHT, MODEL_WIDTH), strides=(8, 16, 32), model_name=MODEL_NAME)
		sent_one_frame = False

		for frame, infer_results in infer_engine.run_inference():
			frame_index += 1
			lane_result = post_processor.decode(
				infer_results,
				quant_params=infer_engine.get_quant_params(),
				conf_th=LANE_CONFIDENCE_THRESHOLD,
				iou_th=IOU_THRESHOLD,
			)
			# Normalize decoder outputs: `masks` may be a list of 2D arrays.
			masks = lane_result.get("masks", None)
			scores = lane_result.get("scores", None)
			scales = lane_result.get("scales", None)
			classes = lane_result.get("classes", None)
	
			# Start lane detection post-processing
			lane_mask, lane_instances = build_lane_result(
				masks,
				classes=classes,
				lane_class_id=LANE_CLASS_ID,
				min_area=MIN_LANE_AREA,
				use_roi=ENABLE_LANE_ROI,
			)

			# Pick first score/scale if arrays are returned, else default values
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

			if scales is None or (hasattr(scales, "size") and scales.size == 0):
				lane_scale = None
			else:
				try:
					lane_scale = int(scales[0])
				except Exception:
					lane_scale = int(scales)

			# Write lane mask to named pipe if enabled
			if pipe_writer is not None:
				pipe_writer.write_mask(lane_mask, frame_index, lane_score)
				sent_one_frame = True

			for lane_index, lane_instance in enumerate(lane_instances):
				x1, y1, x2, y2 = lane_instance["bbox"]
				centroid_x, centroid_y = lane_instance["centroid"]
				polyline = lane_instance["polyline"]
				component_score = lane_score

				# Boundary check for text placement
				cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)

				if ENABLE_POLYFIT and polyline.size > 0:
					cv2.polylines(frame, [polyline], isClosed=False, color=(0, 255, 255), thickness=3)
				
				cv2.putText(
					frame,
					f"Lane {lane_index + 1} x={int(centroid_x)}",
					(x1, max(0, y1 - 8)),
					cv2.FONT_HERSHEY_SIMPLEX,
					0.6,
					(0, 255, 255),
					2,
				)

			if ENABLE_LANE_ROI_VISUALIZATION:
				_, roi_polygon = create_trapezoid_roi(frame.shape)
				overlay = frame.copy()
				cv2.fillPoly(overlay, roi_polygon, (0, 0, 255))
				frame = cv2.addWeighted(overlay, 0.18, frame, 0.82, 0)
			
			ok = post_processor.write_segmentation_mask_to_display(
				camera.display_proc,
				lane_mask,
				base_frame=frame,
			)

			if not ok:
				break
	except Exception as e:
		print(f"FATAL ERROR: {e}", file=sys.stderr if stream_masks else sys.stdout)
	finally:
		if pipe_writer is not None:
			pipe_writer.cleanup()
		if camera:
			camera.terminate_camera()
			camera.terminate_display()
