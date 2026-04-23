#
# Script to handle AI and camera pipelines
#

import cv2
import numpy as np
import argparse
import json
from pathlib import Path
from hailo_lib import Camera, Inference
from hailo_lib.CameraCarla import CARLACamera
from hailo_lib.yoloPostProcessor import inference_result_handler

CAM_HEIGHT = 640
CAM_WIDTH = 640
MODEL_HEIGHT = 640
MODEL_WIDTH = 640
PRINT_OUTPUT_TENSORS_ONCE = True
MODEL_TYPE = "v8"
CONFIG_PATH = Path(__file__).resolve().parent / "config.json"


def _load_config(config_path):
	with open(config_path, "r", encoding="utf-8") as f:
		return json.load(f)


def _default_labels(config_data, model_type):
	num_classes = int(config_data[model_type]["classes"])
	return [f"class_{i}" for i in range(num_classes)]


def _write_display_frame(camera, frame):
	if hasattr(camera, "write_frame_to_pipe"):
		return camera.write_frame_to_pipe(frame)

	if hasattr(camera, "display_proc") and camera.display_proc and camera.display_proc.stdin:
		try:
			camera.display_proc.stdin.write(frame.tobytes())
			camera.display_proc.stdin.flush()
			return True
		except BrokenPipeError:
			return False

	return False

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
	config_data = _load_config(CONFIG_PATH)
	labels = _default_labels(config_data, MODEL_TYPE)
	try:
		if args.use_carla_camera:
			print("Initializing CARLA camera...")
			camera = CARLACamera(
				CAM_HEIGHT,
				CAM_WIDTH,
				MODEL_HEIGHT,
				MODEL_WIDTH,
				port=args.carla_port,
			)
			print(f"Using CARLA camera on port {args.carla_port}")
		else:
			print("Initializing Raspberry Pi camera...")
			camera = Camera(CAM_HEIGHT, CAM_WIDTH, MODEL_HEIGHT, MODEL_WIDTH)
			print("Using Raspberry Pi camera")
		infer_engine = Inference(camera, "/root/trained_models/yolov8n_seg.hef")
		for frame, infer_results in infer_engine.run_inference():
			if PRINT_OUTPUT_TENSORS_ONCE and not printed_tensor_info:
				print("=== Model output tensors ===")
				for out_name, out_tensor in infer_results.items():
					print(f"{out_name}: shape={out_tensor.shape}, dtype={out_tensor.dtype}")
				print("============================")
				printed_tensor_info = True

			annotated_frame = inference_result_handler(
				frame=np.array(frame, copy=True),
				infer_results=infer_results,
				config_data=config_data,
				model_type=MODEL_TYPE,
				labels=labels,
			)
			cv2.putText(annotated_frame, "Inference Active", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

			ok = _write_display_frame(camera, annotated_frame)

			if not ok:
				break
	except Exception as e:
		print(f"FATAL ERROR: {e}")
	finally:
		if camera:
			camera.terminate_camera()
			camera.terminate_display()


