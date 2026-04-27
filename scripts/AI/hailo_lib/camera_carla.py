import socket
import struct
import sys
import subprocess
import threading
import numpy as np
import cv2

CARLA_TCP_PORT = 5005

class CARLACamera:
    def __init__(self, CAM_HEIGHT, CAM_WIDTH, MODEL_HEIGHT, MODEL_WIDTH, port=CARLA_TCP_PORT):
        self.CAM_HEIGHT   = CAM_HEIGHT
        self.CAM_WIDTH    = CAM_WIDTH
        self.MODEL_HEIGHT = MODEL_HEIGHT
        self.MODEL_WIDTH  = MODEL_WIDTH
        self._conn = None
        self._lock = threading.Lock()

        # FFmpeg interno — lê raw BGR do stdin, escreve AVI para o pipe
        self._ffmpeg = subprocess.Popen([
            'ffmpeg', '-y',
            '-f', 'rawvideo',
            '-pix_fmt', 'bgr24',
            '-s', f'{MODEL_WIDTH}x{MODEL_HEIGHT}',
            '-r', '20',
            '-i', 'pipe:0',
            '-f', 'avi',
            '/tmp/carla.pipe'
        ], stdin=subprocess.PIPE, stderr=subprocess.DEVNULL)

        # TCP em background
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sock.bind(('0.0.0.0', port))
        self._sock.listen(1)
        print(f"[CARLACamera] waiting on port {port}...", file=sys.stderr, flush=True)
        threading.Thread(target=self._accept, daemon=True).start()

    def _accept(self):
        conn, addr = self._sock.accept()
        print(f"[CARLACamera] connected to {addr}", file=sys.stderr, flush=True)
        with self._lock:
            self._conn = conn

    def _recvall(self, n):
        buf = b''
        while len(buf) < n:
            with self._lock:
                conn = self._conn
            if conn is None:
                return None
            chunk = conn.recv(n - len(buf))
            if not chunk:
                return None
            buf += chunk
        return buf

    def read_frame(self):
        with self._lock:
            if self._conn is None:
                return np.zeros((self.MODEL_HEIGHT, self.MODEL_WIDTH, 3), dtype=np.uint8)
        header = self._recvall(8)
        if header is None:
            return None
        rows, cols = struct.unpack(">II", header)
        if rows == 0 or cols == 0 or rows > 4096 or cols > 4096:
            return None
        raw = self._recvall(rows * cols * 3)
        if raw is None:
            return None
        frame = np.frombuffer(raw, dtype=np.uint8).reshape((rows, cols, 3)).copy()
        if rows != self.MODEL_HEIGHT or cols != self.MODEL_WIDTH:
            frame = cv2.resize(frame, (self.MODEL_WIDTH, self.MODEL_HEIGHT))
        return frame

    def write_frame_to_pipe(self, frame: np.ndarray) -> bool:
        ret, jpeg_buf = cv2.imencode('.jpg', frame, [int(cv2.IMWRITE_JPEG_QUALITY), 90])
        if not ret:
            return False
        jpeg_bytes = jpeg_buf.tobytes()
        try:
            boundary = (
                b"--frame\r\n"
                b"Content-Type: image/jpeg\r\n"
                b"Content-Length: " + str(len(jpeg_bytes)).encode() + b"\r\n"
                b"\r\n"
            )
            try:
                self._ffmpeg.stdin.write(frame.tobytes())
                return True
            except BrokenPipeError:
                print("[CARLACamera] stdout pipe closed.", file=sys.stderr, flush=True)
                return False
        except Exception as e:
            print(f"[CARLACamera] error writing to pipe: {e}", file=sys.stderr, flush=True)
            return False

    def terminate_camera(self):
        self._ffmpeg.stdin.close()
        self._ffmpeg.wait()
        with self._lock:
            if self._conn:
                self._conn.close()
        self._sock.close()


if __name__ == "__main__":
    W, H = 320, 240
    cam = CARLACamera(CAM_HEIGHT=H, CAM_WIDTH=W, MODEL_HEIGHT=H, MODEL_WIDTH=W)
    try:
        while True:
            frame = cam.read_frame()
            if frame is None:
                break
            cam.write_frame_to_pipe(frame)
    finally:
        cam.terminate_camera()