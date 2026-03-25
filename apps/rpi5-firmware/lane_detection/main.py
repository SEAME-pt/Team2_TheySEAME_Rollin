from ultralytics import YOLO
import kagglehub

# Download latest version
path = kagglehub.dataset_download("manideep1108/tusimple")

print("Path to dataset files:", path)

model = YOLO("yolo11n-seg.pt")
model.train(data="../../datasets/TuSimple/tusimple.yaml", epochs=100, imgsz=640, device=0)
results = model("path/to/image.jpg")
