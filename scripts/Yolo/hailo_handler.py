#
# Script to handle AI and camera pipelines
#

import numpy as np
import cv2
from hailo_lib import (apply_sigmoid, apply_threshold, polyfit_lines, Camera, Inference)

CAM_HEIGHT = 640
CAM_WIDTH = 640
MODEL_HEIGHT = 320
MODEL_WIDTH = 320

if __name__ == '__main__':
	camera = None
	try:
		camera = Camera(CAM_HEIGHT, CAM_WIDTH, MODEL_HEIGHT, MODEL_WIDTH)
		infer_engine = Inference(camera)
		for frame, infer_results in infer_engine.run_inference():
			# print("name:", infer_results.keys())
			for name, tensor in infer_results.items():
				if "concat19" in name:
					res_tensor = np.clip(tensor, -10, 10)
					res_tensor = apply_sigmoid(res_tensor)
					print("Min:", res_tensor.min(), "Max:", res_tensor.max())
					if res_tensor.ndim != 4:
						if res_tensor.size == 80 * 80 * 32:
							res_tensor = res_tensor.reshape(1, 80, 80, 32)
						else:
							continue
					res_tensor = apply_threshold(res_tensor, 0.7)
					points = polyfit_lines(res_tensor)
					if points.size > 0:
						cv2.polylines(frame, [points], isClosed=False, color=(0, 255, 0), thickness=5)
						cv2.putText(frame, "Lane Detected", (0, 150), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 0, 0), 2)
					else:
						cv2.putText(frame, "No Lane", (0, 150), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
			cv2.putText(frame, "Inference Active", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
			try:
				camera.display_proc.stdin.write(frame.tobytes())
				camera.display_proc.stdin.flush()
			except BrokenPipeError:
				break
	except Exception as e:
		print(f"FATAL ERROR: {e}")
	finally:
		if camera:
			camera.terminate_camera()
			camera.terminate_display()