import os

import cv2
import numpy as np
from hailo_platform import (
	HEF,
	VDevice,
	HailoStreamInterface,
	InferVStreams,
	ConfigureParams,
	InputVStreamParams,
	OutputVStreamParams,
)

CAM_HEIGHT = 640
CAM_WIDTH =640
MODEL_HEIGHT = 640
MODEL_WIDTH = 640


class Inference:
	def __init__(self, camera, hef_path=None):
		default_hef_path = os.path.normpath(
			os.path.join(os.path.dirname(__file__), "..", "..", "trained_models", "yolov8n_seg_100e.hef")
		)
		self.hef_path = hef_path or default_hef_path
		self.HEF = HEF(self.hef_path)
		self.network_group = None
		self.input_vstream_params = None
		self.output_vstream_params = None
		self.target = VDevice()
		self.camera = camera
		self.CAM_HEIGHT = CAM_HEIGHT
		self.CAM_WIDTH = CAM_WIDTH
		self.MODEL_HEIGHT = MODEL_HEIGHT
		self.MODEL_WIDTH = MODEL_WIDTH
		self.network_group_params = None

	def get_quant_params(self):
		output_vstream_infos = self.HEF.get_output_vstream_infos()
	
		quant_params = {}
		for output_info in output_vstream_infos:
			quant_params[output_info.name] = (
				output_info.quant_info.qp_scale,
				output_info.quant_info.qp_zp
			)
		return quant_params

	def config_network_streams(self):
		configure_params = ConfigureParams.create_from_hef(
			hef=self.HEF, interface=HailoStreamInterface.PCIe
		)
		network_groups = self.target.configure(self.HEF, configure_params)
		self.network_group = network_groups[0]
		self.network_group_params = self.network_group.create_params()
		self.input_vstream_params = InputVStreamParams.make(self.network_group)
		self.output_vstream_params = OutputVStreamParams.make(self.network_group)

	def _read_frame(self):
		if hasattr(self.camera, "read_frame"):
			return self.camera.read_frame()

		frame_size = self.CAM_HEIGHT * self.CAM_WIDTH * 3 // 2
		raw_frame = self.camera.cam_proc.stdout.read(frame_size)
		if not raw_frame or len(raw_frame) != frame_size:
			return None

		yuv = np.frombuffer(raw_frame, dtype=np.uint8).reshape(
			(int(self.CAM_HEIGHT * 1.5), self.CAM_WIDTH)
		)
		frame = cv2.cvtColor(yuv, cv2.COLOR_YUV2BGR_I420)
		return frame

	def run_inference(self):
		self.config_network_streams()
		input_name = list(self.input_vstream_params.keys())[0]

		with InferVStreams(
			self.network_group,
			self.input_vstream_params,
			self.output_vstream_params,
		) as pipeline:
			with self.network_group.activate(self.network_group_params):
				while True:
					frame = self._read_frame()
					if frame is None:
						break

					frame_resized = cv2.resize(
						frame, (self.MODEL_WIDTH, self.MODEL_HEIGHT)
					)
					input_frame = np.expand_dims(frame_resized, axis=0).astype(np.uint8)
					infer_results = pipeline.infer({input_name: input_frame})
					yield frame, infer_results

