# Comparison between UFLDv2, LaneATT, and YOLOv11-seg
This study aims to compare three different models that suite Lane Detection.

![Image](LaneDetection/comparison_between_three_models.png)

## YOLOv11 Segmentation Model
- This is a general-purpose segmentation model, not designed with lane topology in mind. It provides extensive Hailo ecosystem support from the Hailo Model Zoo, with pre-compiled HEF binary files.
- It comes in many sizes (n, s, m, l, x): n - nano, for edge devices at max speed, to x - extra large, with slowest speed but max accuracy.
- According to Model Zoo, v11n achieves roughly 185FPS for lane detection tasks at 640x640. The segmentation variants run somewhat slower due to mask decoding overhead — expect roughly 160 FPS (nano) down to ~35 FPS (medium). Vales may be lower since the FPS values recorded in Model Zoo benchmarks are recorded on a four-lane PCIe interface.
- The segmentation overlay can become a bottleneck when running YOLOv8/v11 segmentation on a Raspberry Pi 5 with Hailo, and optimization with techniques like Numba and LUT can help.
- Allows to run other tasks (object detection, depth estimation) on the same HEF file (cascading)

## LaneATT
- LaneATT is an anchor-based detector that uses attention to generate anchors along lines, then regresses the deviation of detected lanes from those anchors — a design purpose-built for structured lane geometry. 
- LaneATT is available as a lane-detection-specific HEF for Hailo-8 (primarily ResNet-18 and ResNet-34 backbone variants), referenced in Hailo community examples.
- LaneATT with a ResNet-18 backbone is significantly lighter computationally and should push well above 200 FPS on Hailo-8 hardware, though exact figures require profiling with hailomz.

## Ultra-Fast Lane Detection v2
- Similar to LaneATT in terms of structure.
- Comes with a Hybrid Architecture combining both row and column anchors. This is what separates this model from LaneATT.
- While not officially supported, a ufld_v2.hef is confirmed being used on Hailo-8, paired alonside Yolov8m for Object Detection tasks.
- UFLDv2 achieves F1 scores of 96.11% on TuSimple and 75.0% on CULane with ResNet-18, and 96.24% / 76.0% with ResNet-34.
- UFLDv2's output is a set of class probability distributions over row/column positions — decoding is simply an argmax operation with no NMS step, giving it one of the lightest postprocessing footprints of the three.
- 

## UFLD vs LaneATT
LaneATT uses a global attention mechanism — each anchor "attends" to the entire feature map before predicting, which helps it handle occlusions and long-range lane continuity. UFLDv2 uses a classification approach: it divides the image into a grid of row and column anchors
and predicts the most likely x-position of the lane in each row (or y-position in each column). There's no cross-anchor attention; each row/column anchor is decided independently.


## Summary
LaneATT is the better-specialized tool for dedicated lane detection at high throughput with low CPU overhead. YOLOv11-seg shines when you need to combine lane segmentation with other perception tasks in a single model, or when you want the flexibility of re-training on your own data with Ultralytics tooling.
UFLDv2 is similar to LaneATT in throughput and CPU cost, is slightly better documented on TuSimple, and has a hybrid anchor design that gives advantage in curved roads compared to LaneATT.

