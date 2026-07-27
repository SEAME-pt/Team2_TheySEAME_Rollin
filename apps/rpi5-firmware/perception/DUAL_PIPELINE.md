# Dual pipeline (`run_dual_pipeline.py`)

Run **lane segmentation** and **traffic-sign / object detection (TSR)** from a single camera and a single Hailo device.

This replaces running two separate `run_ai_pipeline.py` processes, which can conflict over the Hailo accelerator (`HAILO_OUT_OF_PHYSICAL_DEVICES`).

## What it does

Each frame:

1. Read one image from the Pi camera (or CARLA) at **640×640**.
2. Run **lane segmentation** (`--seg-hef`) on Hailo.
3. Run **TSR detection** (`--det-hef`) on the **same frame** and **same `VDevice`**.
4. Post-process both outputs.
5. Optionally write to named pipes and/or show a combined display.

```text
Camera (640×640)
      │
      ▼
 DualInference (one VDevice)
      │
      ├── seg HEF → lane masks + score
      └── det HEF → sign/car boxes + classes
      │
      ├── --lane-pipe  → LKA firmware (optional)
      ├── --tsr-pipe   → Tsr C++ reader
      └── display      → green lane mask + orange boxes
```

Inference is **sequential** (seg then det), not two parallel Hailo processes. Expect lower FPS than a single model, but stable sharing of the chip.

## Requirements

- Raspberry Pi 5 with Hailo (tested on Hailo-8).
- Two compiled HEF files:
  - Lane: e.g. `trained_models/yolov8s_seg.hef` or `yolov8s_40e.hef`
  - TSR: e.g. `../tsr/models/yolov8s.hef`
- TSR labels: `../tsr/models/labels.txt`
- Python deps: `opencv-python`, `numpy`, `hailo_platform` (same as the single pipelines).

Run from `apps/rpi5-firmware/perception/` (or set paths explicitly).

## Quick start (Raspberry Pi)

### 1. Create the TSR named pipe

```bash
rm -f /root/tsr/NamedPipeTsr
mkfifo /root/tsr/NamedPipeTsr
chmod 666 /root/tsr/NamedPipeTsr
```

### 2. Start the dual pipeline (display)

```bash
cd /path/to/perception
XDG_RUNTIME_DIR=/run/user/200 WAYLAND_DISPLAY=wayland-1 \
python3 run_dual_pipeline.py \
  --seg-hef /root/new_perception/trained_models/yolov8s_40e.hef \
  --det-hef /root/tsr/models/yolov8s.hef \
  --labels /root/tsr/models/labels.txt \
  --seg-model-name yolov8s_seg \
  --tsr-pipe /root/tsr/NamedPipeTsr
```

### 3. Start the TSR C++ reader (separate terminal)

```bash
cd /root/tsr
./Tsr
```

You should see `NamedPipeTsr opened successfully` and **no** `Sync Problem` if the pipe writer uses big-endian format (`NamedPipeDetWriter.py` in this repo).

### Headless (no display)

Add `--no-display` and drop the `XDG_RUNTIME_DIR` / `WAYLAND_DISPLAY` variables.

## Full system (lanes + TSR downstream)

To feed **both** LKA and TSR firmware, create both pipes and pass both flags:

```bash
# Example: lane pipe for LKA (path must match your firmware)
mkfifo /tmp/lane_mask_pipe && chmod 666 /tmp/lane_mask_pipe

python3 run_dual_pipeline.py \
  --seg-hef trained_models/yolov8s_seg.hef \
  --det-hef ../tsr/models/yolov8s.hef \
  --lane-pipe /tmp/lane_mask_pipe \
  --tsr-pipe /root/tsr/NamedPipeTsr \
  --no-display
```

Start the corresponding C++ readers **before** or **after** the Python process; the Python side opens pipes in read/write mode so it does not block on an empty FIFO.

| Output | Flag | Default | Consumer |
|--------|------|---------|----------|
| Lane mask | `--lane-pipe` | off (`None`) | LKA / `mainLka.cpp` |
| TSR detections | `--tsr-pipe` | `/root/tsr/NamedPipeTsr` | `Tsr` / `mainTsr.cpp` |

If you omit `--lane-pipe`, lane segmentation still runs (for display and internal scoring) but **no lane FIFO** is written.

## Command-line options

