#
# Script to handle AI and camera pipelines
#

import cv2
import numpy as np
import argparse
from hailo_lib import PostProcessor, polyfit_lines, Camera, Inference
from hailo_lib.CameraCarla import CARLACamera

CAM_HEIGHT = 640
CAM_WIDTH = 640
MODEL_HEIGHT = 640
MODEL_WIDTH = 640
PRINT_OUTPUT_TENSORS_ONCE = True
LANE_CLASS_ID = 0
LANE_CONFIDENCE_THRESHOLD = 0.3
MODEL_FAMILY = "yolov8"
DISPLAY_MASK_ONLY = False
DEBUG_LANE_MASK = False
DEBUG_EVERY_N_FRAMES = 10


def _parse_args():
	parser = argparse.ArgumentParser(description="Run AI pipeline with either Pi camera or CARLA camera")
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
	printed_tensor_info = False
	frame_index = 0
	try:
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
		infer_engine = Inference(camera, "/root/trained_models/yolov8n_seg_100e.hef")
		post_processor = PostProcessor(input_size=(MODEL_HEIGHT, MODEL_WIDTH), model_family=MODEL_FAMILY)
		for frame, infer_results in infer_engine.run_inference():
			frame_index += 1
			if PRINT_OUTPUT_TENSORS_ONCE and not printed_tensor_info:
				print("=== Model output tensors ===")
				for out_name, out_tensor in infer_results.items():
					print(f"{out_name}: shape={out_tensor.shape}, dtype={out_tensor.dtype}")
				print("============================")
				printed_tensor_info = True
			lane_result = post_processor.process(
				infer_results,
				mode=PostProcessor.MODE_LANE_SEGMENTATION,
				lane_class_id=LANE_CLASS_ID,
				confidence_threshold=LANE_CONFIDENCE_THRESHOLD,
			)
			lane_mask = lane_result["mask"]
			lane_score = lane_result["score"]
			lane_scale = lane_result["scale"]

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
					cv2.polylines(frame, [points], isClosed=False, color=(0, 255, 0), thickness=5)
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
		print(f"FATAL ERROR: {e}")
	finally:
		if camera:
			camera.terminate_camera()
			camera.terminate_display()


