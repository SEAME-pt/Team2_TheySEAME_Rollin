#
# This Class deals with connecting to the CARLA simulation camera over TCP and streaming frames for real time inference with hailo
#
import socket
import struct
import numpy as np
import cv2

CARLA_TCP_PORT = 5005

class CARLACamera:
    def __init__(self, CAM_HEIGHT, CAM_WIDTH, MODEL_HEIGHT, MODEL_WIDTH, port: int = CARLA_TCP_PORT):
        self.CAM_HEIGHT = CAM_HEIGHT
        self.CAM_WIDTH = CAM_WIDTH
        self.MODEL_HEIGHT = MODEL_HEIGHT
        self.MODEL_WIDTH = MODEL_WIDTH

        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sock.bind(('0.0.0.0', port))
        self._sock.listen(1)
        print(f"[CARLACamera] waiting connection on port {port}...", flush=True)
        self._conn, addr = self._sock.accept()
        print(f"[CARLACamera] Connected to {addr}", flush=True)

    def _recvall(self, n: int) -> bytes | None:
        buf = b''
        while len(buf) < n:
            chunk = self._conn.recv(n - len(buf))
            if not chunk:
                return None
            buf += chunk
        return buf

    def read_frame(self) -> np.ndarray | None:
        header = self._recvall(8)
        if header is None:
            return None

        rows, cols = struct.unpack(">II", header)  # big-endian, 2x uint32

        if rows == 0 or cols == 0 or rows > 4096 or cols > 4096:
            return None

        pixel_bytes = rows * cols * 3
        raw = self._recvall(pixel_bytes)
        if raw is None:
            return None

        frame = np.frombuffer(raw, dtype=np.uint8).reshape((rows, cols, 3)).copy()
        
        if rows != self.MODEL_HEIGHT or cols != self.MODEL_WIDTH:
            frame = cv2.resize(frame, (self.MODEL_WIDTH, self.MODEL_HEIGHT))

        return frame

    def terminate_camera(self):
        self._conn.close()
        self._sock.close()

    def terminate_display(self):
        pass