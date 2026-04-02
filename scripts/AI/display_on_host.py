import cv2, socket, struct

cap = cv2.VideoCapture("libcamerasrc ! video/x-raw,width=640,height=480,format=BGR ! videoconvert ! appsink", cv2.CAP_GSTREAMER)
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(('10.21.220.143', 9999))
print("Connected")
while True:
    ret, frame = cap.read()
    if not ret:
        continue
    _, buf = cv2.imencode('.jpg', frame)
    data = buf.tobytes()
    client.sendall(struct.pack(">L", len(data)) + data)
