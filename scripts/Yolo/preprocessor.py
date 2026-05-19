##  
##  This script uses gstream to pipe the  Rpicam library output to waylandsink 
##

import os
import subprocess
import numpy as np
import cv2
import socket
import struct
from hailo_platform import (HEF, VDevice, HailoStreamInterface, InferVStreams,
	ConfigureParams,
	InputVStreamParams, OutputVStreamParams, InputVStreams, OutputVStreams,
	FormatType)

CAM_HEIGHT = 640
CAM_WIDTH = 640
MODEL_HEIGHT = 320
MODEL_WIDTH = 320
# Port number - to send data over socket
PORT_NUMBER = 4450

class Inference:
	def __init__(self, camera):
		self.hef_path = "../yolo_tusimple.hef"
		self.HEF = HEF(self.hef_path)
		
		self.network_group = None
		self.input_vstream_params = None
		self.output_vstreams_params = None
		self.target = VDevice()
		self.camera = camera
		self.CAM_HEIGHT = CAM_HEIGHT
		self.CAM_WIDTH = CAM_WIDTH
		self.MODEL_HEIGHT = MODEL_HEIGHT
		self.MODEL_WIDTH = MODEL_WIDTH
		self.network_group_params = None
		
		self.socket = None
		self.conn = None
	def config_network_streams(self):
		configure_params = ConfigureParams.create_from_hef(hef=self.HEF,interface=HailoStreamInterface.PCIe)
		network_groups = self.target.configure(self.HEF, configure_params)
		self.network_group = network_groups[0]
		self.network_group_params =self. network_group.create_params()

		self.input_vstream_params = InputVStreamParams.make(self.network_group)
		self.output_vstream_params = OutputVStreamParams.make(self.network_group)
	  
	def run_inference(self):
		self.config_network_streams()
		with InferVStreams(self.network_group, self.input_vstream_params, self.output_vstream_params) as pipeline:
			with self.network_group.activate(self.network_group_params):
				self.inference_loop(pipeline, self.camera)

	def send_output_over_socket(self, data, header):
		if not self.socket:
			# Initialize socket only once
			self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			self.socket.bind(('localhost', PORT_NUMBER))
			self.socket.listen()
			print("Socket initialized, waiting for connection...")
		if not self.conn:
			# Accept connection only once
			self.conn, addr = self.socket.accept()
			print(f"Connected to {addr}")
		self.conn.sendall(header + data)

	def inference_loop(self, pipeline, camera):
		input_name = list(self.input_vstream_params.keys())[0]
		while True:
			# YUV420 format: Y plane (full resolution) + U plane (1/4) + V plane (1/4) = 1.5x pixels
			frame_size = self.CAM_HEIGHT * self.CAM_WIDTH * 3 // 2
			raw_frame = camera.cam_proc.stdout.read(frame_size)
			if not raw_frame or len(raw_frame) != frame_size:
				break
			
			# Fixed: YUV420 reshape - height is CAM_HEIGHT * 1.5, width is CAM_WIDTH
			yuv = np.frombuffer(raw_frame, dtype=np.uint8).reshape((int(self.CAM_HEIGHT * 1.5), self.CAM_WIDTH))
			frame = cv2.cvtColor(yuv, cv2.COLOR_YUV2BGR_I420)
			
			# Now frame is 640x640x3 BGR - resize to model input size
			frame_resized = cv2.resize(frame, (MODEL_WIDTH, MODEL_HEIGHT))
			input_frame = np.expand_dims(frame_resized, axis=0).astype(np.uint8)
			
			# --- 3. Inference ---
			# Prepare input dictionary using the name from the info object
			input_data = {input_name: input_frame}
			infer_results = pipeline.infer(input_data)
			print(infer_results.items())
			for name, tensor in infer_results.items():
				#print(f"Output {name} data: {data}")
				data = tensor.tobytes()
				size = len(data)
				shape = tensor.shape
				ndim = len(shape)
				name_bytes = name.encode('utf-8')
				name_len = len(name_bytes)

				print("Name: ", name, "Tensor: ", tensor)
				# Pack: size (4B), name_len (4B), name (name_len B), ndim (4B), shape (ndim x 4B)
				header = struct.pack(
					f"II{name_len}sI{ndim}I",
					size, name_len, name_bytes, ndim, *shape
				)
				self.send_output_over_socket(data, header)
			# --- 4. Visualize ---
			# Use original 640x640 frame for display
			cv2.putText(frame, "Inference Active", (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
			
			# Send processed frame to GStreamer Wayland sink
			try:
				camera.display_proc.stdin.write(frame.tobytes())
				camera.display_proc.stdin.flush()
			except BrokenPipeError:
				break
		
		def __del__(self):
			if self.conn:
				self.conn.close()
			if self.socket:
				self.socket.close()

class Camera:
	def __init__(self):
		self.rpicam_cmd = [
			"rpicam-vid", "-t", "0", "--width", "640", "--height", "640",
			"--nopreview", "--codec", "yuv420", "-o", "-"
		]

		self.gst_display_cmd = (
			"gst-launch-1.0 fdsrc ! "
			"rawvideoparse format=bgr width=640 height=640 framerate=30/1 ! "
			"videoconvert ! waylandsink"
		)
		self.cam_proc = self.init_camera_process()
		self.display_proc = self.init_display_process()

	def init_camera_process(self):
		
		# Get Wayland environment variables from the current system
		# If running as root, you may need to manually set these to "/run/user/1000" and "wayland-0"
		# Updated display command using rawvideoparse for better stability
		print("---Innitializing Camera Process---")
		self.cam_proc =  subprocess.Popen(self.rpicam_cmd, stdout=subprocess.PIPE, bufsize=10**7)
		return self.cam_proc

	def init_display_process(self):
		print("---Innitializing Display Process---")
		env = os.environ.copy()    
		self.display_proc = subprocess.Popen(
			self.gst_display_cmd, 
			stdin=subprocess.PIPE, 
			env=env, 
			shell=True
		)
		return self.display_proc

	def terminate_camera(self):
		self.cam_proc.terminate()
	
	def terminate_display(self):
		if self.display_proc.stdin:
			self.display_proc.stdin.close()
		self.display_proc.terminate()
	
if __name__ == '__main__':
	camera = None
	try:
		camera = Camera()
		# Ensure Inference is an instance, not a class call
		infer_engine = Inference(camera) 
		infer_engine.run_inference()
	except Exception as e:
		print(f"FATAL ERROR: {e}")
	finally:
		# This ensures that even if it crashes, the camera is released
		if camera:
			camera.terminate_camera()
			camera.terminate_display()
