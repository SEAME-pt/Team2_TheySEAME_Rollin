# carla_camera.py
import socket
import struct
import sys
import numpy as np
import cv2

CARLA_TCP_PORT = 5005

class CARLACamera:
    def __init__(
        self,
        CAM_HEIGHT,
        CAM_WIDTH,
        MODEL_HEIGHT,
        MODEL_WIDTH,
        port: int = CARLA_TCP_PORT,
    ):
        self.CAM_HEIGHT   = CAM_HEIGHT
        self.CAM_WIDTH    = CAM_WIDTH
        self.MODEL_HEIGHT = MODEL_HEIGHT
        self.MODEL_WIDTH  = MODEL_WIDTH

        self._out = sys.stdout.buffer

        # ── TCP server ──────────────────────────────────────────────────
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sock.bind(('0.0.0.0', port))
        self._sock.listen(1)
        print(f"[CARLACamera] waiting on port {port}...", file=sys.stderr, flush=True)
        self._conn, addr = self._sock.accept()
        print(f"[CARLACamera] connected to {addr}", file=sys.stderr, flush=True)

    # ------------------------------------------------------------------ #
    #  TCP receive                                                         #
    # ------------------------------------------------------------------ #

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

    # ------------------------------------------------------------------ #
    #  Pipe to stdout (FFmpeg rawvideo BGR24)                              #
    # ------------------------------------------------------------------ #

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
            self._out.write(boundary)
            self._out.write(jpeg_bytes)
            self._out.write(b"\r\n")
            self._out.flush()
            return True
        except BrokenPipeError:
            print("[CARLACamera] stdout pipe closed.", file=sys.stderr, flush=True)
            return False

    # ------------------------------------------------------------------ #
    #  Cleanup                                                             #
    # ------------------------------------------------------------------ #

    def terminate_camera(self):
        self._conn.close()
        self._sock.close()

    def terminate_display(self):
        pass


# # ── Entry point ────────────────────────────────────────────────────────────────
# if __name__ == "__main__":
#     W, H = 320, 240
#     cam = CARLACamera(
#         CAM_HEIGHT=H, CAM_WIDTH=W,
#         MODEL_HEIGHT=H, MODEL_WIDTH=W,
#     )
#     try:
#         while True:
#             frame = cam.read_frame()          # ← was read_and_pipe_frame()
#             if frame is None:
#                 break
#             if not cam.write_frame_to_pipe(frame):  # ← pipe the frame out
#                 break
#     finally:
#         cam.terminate_camera()