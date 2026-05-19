import os
import subprocess
import sys
import struct
import traceback
import argparse
import threading
import queue

import numpy as np
import cv2

MODEL_HEIGHT = 320
MODEL_WIDTH  = 320
DISPLAY_WIDTH  = 640
DISPLAY_HEIGHT = 480
USE_HAILO = True

if USE_HAILO:
    from hailo_platform import (
        HEF, VDevice, HailoStreamInterface, InferVStreams,
        ConfigureParams, InputVStreamParams, OutputVStreamParams,
    )

def read_frame_from_stdin() -> np.ndarray | None:
    import select

    while True:
        rlist, _, _ = select.select([sys.stdin.buffer], [], [], 0)
        if not rlist:
            break
        header = sys.stdin.buffer.read(8)
        if len(header) < 8:
            return None
        rows, cols = struct.unpack("II", header)
        if rows == 0 or cols == 0 or rows > 4096 or cols > 4096:
            return None
        sys.stdin.buffer.read(rows * cols * 3)

    header = sys.stdin.buffer.read(8)
    if len(header) < 8:
        return None

    rows, cols = struct.unpack("II", header)
    if rows == 0 or cols == 0 or rows > 4096 or cols > 4096:
        return None

    nbytes = rows * cols * 3
    raw = sys.stdin.buffer.read(nbytes)
    if len(raw) < nbytes:
        return None

    return np.frombuffer(raw, dtype=np.uint8).reshape((rows, cols, 3)).copy()


class Display:
    def __init__(self, width: int, height: int):
        gst_cmd = (
            f"gst-launch-1.0 fdsrc ! "
            f"rawvideoparse format=bgr width={width} height={height} framerate=30/1 ! "
            f"queue max-size-buffers=1 leaky=downstream ! "
            f"videoconvert ! waylandsink sync=false max-lateness=0"
        )
        print("[Display] A inicializar GStreamer...", flush=True)
        self.width  = width
        self.height = height
        self.proc   = subprocess.Popen(
            gst_cmd, stdin=subprocess.PIPE, shell=True, bufsize=0,
            env=os.environ.copy()
        )
        self._q = queue.Queue(maxsize=1)
        self._thread = threading.Thread(target=self._writer, daemon=True)
        self._thread.start()

    def _writer(self):
        while True:
            frame = self._q.get()
            if frame is None:
                break
            try:
                if frame.shape[1] != self.width or frame.shape[0] != self.height:
                    frame = cv2.resize(frame, (self.width, self.height),
                                       interpolation=cv2.INTER_LINEAR)
                self.proc.stdin.write(frame.tobytes())
            except BrokenPipeError:
                break

    def show(self, frame: np.ndarray):
        if self._q.full():
            try:
                self._q.get_nowait()
            except queue.Empty:
                pass
        self._q.put(frame)

    def terminate(self):
        self._q.put(None)
        if self.proc.stdin:
            self.proc.stdin.close()
        self.proc.terminate()


class Inference:
    def __init__(self, use_hailo: bool = True):
        self.use_hailo = use_hailo
        if self.use_hailo:
            self.hef_path = "../yolo_tusimple.hef"
            self.HEF      = HEF(self.hef_path)
            self.target   = VDevice()
            self.network_group        = None
            self.network_group_params = None
            self.input_vstream_params  = None
            self.output_vstream_params = None
        else:
            print("[Inference] Hailo desactivado — modo display-only.", flush=True)
        print("[Inference] Inicializado.", flush=True)

    def _config_streams(self):
        configure_params = ConfigureParams.create_from_hef(
            hef=self.HEF, interface=HailoStreamInterface.PCIe)
        network_groups = self.target.configure(self.HEF, configure_params)
        self.network_group        = network_groups[0]
        self.network_group_params = self.network_group.create_params()
        self.input_vstream_params  = InputVStreamParams.make(self.network_group)
        self.output_vstream_params = OutputVStreamParams.make(self.network_group)

    def run_inference(self, display: Display | None):
        if not self.use_hailo:
            self._inference_loop(None, display)
            return

        self._config_streams()
        with InferVStreams(self.network_group,
                           self.input_vstream_params,
                           self.output_vstream_params) as pipeline:
            with self.network_group.activate(self.network_group_params):
                self._inference_loop(pipeline, display)

    def _inference_loop(self, pipeline, display: Display | None):
        input_name = None
        if self.use_hailo:
            input_name = list(self.input_vstream_params.keys())[0]

        frame_count = 0
        while True:
            new_frame = read_frame_from_stdin()
            if new_frame is None:
                break
            frame = new_frame
            if sys.stdin.buffer.peek(1) == b'':
                break

            frame_count += 1

            frame_model = cv2.resize(frame, (MODEL_WIDTH, MODEL_HEIGHT),
                                     interpolation=cv2.INTER_LINEAR)
            input_tensor = np.expand_dims(frame_model, axis=0).astype(np.uint8)

            if self.use_hailo and pipeline is not None:
                results = pipeline.infer({input_name: input_tensor})
                for name, data in results.items():
                    print(f"[Hailo] {name}: shape={data.shape}", flush=True)

            # Display
            if display is not None:
                if frame_count % 60 == 0:
                    print(f"[Inference] {frame_count} frames processados.", flush=True)
                cv2.putText(frame, f"Frame {frame_count}", (10, 30),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
                display.show(frame)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--no-hailo", action="store_true",
                        help="Desactiva inferência Hailo")
    parser.add_argument("--display",  action="store_true",
                        help="Activa display Wayland via GStreamer")
    args = parser.parse_args()

    display = None
    try:
        if args.display:
            display = Display(DISPLAY_WIDTH, DISPLAY_HEIGHT)

        engine = Inference(use_hailo=not args.no_hailo)
        engine.run_inference(display)

    except KeyboardInterrupt:
        print("\n[Main] Interrompido.", flush=True)
    except Exception:
        traceback.print_exc()
    finally:
        if display:
            display.terminate()
