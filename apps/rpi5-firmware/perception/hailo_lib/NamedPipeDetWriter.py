import os
import signal
import stat
import struct


try:
	signal.signal(signal.SIGPIPE, signal.SIG_DFL)
except AttributeError:
	pass


class NamedPipeDetWriter:
	"""Write TSR detection frames to a named pipe in the firmware binary format."""

	MSG_FMT = ">IHHfIIII"

	def __init__(self, pipe_path, frame_start=77000):
		self.pipe_path = pipe_path
		self.pipe_fd = None
		self.frame_nbr = frame_start
		self._setup_pipe()

	def _setup_pipe(self):
		try:
			if os.path.exists(self.pipe_path):
				file_stat = os.stat(self.pipe_path)
				if not stat.S_ISFIFO(file_stat.st_mode):
					os.remove(self.pipe_path)
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

			for traffic_sign, accuracy, x, y, w, h in dets:
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
