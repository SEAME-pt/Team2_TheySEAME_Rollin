import os
import sys
import stat
import struct
import numpy as np
from pathlib import Path
import cv2
import fcntl


class NamedPipeWriter:
	"""
	Writes lane mask data to a named pipe in binary format.
	
	Binary format:
	- Frame number (uint32, 4 bytes)
	- Mask height (uint16, 2 bytes)
	- Mask width (uint16, 2 bytes)
	- Lane score (float32, 4 bytes)
	- Raw mask data (height * width bytes, uint8)
	"""
	
	HEADER_SIZE = 4 + 2 + 2 + 4  # 12 bytes
	
	def __init__(self, pipe_path="/root/bird/NamedPipe", include_header=True):
		"""
		Initialize the named pipe writer.
		
		Args:
			pipe_path: Path to the named pipe (FIFO)
		"""
		self.pipe_path = pipe_path
		self.pipe_fd = None
		self.include_header = include_header
		self._setup_pipe()
	
	def _setup_pipe(self):
		"""Create or open the named pipe."""
		try:
			# Remove existing pipe if it exists
			if os.path.exists(self.pipe_path):
				try:
					file_stat = os.stat(self.pipe_path)
					if not stat.S_ISFIFO(file_stat.st_mode):
						os.remove(self.pipe_path)
				except OSError:
					pass
				# If it's a FIFO, we can try to use it
				cv2.namedWindow('Binary Mask', cv2.WINDOW_NORMAL)
			else:
				# Create the FIFO
				os.mkfifo(self.pipe_path)
				print(f"Created named pipe at {self.pipe_path}", file=sys.stderr)
			
			# Open the pipe read-write so the writer can start before the C reader.
			self.pipe_fd = os.open(
				self.pipe_path,
				os.O_RDWR | os.O_NONBLOCK
			)
			print(f"Opened named pipe for read-write: {self.pipe_path}", file=sys.stderr)
			if hasattr(fcntl, 'F_SETPIPE_SZ'):
				try:
					new_size = fcntl.fcntl(self.pipe_fd, fcntl.F_SETPIPE_SZ, 4 * 1048576)
					print(f"New pipe buffer size: {new_size} bytes")
				except PermissionError:
					print("Failed to increase buffer: Exceeds system limits.")
		except (OSError, FileExistsError) as e:
			print(f"Warning: Could not setup named pipe at {self.pipe_path}: {e}", file=sys.stderr)
			self.pipe_fd = None

	def write_all(self, fd, data):
		total_sent = 0
		# If there's nothing to send, return 0 immediately
		data_len = len(data)
		while total_sent < data_len:
			# Write only the remaining portion of the data
			sent = os.write(fd, data[total_sent:])
			if sent == 0:
				raise RuntimeError("Pipe broken - reader likely closed")
			total_sent += sent
		return total_sent
	
	def write_mask(self, mask, frame_number, lane_score=0.0):
		"""
		Write a mask frame to the named pipe.
		
		Args:
			mask: numpy array (uint8) of shape (height, width) or None
			frame_number: Frame index (uint32)
			lane_score: Confidence score for the lane (float32)
		
		Returns:
			True if write was successful, False otherwise
		"""
		if self.pipe_fd is None:
			return False
		try:
			if mask is None:
				# Empty mask case: zero dimensions
				height, width = 0, 0
				header_bytes = struct.pack('!IHHf', frame_number, height, width, lane_score)
				if self.include_header:
					binary_data = header_bytes
				else:
					binary_data = b""
			else:
				# Ensure mask is uint8
				if mask.dtype != np.uint8:
					mask = (mask > 0).astype(np.uint8)
				else:
					mask = mask.astype(np.uint8)
				
				height, width = mask.shape
				if height > 65535 or width > 65535:
					print(f"Warning: Mask dimensions exceed uint16 range: {height}x{width}", file=sys.stderr)
					return False
				
				# Prepare header and raw bytes
				header_bytes = struct.pack('!IHHf', frame_number, height, width, lane_score)
				raw_bytes = mask.tobytes()
				if self.include_header:
					binary_data = header_bytes + raw_bytes
				else:
					binary_data = raw_bytes
				# Display mask for debugging
				binary_image = (mask * 255).astype(np.uint8)
				cv2.imshow('Binary Mask', binary_image)
				cv2.waitKey(1)
			
			# Try to write, handle the case where reader is not connected
			print(f"Attempting to write frame {frame_number} with score {lane_score:.2f} and mask size {height}x{width}", file=sys.stderr)
			bytes_written = self.write_all(self.pipe_fd, binary_data)
			if bytes_written != len(binary_data):
				print(f"Warning: Incomplete write to pipe. Expected {len(binary_data)} bytes, wrote {bytes_written}", file=sys.stderr)
				return False
			
			return True
		except BlockingIOError:
			# No reader connected yet
			return False
		except OSError as e:
			print(f"Error writing to pipe: {e}", file=sys.stderr)
			# Try to reconnect
			self._reconnect_pipe()
			return False
	
	def _reconnect_pipe(self):
		"""Attempt to reconnect to the pipe."""
		try:
			if self.pipe_fd is not None:
				os.close(self.pipe_fd)
			self.pipe_fd = os.open(
				self.pipe_path,
				os.O_RDWR | os.O_NONBLOCK
			)
			print(f"Reconnected to named pipe: {self.pipe_path}", file=sys.stderr)
		except OSError:
			self.pipe_fd = None
	
	def close(self):
		"""Close the named pipe."""
		if self.pipe_fd is not None:
			try:
				os.close(self.pipe_fd)
				print(f"Closed named pipe: {self.pipe_path}", file=sys.stderr)
				cv2.destroyAllWindows()
			except OSError as e:
				print(f"Warning: Error closing pipe: {e}", file=sys.stderr)
			self.pipe_fd = None
	
	def cleanup(self):
		"""Clean up resources and remove the named pipe."""
		self.close()
		try:
			if os.path.exists(self.pipe_path):
				try:
					file_stat = os.stat(self.pipe_path)
					if stat.S_ISFIFO(file_stat.st_mode):
						os.remove(self.pipe_path)
						print(f"Removed named pipe: {self.pipe_path}", file=sys.stderr)
				except OSError as e:
					print(f"Warning: Could not remove named pipe: {e}", file=sys.stderr)
		except OSError as e:
			print(f"Warning: Could not remove named pipe: {e}", file=sys.stderr)
	
	def __del__(self):
		"""Destructor to ensure cleanup."""
		self.close()
