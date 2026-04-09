import sys
import cv2
import numpy as np

# ===== Dummy model inference placeholder =====
# Replace this with your Hailo model inference
def run_yolo_seg(frame):
    """
    frame: HxWx3 BGR numpy array
    Returns:
        list of detections, each detection is a dict:
        {
            'box': [x1, y1, x2, y2],
            'mask': HxW binary mask (0/1),
            'class_id': int,
            'score': float
        }
    """
    H, W = frame.shape[:2]
    # Example: one fake mask in the middle
    mask = np.zeros((H, W), dtype=np.uint8)
    mask[H//4:H//4*3, W//4:W//4*3] = 1
    return [{'box':[W//4, H//4, W//4*3, H//4*3], 'mask':mask, 'class_id':0, 'score':0.9}]

# ===== Camera setup =====
cap = cv2.VideoCapture(0)  # your libcamerasrc setup works as well

while True:
    ret, frame = cap.read()
    if not ret:
        break

    detections = run_yolo_seg(frame)

    # ===== Overlay masks =====
    overlay = frame.copy()
    for det in detections:
        mask = det['mask']
        color = (0, 255, 0)  # green mask
        overlay[mask.astype(bool)] = (0.5*overlay[mask.astype(bool)] + 0.5*np.array(color)).astype(np.uint8)
        x1, y1, x2, y2 = det['box']
        cv2.rectangle(overlay, (x1, y1), (x2, y2), color, 2)
        cv2.putText(overlay, f"class {det['class_id']} {det['score']:.2f}", 
                    (x1, y1-5), cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 1)

    # ===== Encode frame as JPEG =====
    ret, buf = cv2.imencode('.jpg', overlay)
    if not ret:
        continue

    # ===== Write to stdout =====
    sys.stdout.buffer.write(buf.tobytes())
    sys.stdout.flush()
