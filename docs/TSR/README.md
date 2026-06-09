# Traffic Sign Recognition (TSR)

Documentation and model artifacts for the TSR perception pipeline on Raspberry Pi / Hailo.

## Model artifacts (in repo)

| Path | Description |
|------|-------------|
| `apps/rpi5-firmware/tsr/models/yolov8s.hef` | Hailo-compiled YOLOv8s detection model (13 classes) |
| `apps/rpi5-firmware/tsr/models/labels.txt` | Class labels for the compiled model |

Source: local `seame-lane-objects` training bundle (`hailo_artifacts/`).

## Documentation

| Document | Contents |
|----------|----------|
| [pipeline-runbook.md](pipeline-runbook.md) | SSH commands to start/stop/check the TSR pipeline on the Pi |
| [hailo-detect-compilation.md](hailo-detect-compilation.md) | ONNX → HEF compilation workflow for the detect model |
| [dataset-seame-lane-objects.md](dataset-seame-lane-objects.md) | Roboflow export notes for the lane/objects dataset |
| [dataset-obstacle-tuning.md](dataset-obstacle-tuning.md) | Obstacle tuning dataset metadata |
| [dataset-obstacle-tuning-roboflow.md](dataset-obstacle-tuning-roboflow.md) | Roboflow export notes for obstacle tuning |

## Related remote branches

- `feature/452-tsr-post-processing` — distance estimation and TSR C++ post-processing
- `feature/446-create-comunication-for-taffic-signs` — Kuksa / cluster traffic-sign communication

This branch (`feature/traffic-sign-recognition`) adds the compiled model and docs on top of `develop`.
