import cv2 as cv
import sys
import subprocess
import numpy as np

# Replace with your model input size
MODEL_WIDTH = 320
MODEL_HEIGHT = 320
MODEL_CHANNELS = 3  # usually 3 for RGB

# Hailortcli command
cmd = [
    "hailortcli",
    "run", "/root/yolo_tusimple.hef",
]

proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

cap = cv.VideoCapture("gst-launch-1.0 libcamerasrc ! video/x-raw,width=640,height=480,format=RGB ! videoconvert ! video/x-raw,format=BGR ! appsink", cv.CAP_GSTREAMER)
while (1):
    ret, frame = cap.read()
    if not ret:
        print("Can't receive frame (stream end?). Exiting ...")
        break
    frame_resized = cv.resize(frame, (MODEL_WIDTH, MODEL_HEIGHT))
    frame_rgb = cv.cvtColor(frame_resized, cv.COLOR_BGR2RGB)
    frame_bytes = frame_rgb.astype(np.uint8).tobytes()
    try:
        proc.stdin.write(frame_bytes)
        proc.stdin.flush()
    except BrokenPipeError:
        print("Hailortcli closed the pipe. Exiting.")
        break
    output_line = proc.stdout.readline()
    print("Model output:", output_line.decode().strip())
cap.release()
proc.stdin.close()
proc.terminate()
