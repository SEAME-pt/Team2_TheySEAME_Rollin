import argparse
import os
import signal
import stat
import struct
import sys
import time

import cv2

from hailo_lib import Camera
from hailo_lib import Inference
from hailo_lib import PostProcessor
from ArucoDetector import ArucoDetector

CAM_HEIGHT = 640
CAM_WIDTH = 640
MODEL_HEIGHT = 640
MODEL_WIDTH = 640
PRINT_OUTPUT_TENSORS_ONCE = True


try:
    signal.signal(signal.SIGPIPE, signal.SIG_DFL)
except AttributeError:
    pass


class NamedPipeDetWriter:
    """
    Binary format per frame:
    - frameNbr (uint32)
    - numDetections (uint16)
    - trafficSign (uint16)
    - accuracy (float32)
    - x (uint32)
    - y (uint32)
    - width (uint32)
    - height (uint32)
    """

    MSG_FMT = ">IHHfIIII"

    def __init__(self, pipe_path):
        self.pipe_path = pipe_path
        self.pipe_fd = None
        self.frame_nbr = 77000
        self._setup_pipe()

    def _setup_pipe(self):
        try:
            if os.path.exists(self.pipe_path):
                try:
                    file_stat = os.stat(self.pipe_path)
                    if not stat.S_ISFIFO(file_stat.st_mode):
                        os.remove(self.pipe_path)
                except OSError:
                    pass
            else:
                os.mkfifo(self.pipe_path)

            self.pipe_fd = os.open(self.pipe_path, os.O_RDWR | os.O_NONBLOCK)
        except OSError:
            self.pipe_fd = None

    def write_all(self, fd, data):
        total_sent = 0
        data_len = len(data)
        while total_sent < data_len:
            sent = os.write(fd, data[total_sent:])
            if sent == 0:
                raise RuntimeError("Pipe broken - reader likely closed")
            total_sent += sent
        return total_sent

    def write_detections(self, dets):
        if self.pipe_fd is None:
            return False

        try:
            num_det = len(dets)
            if num_det == 0:
                payload = struct.pack(
                    self.MSG_FMT,
                    self.frame_nbr,
                    0,
                    0,
                    0.0,
                    0,
                    0,
                    0,
                    0,
                )
                self.write_all(self.pipe_fd, payload)
                return True

            for det in dets:
                traffic_sign, accuracy, x, y, w, h = det
                payload = struct.pack(
                    self.MSG_FMT,
                    self.frame_nbr,
                    int(num_det) & 0xFFFF,
                    int(traffic_sign) & 0xFFFF,
                    float(accuracy),
                    int(x) & 0xFFFFFFFF,
                    int(y) & 0xFFFFFFFF,
                    int(w) & 0xFFFFFFFF,
                    int(h) & 0xFFFFFFFF,
                )
                self.write_all(self.pipe_fd, payload)
            return True
        except (BlockingIOError, OSError, RuntimeError):
            return False

    def close(self):
        if self.pipe_fd is not None:
            try:
                os.close(self.pipe_fd)
            except OSError:
                pass
            self.pipe_fd = None


def load_labels(path):
    if not path or not os.path.exists(path):
        return []
    with open(path, "r", encoding="utf-8") as f:
        return [line.strip() for line in f if line.strip()]


def draw_detections_on_frame(frame, dets, labels):
    """Overlay bounding boxes and labels on the BGR frame shown on the display."""
    for cls_id, score, x, y, w, h in dets:
        x1, y1 = int(x), int(y)
        x2, y2 = x1 + int(w), y1 + int(h)
        color = (0, 0, 255)
        cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
        label = labels[cls_id] if 0 <= cls_id < len(labels) else str(cls_id)
        text = f"{label} {score:.2f}"
        cv2.putText(
            frame,
            text,
            (x1, max(y1 - 6, 12)),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            color,
            1,
            cv2.LINE_AA,
        )
    return frame


