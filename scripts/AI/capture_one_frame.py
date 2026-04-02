import cv2
import numpy as np
import os

# -------------------- CONFIG --------------------
CAM_PIPELINE = "gst-launch-1.0 libcamerasrc ! video/x-raw,width=640,height=480,format=RGB ! videoconvert ! video/x-raw,format=BGR ! appsink"
MODEL_WIDTH = 320
MODEL_HEIGHT = 320
BIN_PATH = "bin/model_output.bin"   # Hailo inference output (simulated)
OUTPUT_PATH = "results/overlay_result.png"
OUTPUT_PATH_RAW = "results/overlay_result_raw.png"
NUM_CLASSES = 2  # 0=background, 1=object
# ------------------------------------------------

# Capture a frame
cap = cv2.VideoCapture(CAM_PIPELINE, cv2.CAP_GSTREAMER)
ret, frame = cap.read()
cap.release()

if not ret:
    raise RuntimeError("Failed to capture frame")

# Resize frame to model input
frame_resized = cv2.resize(frame, (MODEL_WIDTH, MODEL_HEIGHT))

# ------------------- Load segmentation mask -------------------
if not os.path.exists(BIN_PATH):
    # For testing, generate random mask with 0=background, 1=object
    seg_mask = np.random.randint(0, NUM_CLASSES, (MODEL_HEIGHT, MODEL_WIDTH), dtype=np.uint8)
else:
    # Load binary file
    seg_mask = np.fromfile(BIN_PATH, dtype=np.uint8)
    seg_mask = seg_mask.reshape((MODEL_HEIGHT, MODEL_WIDTH))

mask_vis = (seg_mask * (255 // (NUM_CLASSES - 1))).astype(np.uint8)
os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)
print("Unique mask values:", np.unique(seg_mask))
cv2.imwrite(OUTPUT_PATH_RAW, mask_vis)

# ------------------- Create color overlay -------------------
# Only assign color to the object class; leave background transparent
colors = {
    1: (0, 255, 0),  # class 1 -> green
}

overlay = np.zeros_like(frame_resized, dtype=np.uint8)
for cls, color in colors.items():
    overlay[seg_mask == cls] = color

# Blend overlay only where mask != 0
alpha = 0.5
mask_bool = seg_mask != 0
result = frame_resized.copy()
result[mask_bool] = cv2.addWeighted(frame_resized, 1 - alpha, overlay, alpha, 0)[mask_bool]

# ------------------- Save result -------------------
cv2.imwrite(OUTPUT_PATH, result)
print(f"Overlay saved to {OUTPUT_PATH}")
