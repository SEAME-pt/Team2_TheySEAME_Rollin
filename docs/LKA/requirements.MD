# Lane Perception - Features

## Overview
The Lane Perception module detects lane markings using a deep learning segmentation model trained on labeled image data and deployed on Hailo hardware.

## Functional Requirements

### Input
- The system shall receive input from a front-facing camera.
- The input shall be a continuous stream of image frames.

### Lane Detection
- The system shall detect lane regions using a segmentation model.
- The system shall classify pixels belonging to lane markings.
- The system shall support detection of straight and curved lanes.

### Output
- The system shall output segmentation masks for lane markings.
- The system shall provide lane boundary extraction from segmentation results.
- The system shall support overlay visualization on the original image.

### Model Integration
- The system shall use a YOLO-based segmentation model.
- The model shall be trained using labeled image datasets.
- The model shall be compatible with Hailo hardware acceleration.

### Performance
- The system shall operate in real time on Hailo hardware.
- The system shall maintain low inference latency.

### Robustness
- The system shall handle different lighting conditions.
- The system shall tolerate partial occlusions and worn lane markings.
