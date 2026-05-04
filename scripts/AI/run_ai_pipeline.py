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

CAM_HEIGHT = 640
CAM_WIDTH = 640
MODEL_HEIGHT = 640
MODEL_WIDTH = 640
PRINT_OUTPUT_TENSORS_ONCE = True
LANE_CLASS_ID = 0
LANE_CONFIDENCE_THRESHOLD = 0.6
MODEL_FAMILY = "yolov8"
DISPLAY_MASK_ONLY = False
DEBUG_LANE_MASK = False
DEBUG_EVERY_N_FRAMES = 10


def polyfit_lines(tensor):
	tensor = np.asarray(tensor)
	if tensor.ndim == 4:
		if tensor.shape[0] == 0:
			return np.empty((0, 1, 2), dtype=np.int32)
		lane_mask = tensor[0, :, :, 0] > 0
	elif tensor.ndim == 2:
		lane_mask = tensor > 0
	else:
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

		infer_engine = Inference(camera, "/root/perception/lanes/trained_models/yolov8n_seg_2c_100e_16.hef")
		post_processor = PostProcessor(input_size=(MODEL_HEIGHT, MODEL_WIDTH), strides=(8, 16, 32))
		sent_one_frame = False
		for frame, infer_results in infer_engine.run_inference():
			frame_index += 1
			if PRINT_OUTPUT_TENSORS_ONCE and not printed_tensor_info:
				print("=== Model output tensors ===", file=sys.stderr if stream_masks else sys.stdout)
				for out_name, out_tensor in infer_results.items():
					print(f"{out_name}: shape={out_tensor.shape}, dtype={out_tensor.dtype}", file=sys.stderr if stream_masks else sys.stdout)
				print("============================", file=sys.stderr if stream_masks else sys.stdout)
				printed_tensor_info = True
			lane_result = post_processor.decode(
				infer_results,
				quant_params=infer_engine.get_quant_params(),
				conf_th=LANE_CONFIDENCE_THRESHOLD,
				iou_th=0.5,
			)
			# Normalize decoder outputs: `masks` may be a list of 2D arrays.
			masks = lane_result.get("masks", None)
			scores = lane_result.get("scores", None)
			scales = lane_result.get("scales", None)

			if masks is None:
				lane_mask = None
			else:
				# If decoder returned a list of masks, stack/merge them into a single 2D mask.
				if isinstance(masks, list):
					if len(masks) == 0:
						lane_mask = None
					else:
						try:
							lane_mask = np.any(np.stack(masks, axis=0), axis=0).astype(np.uint8)
						except Exception:
							# Fallback: use the first mask element
							lane_mask = np.asarray(masks[0])
				else:
					lane_mask = np.asarray(masks)

			# Pick first score/scale if arrays are returned, else default values
			if scores is None or (hasattr(scores, "size") and scores.size == 0):
				lane_score = 0.0
			else:
				try:
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

			if DEBUG_LANE_MASK:
				if lane_mask is None:
					if frame_index % DEBUG_EVERY_N_FRAMES == 0:
						print(f"[lane_mask] frame={frame_index} mask=None")
				else:
					nonzero_count = int(np.count_nonzero(lane_mask))
					total_count = int(lane_mask.size)
					nonzero_ratio = (nonzero_count / total_count) if total_count > 0 else 0.0
					if frame_index % DEBUG_EVERY_N_FRAMES == 0:
						unique_vals = np.unique(lane_mask)
						print(
							f"[lane_mask] frame={frame_index} nonzero={nonzero_count}/{total_count} "
							f"({nonzero_ratio * 100:.2f}%) unique={unique_vals.tolist()}"
						)
					cv2.putText(
						frame,
						f"Mask nz: {nonzero_count} ({nonzero_ratio * 100:.2f}%)",
						(0, 185),
						cv2.FONT_HERSHEY_SIMPLEX,
						0.75,
						(0, 255, 255) if nonzero_count > 0 else (0, 0, 255),
						2,
					)

			if lane_mask is not None:
				points = polyfit_lines(lane_mask)
				if points.size > 0:
					cv2.polylines(frame, [points], isClosed=False, color=(255, 0, 0), thickness=5)
					cv2.putText(
						frame,
						f"Lane Detected {lane_score:.2f}",
						(0, 150),
						cv2.FONT_HERSHEY_SIMPLEX,
						1,
						(255, 0, 0),
						2,
					)
				else:
					cv2.putText(
						frame,
						"Lane Mask Found",
						(0, 150),
						cv2.FONT_HERSHEY_SIMPLEX,
						1,
						(255, 0, 0),
						2,
					)
			else:
				cv2.putText(frame, "No Lane", (0, 150), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
			cv2.putText(frame, "Inference Active", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

			if DISPLAY_MASK_ONLY:
				ok = post_processor.write_segmentation_mask_to_display(
					camera.display_proc,
					lane_mask,
					base_frame=None,
				)
			else:
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
