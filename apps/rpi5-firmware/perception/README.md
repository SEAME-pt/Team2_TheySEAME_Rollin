# Perception Pipeline

This directory contains the AI perception script that consumes Hailo segmentation outputs and turns them into lane-level detections for the rest of the RPi5 firmware.

## Pipeline Overview

The processing flow is:

1. Read a camera frame from either the Raspberry Pi camera or CARLA.
2. Run the frame through the Hailo inference pipeline.
3. Decode the raw model outputs in `PostProcessor.decode()`.
4. Filter detections by confidence.
5. Apply class-aware NMS.
6. Rebuild segmentation masks from the prototype tensor and mask coefficients.
7. Merge lane masks.
8. Split the merged lane mask into connected components.
9. Build one lane instance per component.
10. Polyfit each lane component and draw or stream the result.

The main entry point is [run_ai_pipeline.py](run_ai_pipeline.py), the decoder lives in [hailo_lib/PostProcessor.py](hailo_lib/PostProcessor.py), and the lane helpers live in [detector/lane_detector.py](detector/lane_detector.py).

## How Post Processing Works

### 1. Box and class decoding

`PostProcessor.decode()` dequantizes the Hailo output tensors, then decodes the YOLOv8 box distribution into bounding boxes using DFL-style expectation over bins.

For each detection location it computes:

- `scores`: the maximum class confidence
- `class_ids`: the winning class per location
- `boxes`: decoded `x1, y1, x2, y2`
- `coeffs`: the mask coefficients used later to reconstruct the segmentation mask

Only detections with `score > conf_th` are kept.

### 2. NMS

After confidence filtering, the decoder applies NMS.

The current behavior is class-aware:

- each class is processed separately
- lane detections use a looser IoU threshold than the rest of the classes
- non-lane detections still use the standard IoU threshold

This matters because lane predictions often produce several nearby proposals for the same physical lane. If NMS is too aggressive, the surviving proposal can flip from frame to frame, which looks like flicker.

### 3. Mask reconstruction

For each surviving box, the decoder reconstructs a mask by combining:

- the prototype tensor
- that detection's mask coefficients

The reconstructed mask is:

1. passed through a sigmoid
2. resized to the model input size
3. binarized with a hard threshold
4. cropped to the predicted box

The result is a per-detection binary mask.

## Lane-Specific Handling

The lane-processing path in [run_ai_pipeline.py](run_ai_pipeline.py) delegates lane-specific mask handling to [detector/lane_detector.py](detector/lane_detector.py) and does not treat every raw detection as a separate lane.

It does this instead:

1. Select masks whose class is `LANE_CLASS_ID`.
2. Merge those masks into a single lane mask with `np.any(...)`.
3. Run connected-component analysis on the merged mask.
4. Create one explicit lane instance per connected component.
5. Sort instances left-to-right using centroid x.
6. Polyfit each instance independently.
7. Draw one bbox per component.

The explicit lane-instance structure contains:

- `bbox`
- `mask`
- `polyline`
- `centroid`
- `area`

That structure is better than using raw detection boxes directly, because raw boxes are often duplicated across scales and overlap heavily.

## Flickering Problem

The flicker came from suppression, not from the model alone.

What was happening:

- the model produced several overlapping lane proposals
- NMS removed some of them
- a different proposal survived on the next frame
- the chosen mask changed shape or disappeared
- polyfitting then reacted to that change

So the visible output looked unstable even though the raw segmentation was mostly consistent.

Removing NMS made the flicker better, but FPS dropped because many more proposals survived and every one of them had to be decoded and processed.

## Solutions Found

We tried a few approaches and converged on the current one:

### 1. Remove NMS entirely

This stabilized the lane output, but it was too expensive and reduced FPS a lot.

### 2. Keep NMS but make it less aggressive for lanes

This is the current choice.

The decoder now uses a higher lane IoU threshold so overlapping lane proposals are not collapsed as aggressively as other classes.

### 3. Merge lane masks and split by connected component

This is the lane-instance step currently used downstream.

It gives you one bbox and one polyline per physical lane candidate, instead of one per raw detector proposal.

## Practical Rule For Downstream Code

If you need the lane to stay stable over time, do not use a raw detection box as the lane identity.

Use the explicit lane-instance output instead:

- pick the connected component mask
- use its bbox for drawing
- use its centroid x to order left vs right lanes
- polyfit the component mask, not the raw detector box

If you need a single lane, choose the instance with the largest area or the centroid closest to your expected lane position.

If you need multiple lanes, keep the list sorted left-to-right and polyfit each component independently.

## Dual lane + TSR pipeline

`run_dual_pipeline.py` runs lane segmentation and TSR detection from **one camera** and **one Hailo VDevice** (two HEFs, sequential inference per frame).

See **[DUAL_PIPELINE.md](DUAL_PIPELINE.md)** for setup, CLI options, pipe formats, and troubleshooting.

## Relevant Files

- [run_ai_pipeline.py](run_ai_pipeline.py)
- [run_dual_pipeline.py](run_dual_pipeline.py)
- [DUAL_PIPELINE.md](DUAL_PIPELINE.md)
- [hailo_lib/PostProcessor.py](hailo_lib/PostProcessor.py)
- [hailo_lib/DualInference.py](hailo_lib/DualInference.py)
- [../README.md](../README.md)
