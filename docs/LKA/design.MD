# Lane Perception - Design

## Overview
The Lane Perception module uses a YOLO-based segmentation model trained on labeled data to detect lane markings and runs inference on Hailo hardware.

## System Architecture

### Input
- Video stream from a front-facing camera

### Data Pipeline (Training Phase)

1. Data Collection
   - Capture road images under different conditions

2. Data Labeling
   - Annotate lane markings using segmentation masks

3. Dataset Preparation
   - Split into training, validation, and test sets

4. Model Training
   - Train YOLO segmentation model on labeled dataset

5. Model Export
   - Convert trained model to Hailo-compatible format

---

### Inference Pipeline (Runtime)

1. Frame Acquisition
   - Capture frame from camera

2. Preprocessing
   - Resize and normalize image

3. Model Inference
   - Run YOLO segmentation model on Hailo accelerator

4. Post-processing
   - Extract lane regions from segmentation mask
   - Convert mask to lane boundaries (optional)

5. Output
   - Segmentation mask
   - Lane visualization overlay

---

## Output
- Pixel-wise lane segmentation mask
- Optional lane boundary lines
- Visualization output

---

## Design Decisions

### Choice of Model
YOLO-based segmentation was selected due to:
- Real-time performance capabilities
- Compatibility with edge deployment
- Strong performance in object and region detection

### Hardware Acceleration
Hailo hardware is used to:
- Reduce inference latency
- Enable real-time processing on edge devices

### Trade-offs
- Requires large labeled dataset
- Training and optimization complexity
- Dependency on hardware-specific toolchain

---

## Future Improvements
- Improve dataset diversity (weather, night driving)
- Add temporal smoothing across frames
- Explore transformer-based models for segmentation