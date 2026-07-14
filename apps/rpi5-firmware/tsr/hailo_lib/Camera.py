#
# This Class deals with connecting the Raspberry Pi camera and streaming it's output both for real time inference with hailo, and for display in the Cluster monitor
#

import os
import subprocess
import numpy as np
import cv2
from hailo_platform import (HEF, VDevice, HailoStreamInterface, InferVStreams,
	ConfigureParams, InputVStreamParams, OutputVStreamParams, FormatType)

class Camera:
	def __init__(self, CAM_HEIGHT, CAM_WIDTH, MODEL_HEIGHT, MODEL_WIDTH):
		self.display_fps = 20
		self.rpicam_cmd = [
			"rpicam-vid", "-t", "0", "--rotation", "180", "--width", "640", "--height", "640",
			"--nopreview", "--codec", "yuv420", "-o", "-"
		]
		self.gst_display_cmd = (
			"gst-launch-1.0 fdsrc do-timestamp=true ! "
			f"rawvideoparse format=bgr width={CAM_WIDTH} height={CAM_HEIGHT} framerate={self.display_fps}/1 ! "
			"queue leaky=downstream max-size-buffers=2 max-size-bytes=0 max-size-time=0 ! "
			"videoconvert ! waylandsink sync=false qos=false max-lateness=-1"
		)
		self.cam_proc = self.init_camera_process()
		self.display_proc = self.init_display_process()
		self.CAM_HEIGHT = CAM_HEIGHT
		self.CAM_WIDTH = CAM_WIDTH
		self.MODEL_HEIGHT = MODEL_HEIGHT
		self.MODEL_WIDTH = MODEL_WIDTH

	def init_camera_process(self):
		print("---Initializing Camera Process---")
		self.cam_proc = subprocess.Popen(self.rpicam_cmd, stdout=subprocess.PIPE, bufsize=10**7)
		return self.cam_proc

	def init_display_process(self):
		print("---Initializing Display Process---")
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

