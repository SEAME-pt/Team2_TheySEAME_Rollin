# YOLO Detect (`best.pt`) to Hailo HEF

This folder contains prepared inputs for Hailo compilation:
- `best.onnx` (exported from your trained model)
- `calib_set/` (200 representative images)
- `labels.txt` (13 classes)
- `compile_hailo_detect.sh` (compile command template)

## 1) Copy artifacts to your Hailo host

```bash
scp -r hailo_artifacts <user>@<host>:/path/to/shared_with_docker/
```

Or copy bundle:

```bash
scp hailo_artifacts_bundle.tar.gz <user>@<host>:/path/to/
```

## 2) On Hailo host, enter SW Suite environment

Use your normal Hailo startup flow (Docker/container) until `hailomz` is available.

## 3) Compile to HEF (detection flow)

```bash
cd /path/to/shared_with_docker/hailo_artifacts
./compile_hailo_detect.sh
```

If your hardware is not Pi AI Kit, edit `HW_ARCH` in script:
- `hailo8l` = Pi AI Kit
- `hailo8` = Hailo-8 module

## 4) Deploy to Raspberry Pi

Copy generated `.hef` and `labels.txt` to Pi and run your Hailo camera app.

```bash
scp <generated>.hef pi@<pi-ip>:/home/pi/models/
scp labels.txt pi@<pi-ip>:/home/pi/models/
```

## Notes
- This model is **YOLO detect**. For TSR deployment use **`yolov8s`** (the compiled artifact in this repo). Do not use segmentation variants (`yolov8s_seg`, `yolov8n_seg`).
- Keep class count as `13`.
- If one-step `hailomz compile` is unsupported in your version, use the parse/optimize/compile commands in `compile_hailo_detect.sh`.
