🚀 YOLO Segmentation to Hailo HEF: Complete Guide

This guide covers the end-to-end workflow for training a YOLOv8/11 segmentation model and compiling it for Hailo-8/8L hardware.
📋 Prerequisites

Ensure your host machine has the following installed before starting:

    Docker

    NVIDIA Container Toolkit (Required for GPU acceleration)

    Hailo AI Software Suite (Download via Hailo Developer Zone)

🛠️ Phase 1: Environment Preparation
1. Clone the Repository
```
git clone https://github.com/hailo-ai/hailo_model_zoo.git
cd hailo_model_zoo/training/yolov8_seg
```
2. Build the Training Image
```
docker build --build-arg timezone=`cat /etc/timezone` -t yolov8_seg:v0 .
```
3. Launch the Container

Map your local dataset directory to the container's workspace.
```
# Example for SEA:ME PC
docker run --name yolov8_seg_train -it --gpus all --ipc=host \
-v /home/seame/Desktop/dataset:/workspace/datasets yolov8_seg:v0
```
🏗️ Phase 2: Training and Exporting
1. Model Training

Prepare your data.yaml and labels in YOLO format, then choose your architecture:

For YOLOv8n-seg:
```
yolo segment train data=/workspace/datasets/data.yaml model=yolov8n-seg.pt epochs=100 imgsz=640 device=0
```
For YOLO11n-seg (Newer/Faster):
```
yolo segment train data=/workspace/datasets/data.yaml model=yolo11n-seg.pt epochs=100 imgsz=640 device=0
```
2. Export to ONNX

Hailo requires ONNX opset 11. Run this command once training is finished:
```
yolo export model=/workspace/ultralytics/runs/segment/train/weights/best.pt \
format=onnx opset=11 imgsz=640
```
    [!IMPORTANT]
    Move the resulting best.onnx file into the shared_with_docker folder of your Hailo Software Suite directory.

⚙️ Phase 3: Hailo Compilation (HEF Generation)
1. Launch Hailo SW Suite

Navigate to your suite directory on the host and run:
```
./hailo_ai_sw_suite_docker_run.sh
```
2. Enter the Shared Directory

Inside the container, navigate to your files:
```
cd /local/shared_with_docker
```
3. Run the Hailo Compiler

The hailomz tool uses the architecture logic of yolov8n_seg while overriding the weights with your custom ONNX file.
Flag	Description
--ckpt	Path to your exported ONNX model
--calib-path	Folder containing calibration images (~100 images)
--classes	Number of classes in your dataset
--hw-arch	hailo8l (RPi AI Kit) or hailo8 (M.2 Module)
```
hailomz compile yolov8n_seg \
    --ckpt best.onnx \
    --calib-path calib_set/ \
    --classes 5 \
    --hw-arch hailo8l
```
🏁 Final Output

Once completed, yolov8n_seg.hef will appear in your shared_with_docker folder. This file is ready for deployment on your Raspberry Pi 5 or Hailo-8 hardware.
💡 Troubleshooting & Pro-Tips

    Calibration Accuracy: Ensure your calib_set contains roughly 100 representative images from your dataset. This is vital for maintaining accuracy during 8-bit quantization.

    Numpy Errors: If you see ModuleNotFoundError: No module named 'numpy._core', delete any hidden .cache files/folders inside your dataset directory and restart.

    Hardware Check: Always double-check your --hw-arch. Using hailo8l settings on a standard hailo8 chip (and vice versa) will result in deployment errors.
