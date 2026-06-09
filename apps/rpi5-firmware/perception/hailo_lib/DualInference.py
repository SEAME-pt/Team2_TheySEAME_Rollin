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
CAM_WIDTH = 640
MODEL_HEIGHT = 640
MODEL_WIDTH = 640


class DualInference:
	"""Run lane segmentation and TSR detection on one camera stream with one VDevice."""

	def __init__(self, camera, seg_hef_path, det_hef_path):
		self.camera = camera
		self.seg_hef_path = seg_hef_path
		self.det_hef_path = det_hef_path
		self.seg_hef = HEF(seg_hef_path)
		self.det_hef = HEF(det_hef_path)
		self.target = VDevice()
		self.seg_network_group = None
		self.det_network_group = None
		self.seg_network_group_params = None
		self.det_network_group_params = None
		self.seg_input_vstream_params = None
		self.seg_output_vstream_params = None
		self.det_input_vstream_params = None
		self.det_output_vstream_params = None
		self.seg_input_name = None
		self.det_input_name = None
		self.CAM_HEIGHT = CAM_HEIGHT
		self.CAM_WIDTH = CAM_WIDTH
		self.MODEL_HEIGHT = MODEL_HEIGHT
		self.MODEL_WIDTH = MODEL_WIDTH

	def _quant_params_for_hef(self, hef):
		quant_params = {}
		for output_info in hef.get_output_vstream_infos():
			quant_params[output_info.name] = (
				output_info.quant_info.qp_scale,
				output_info.quant_info.qp_zp,
			)
		return quant_params

	def get_seg_quant_params(self):
		return self._quant_params_for_hef(self.seg_hef)

	def get_det_quant_params(self):
		return self._quant_params_for_hef(self.det_hef)

	def configure(self):
		interface = HailoStreamInterface.PCIe

		seg_configure_params = ConfigureParams.create_from_hef(
			hef=self.seg_hef, interface=interface
		)
		seg_groups = self.target.configure(self.seg_hef, seg_configure_params)
		self.seg_network_group = seg_groups[0]
		self.seg_network_group_params = self.seg_network_group.create_params()
		self.seg_input_vstream_params = InputVStreamParams.make(self.seg_network_group)
		self.seg_output_vstream_params = OutputVStreamParams.make(self.seg_network_group)
		self.seg_input_name = list(self.seg_input_vstream_params.keys())[0]

		det_configure_params = ConfigureParams.create_from_hef(
			hef=self.det_hef, interface=interface
		)
		det_groups = self.target.configure(self.det_hef, det_configure_params)
		self.det_network_group = det_groups[0]
		self.det_network_group_params = self.det_network_group.create_params()
		self.det_input_vstream_params = InputVStreamParams.make(self.det_network_group)
		self.det_output_vstream_params = OutputVStreamParams.make(self.det_network_group)
		self.det_input_name = list(self.det_input_vstream_params.keys())[0]

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
		return cv2.cvtColor(yuv, cv2.COLOR_YUV2BGR_I420)

	def _prepare_input(self, frame):
		frame_resized = cv2.resize(frame, (self.MODEL_WIDTH, self.MODEL_HEIGHT))
		return np.expand_dims(frame_resized, axis=0).astype(np.uint8)

	def run_inference(self):
		self.configure()

		with InferVStreams(
			self.seg_network_group,
			self.seg_input_vstream_params,
			self.seg_output_vstream_params,
		) as seg_pipeline, InferVStreams(
			self.det_network_group,
			self.det_input_vstream_params,
			self.det_output_vstream_params,
		) as det_pipeline:
			while True:
				frame = self._read_frame()
				if frame is None:
					break

				input_frame = self._prepare_input(frame)

				with self.seg_network_group.activate(self.seg_network_group_params):
					seg_results = seg_pipeline.infer({self.seg_input_name: input_frame})

				with self.det_network_group.activate(self.det_network_group_params):
					det_results = det_pipeline.infer({self.det_input_name: input_frame})

				yield frame, seg_results, det_results
