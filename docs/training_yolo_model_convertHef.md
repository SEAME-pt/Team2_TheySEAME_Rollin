# YOLO-Hailo model conversion

**TuSimple → YOLO Training → ONNX → Hailo `.hef`**
---

## Overview

This document describes the full pipeline for training a YOLO segmentation model on the TuSimple lane detection dataset and deploying it on Hailo hardware.

---

## 1. TuSimple to YOLO Conversion

### 1.1 Why Conversion is Needed

TuSimple dataset:
- Video clips with JSON annotations
- Not compatible with YOLO format

**TuSimple structure:**
- `clips/` → video frames
- `label_data_*.json` → lane annotations
- `seg_label/` → segmentation masks (unused)

**YOLO expects:**
- `images/train/`, `images/val/`
- `labels/train/`, `labels/val/`
- `data.yaml`

Script `convert_tusimple.py` does this. Just download TuSimple from [Kaggle](https://www.kaggle.com/datasets/manideep1108/tusimple), place it on `datasets/` (repo root), and name it `TuSimple`. Then run `python3 convert_tusimple.py` and install the required dependencies on a conda environment (optional).

**Output structure:**
```
convertedYolo/
  images/
    train/
    val/
  labels/
    train/
    val/
  data.yaml
```

---

### 1.2 Train / Validation Split Strategy

- No validation set provided → must create one
- Split must be done **by clip**, not frame

**Reason:** Avoid data leakage from similar frames.

| Split | Source | Labels |
|------|--------|--------|
| train | 80% of clips | Yes |
| val | 20% of clips | Yes |
| test | TuSimple test set | No |

---

### 1.3 Label Format Conversion

- Combine `lanes` (x) with `h_samples` (y)
- Skip points where `x == -2`
- Normalize coordinates (1280 × 864)

**YOLO segmentation format:**
```
<class_id> x1 y1 x2 y2 ... xn yn
```

- `class_id = 0` (single class: lane)
- Skip lanes with < 2 points

---

## 2. YOLO Model Training

---

### 2.1 Training Script

```python
from ultralytics import YOLO

model = YOLO('yolo11n-seg.pt')
model.train(
    data='../../../datasets/convertedYolo/data.yaml',
    epochs=100,
    imgsz=640,
    batch=16
)
```

Run:
```bash
python3 train.py
```

---
---

### 2.3 Training Output

```
runs/segment/train/
  weights/
    best.pt
    last.pt
  results.csv
  confusion_matrix.png
```

Use **`best.pt`** for export.

---

## 3. Export to ONNX

### 3.1 Why ONNX?

- Hailo doesn't support `.pt`
- Requires ONNX intermediate format

---

### 3.2 Export Command

Add this line to the train.py:

```
success = model.export(format='onnx', imgsz=640, opset=11)
```
Note: Ensure you use opset=11 or higher, and it is recommended to use the best.pt file for best performance.

## 4. Hailo Compilation to `.hef`

### 4.1 Prerequisites

```bash
pip install hailo_dataflow_compiler-*.whl
```

Download from:
https://developer.hailo.ai

---

### 4.2 Step 1 — Parse ONNX

```python
from hailo_sdk_client import ClientRunner

runner = ClientRunner(hw_arch='hailo8')
runner.translate_onnx_model(
    'yolo11n-seg.onnx',
    'yolo_tusimple',
    start_node_names=['images'],
    end_node_names=[
        '/model.23/Concat',
        '/model.23/proto/cv3/act/Mul',
        '/model.23/Sigmoid',
        '/model.23/Concat_2'
    ],
    net_input_shapes={'images': [1, 3, 640, 640]}
)
runner.save_har('yolo_tusimple.har')
```

---

### 4.3 Step 2 — Quantization Calibration

```python
import numpy as np
import cv2
from pathlib import Path
from hailo_sdk_client import ClientRunner

runner = ClientRunner(hw_arch='hailo8', har='yolo_tusimple.har')
calib_images = []
for img_path in list(Path('../../../datasets/convertedYolo/images/val').glob('*.jpg'))[:200]:
    img = cv2.imread(str(img_path))
    img = cv2.resize(img, (640, 640))
    img = img.astype(np.float32) / 255.0
    calib_images.append(img)

calib_dataset = np.array(calib_images)

runner.optimize(calib_dataset)
runner.save_har('yolo_tusimple_quantized.har')
```

---

### 4.4 Step 3 — Compile to `.hef`

```python
runner.load_har('yolo_tusimple_quantized.har')
hef = runner.compile()

with open('yolo_tusimple.hef', 'wb') as f:
    f.write(hef)

print('Done! yolo_tusimple.hef ready for deployment.')
```

---

### 4.5 Deploy on Device

```python
from hailo_platform import HEF, VDevice, HailoStreamInterface, InferVStreams, ConfigureParams

hef = HEF('yolo_tusimple.hef')
target = VDevice()

configure_params = ConfigureParams.create_from_hef(
    hef,
    interface=HailoStreamInterface.PCIe
)

network_group = target.configure(hef, configure_params)[0]

with InferVStreams(network_group, {}, {}) as infer_pipeline:
    pass
```

---

## 5. Common Issues & Troubleshooting

| Issue | Cause | Fix |
|------|------|-----|
| Labels not found | Folder mismatch | Mirror structure |
| GPU OOM | Batch too large | Reduce batch |
| ONNX parse error | Wrong node names | Use Netron |
| Accuracy drop | Poor calibration | Use more images |
| Unsupported ops | Model issue | Simplify head |
| Opset error | Version mismatch | Use opset 11/12 |

---

## 6. Required Tools & Installation

| Tool | Purpose | Install |
|------|--------|--------|
| ultralytics | Training/export | `pip install ultralytics` |
| onnx | Format support | `pip install onnx` |
| onnxsim | Simplify graph | `pip install onnxsim` |
| opencv-python | Image processing | `pip install opencv-python` |
| numpy | Arrays | `pip install numpy` |
| netron | Visualization | https://netron.app |
| hailo_dataflow_compiler | Compile | Download |
| hailo_platform | Runtime | Hailo SDK |

---

### Install Core Dependencies

```bash
conda activate myenv
pip install ultralytics onnx onnxsim opencv-python numpy
```
