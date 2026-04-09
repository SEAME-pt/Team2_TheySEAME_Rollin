##  
##  This script uses gstream to pipe the  Rpicam library output to waylandsink 
##

import os
import subprocess
import numpy as np
import cv2

hef = hailo.HEF("../yolo_tusimple.hef")
network_group = hailo.ConfiguredNetwork(hef)

# --- 2. Setup Camera and Display Pipe ---

# Get Wayland environment variables from the current system
# If running as root, you may need to manually set these to "/run/user/1000" and "wayland-0"
env = os.environ.copy()
rpicam_cmd = [
    "rpicam-vid", "-t", "0", "--width", "640", "--height", "640",
    "--nopreview", "--codec", "yuv420", "-o", "-"
]
cam_proc = subprocess.Popen(rpicam_cmd, stdout=subprocess.PIPE, bufsize=10**7)
# Updated display command using rawvideoparse for better stability
gst_display_cmd = (
	"gst-launch-1.0 fdsrc ! "
	"rawvideoparse format=bgr width=640 height=640 framerate=30/1 ! "
	"videoconvert ! waylandsink"
)

# Launch display with environmental context
display_proc = subprocess.Popen(
    gst_display_cmd, 
    stdin=subprocess.PIPE, 
    env=env, 
    shell=True
)

try:
	while True:
		raw_frame = cam_proc.stdout.read(640 * 640 * 3 // 2)
		if not raw_frame:
			break
		
		yuv = np.frombuffer(raw_frame, dtype=np.uint8).reshape((640 * 3 // 2, 640))
		frame = cv2.cvtColor(yuv, cv2.COLOR_YUV2BGR_I420)
		
		# Preprocess frame (resize, normalize, etc.)
		input_tensor = cv2.resize(frame, (320, 320))
		input_tensor = input_tensor.astype(np.float32) / 255.0
		
		# --- 3. Inference --- (Keep your existing logic here)
		outputs = network_group.activate([input_tensor])	

		# --- 4. Visualize ---
		cv2.putText(frame, "Inference Active", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
		# Send processed frame to GStreamer Wayland sink
		try:
			display_proc.stdin.write(frame.tobytes())
			display_proc.stdin.flush()
		except BrokenPipeError:
		    break

except KeyboardInterrupt:
    pass
finally:
    cam_proc.terminate()
    # Properly close the pipe before terminating to avoid "failed to write" errors
    if display_proc.stdin:
        display_proc.stdin.close()
    display_proc.terminate()