| Option | Default | Description |
|--------|---------|-------------|
| `--seg-hef` | `trained_models/yolov8s_seg.hef` | Lane segmentation HEF |
| `--det-hef` | `../tsr/models/yolov8s.hef` | TSR detection HEF |
| `--seg-model-name` | `yolov8s_seg` | Decoder prefix inside the seg HEF (must match compile) |
| `--labels` | `../tsr/models/labels.txt` | TSR class names (order must match training) |
| `--lane-pipe` | off | FIFO path for lane masks |
| `--tsr-pipe` | `/root/tsr/NamedPipeTsr` | FIFO path for TSR detections |
| `--lane-conf` | `0.57` | Lane detection confidence threshold |
| `--det-conf` | `0.35` | TSR confidence threshold |
| `--lane-iou` | `0.5` | Lane NMS IoU |
| `--det-iou` | `0.5` | TSR NMS IoU |
| `--no-display` | off | Disable Wayland/GStreamer preview |
| `--use-carla-camera` | off | Use CARLA TCP camera instead of Pi camera |
| `--carla-port` | `5005` | CARLA camera port |
| `--debug-every` | `30` | Log status every N frames |

## Named pipe formats

### TSR pipe (`NamedPipeDetWriter`)

Binary messages, **big-endian** (`>IHHfIIII`), compatible with `Tsr` / `mainTsr.cpp`:

| Field | Type |
|-------|------|
| frameNbr | uint32 (always **77000**) |
| numDetections | uint16 |
| trafficSign | uint16 (class id) |
| accuracy | float32 |
| x, y, width, height | uint32 each |

One message per detection; empty frames send `numDetections = 0`.

### Lane pipe (`NamedPipeWriter`, optional)

| Field | Type |
|-------|------|
| frame number | uint32 |
| height, width | uint16 each |
| lane score | float32 |
| mask pixels | uint8 × height × width |

See [hailo_lib/NamedPipeWriter.py](hailo_lib/NamedPipeWriter.py) for details.

## How the code is organized

| File | Role |
|------|------|
| [run_dual_pipeline.py](run_dual_pipeline.py) | CLI, main loop, display overlay |
| [hailo_lib/DualInference.py](hailo_lib/DualInference.py) | One `VDevice`, configure both HEFs, infer per frame |
| [hailo_lib/PostProcessor.py](hailo_lib/PostProcessor.py) | YOLOv8 **segmentation** decode (lanes) |
| [hailo_lib/DetectionPostProcessor.py](hailo_lib/DetectionPostProcessor.py) | YOLOv8 **detection** decode (TSR) |
| [hailo_lib/NamedPipeDetWriter.py](hailo_lib/NamedPipeDetWriter.py) | TSR FIFO writer |
| [hailo_lib/NamedPipeWriter.py](hailo_lib/NamedPipeWriter.py) | Lane FIFO writer |

## Display

With display enabled (default on Pi):

- **Green overlay** — merged lane segmentation mask
- **Orange boxes** — TSR detections with label and score
- **Top-left text** — `lane=<score> dets=<count>`

Resolution is **640×640** (repo `Camera.py`). The standalone LKA pipeline on the Pi may use a wider split UI; this script uses a single combined view.

## Limitations

- **Two HEF files**, not one merged HEF. Recompiling as a single multi-network HEF is a separate Hailo `join()` workflow.
- **Sequential inference** — FPS is roughly between one model and half of two parallel (ideal) runs.
- **Do not** run this alongside another Hailo-owning pipeline (old `/root/tsr` + `/root/new_perception` scripts at the same time).
- **Seg model name** must match the HEF (`yolov8s_seg` vs `yolov8n_seg`); wrong name → empty or broken lane masks.
- **HEF target**: if models were compiled for `hailo8l` on a `hailo8` module, Hailo logs a performance warning.
- **LKA BEV UI** from `run_ai_pipeline.py --enable-lka` is not included here; use `--lane-pipe` + LKA firmware or extend this script.

## Troubleshooting

| Symptom | Likely cause |
|---------|----------------|
| `HAILO_OUT_OF_PHYSICAL_DEVICES` | Another Python/Hailo process still running |
| `Sync Problem (got …, expected 77000)` | Pipe endianness mismatch; use repo `NamedPipeDetWriter` (`>IHHfIIII`) |
| `failed to write output bytes` | Display closed; use `--no-display` or keep Weston running |
| No lane mask | Wrong `--seg-model-name` or low `--lane-conf` |
| No TSR boxes | Low `--det-conf` or wrong `--det-hef` / labels |
| `Warning: pipe unavailable` | FIFO missing; create with `mkfifo` first |

## Related docs

- [perception/README.md](README.md) — lane segmentation post-processing
- [docs/TSR/pipeline-runbook.md](../../docs/TSR/pipeline-runbook.md) — single TSR pipeline SSH commands
- [docs/TSR/README.md](../../docs/TSR/README.md) — TSR model and documentation index