def write_frame(display_proc, frame):
    if display_proc is None or getattr(display_proc, "stdin", None) is None:
        return False
    try:
        display_proc.stdin.write(frame.tobytes())
        display_proc.stdin.flush()
        return True
    except BrokenPipeError:
        return False


def _parse_args():
    parser = argparse.ArgumentParser(description="Run YOLOv8 detection on Pi camera")
    parser.add_argument(
        "--hef",
        default="/root/tsr/models/yolov8n.hef",
        help="Path to HEF file",
    )
    parser.add_argument(
        "--labels",
        default="/root/tsr/models/labels.txt",
        help="Path to labels.txt",
    )
    parser.add_argument("--conf", type=float, default=0.35, help="Confidence threshold")
    parser.add_argument("--iou", type=float, default=0.5, help="NMS IoU threshold")
    parser.add_argument(
        "--pipe",
        default="/root/tsr/NamedPipeTsr",
        help="Named pipe path for detections",
    )
    parser.add_argument(
        "--no-display",
        action="store_true",
        help="Disable display output",
    )
    return parser.parse_args()


if __name__ == "__main__":
    args = _parse_args()
    camera = None
    printed_tensor_info = False
    frame_idx = 0

    time.sleep(5)

    try:
        labels = load_labels(args.labels)
        camera = Camera(CAM_HEIGHT, CAM_WIDTH, MODEL_HEIGHT, MODEL_WIDTH)
        print("Using Raspberry Pi camera")

        if args.no_display and camera.display_proc is not None:
            camera.terminate_display()
            camera.display_proc = None

        arDetect = ArucoDetector()
        infer_engine = Inference(camera, args.hef)
        post_processor = PostProcessor(
            input_size=(MODEL_HEIGHT, MODEL_WIDTH),
            strides=(8, 16, 32),
        )

        pipe_writer = NamedPipeDetWriter(args.pipe)

        for frame, infer_results in infer_engine.run_inference():
            frame_idx += 1
            if PRINT_OUTPUT_TENSORS_ONCE and not printed_tensor_info:
                print("=== Model output tensors ===", flush=True)
                for out_name, out_tensor in infer_results.items():
                    print(f"{out_name}: shape={out_tensor.shape}, dtype={out_tensor.dtype}", flush=True)
                print("============================", flush=True)
                printed_tensor_info = True

            detections = post_processor.decode(
                infer_results,
                quant_params=infer_engine.get_quant_params(),
                conf_th=args.conf,
                iou_th=args.iou,
            )
            arDetect.detect(frame)
            markerId = arDetect.getDetection()
            cv2.putText(
                frame,
                str(markerId),
                (100, 100),
                cv2.FONT_HERSHEY_SIMPLEX,
                2.5,
                (0, 0, 255),
                8,
                cv2.LINE_AA,
            )
            print("Marker:", markerId)

            boxes = detections.get("boxes", [])
            scores = detections.get("scores", [])
            classes = detections.get("classes", [])

            dets = []
            for box, score, cls_id in zip(boxes, scores, classes):
                x1, y1, x2, y2 = box.astype(int)
                w = max(0, x2 - x1)
                h = max(0, y2 - y1)
                dets.append((int(cls_id), float(score), x1, y1, w, h))

            if frame_idx % 30 == 0:
                if not dets:
                    print("DET: none", flush=True)
                else:
                    cls_id, score, x, y, w, h = dets[0]
                    label = labels[cls_id] if 0 <= cls_id < len(labels) else None
                    print(
                        f"DET: class={cls_id} label={label} score={score:.3f} "
                        f"box=({x},{y},{w},{h})"
                    , flush=True)

            pipe_writer.write_detections(dets)

            if not args.no_display:
                display_frame = draw_detections_on_frame(frame.copy(), dets, labels)
                if not write_frame(camera.display_proc, display_frame):
                    break

    except Exception as e:
        try:
            sys.stderr.write(f"FATAL ERROR: {e}\n")
        except Exception:
            pass
    finally:
        if camera:
            camera.terminate_camera()
            if getattr(camera, "display_proc", None) is not None:
                camera.terminate_display()
