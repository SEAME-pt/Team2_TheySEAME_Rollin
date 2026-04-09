import cv2
import hailo_platform.pyhailort.pyhailort as hailo
import numpy as np
import subprocess
import sys

# Load Hailo model
print(dir(hailo))
hef = hailo.HEF("../yolo_tusimple.hef")
network_group = hailo.ConfiguredNetwork(hef)

# Open camera
# cap = cv2.VideoCapture(0)

cmd = ["rpicam-vid", "-t", "0", "--inline", "--save-pts", "-", "--output", "-"]

try:
     proc = subprocess.Popen(cmd, stdout=subprocess.PIPE)
except Exception as e:
    print(f"Error occurred while starting subprocess: {e}")
    sys.exit(1)

while True:
	frame_bytes = sys.stdin.buffer.read(320 * 320 * 3)  # Adjust size as needed
	if not frame_bytes:
		break
	frame = np.frombuffer(frame_bytes, dtype=np.uint8).reshape((320, 320, 3))	
	# Preprocess frame (resize, normalize, etc.)
	input_tensor = cv2.resize(frame, (320, 320))
	input_tensor = input_tensor.astype(np.float32) / 255.0	
	# Run inference
	outputs = network_group.activate([input_tensor])	
	# Post-process outputs (e.g., draw bounding boxes)
	# ...
	# with open("/dev/tty1", "w") as f:
		# f.write(outputs[0].tobytes())