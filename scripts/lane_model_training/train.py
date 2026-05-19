from ultralytics import YOLO
# Load model
model = YOLO('yolo11n-seg.pt')
# Train
model.train(data='../../../datasets/convertedYolo/data.yaml', epochs=100, imgsz=640, batch=16)
