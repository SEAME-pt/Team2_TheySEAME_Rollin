import numpy as np


class SlidingWindow:
	"""
	Sliding window lane point extractor.
	Port of SlidingWindow.cpp — vectorized with NumPy instead of pixel-by-pixel loops.
	Expects a binary frame with WHITE_PIXEL == 1 (matching C++ Frame.hpp constant).

	The window walk does *association* only — it decides which white pixels belong
	to each lane and follows the lane up the image with horizontal momentum so it
	keeps tracking through gaps (e.g. dashed markings or the upper part of a curve
	the camera barely sees). The returned points are then *generated from a
	polynomial* fitted to the real detections, so empty rows follow the lane's
	curvature instead of being extrapolated straight up.
	"""

	# Horizontal momentum carried into windows that find no pixels. Decayed each
	# empty step so a long blind stretch coasts to a stop instead of flying off.
	_MOMENTUM_DECAY = 0.7
	# Point-count thresholds for the polynomial degree. A quadratic needs enough
	# real detections or it overfits a near-straight stub into a wild curl.
	_MIN_PTS_QUADRATIC = 6
	_MIN_PTS_LINEAR = 2

	def get_lane_points(
		self,
		frame: np.ndarray,
		n_points: int,
	) -> tuple[list[tuple[int, int]], list[tuple[int, int]]]:
		"""
		Returns (left_pts, right_pts) — each a list of up to n_points (x, y) tuples
		in BEV-frame coordinates, ordered bottom-to-top. A lane with too few
		detections to fit a curve returns an empty list.
		"""
		histogram = frame.sum(axis=0).astype(np.int64)
		rect_w = frame.shape[1] // 12

		hist = histogram.copy()
		lane_x1 = self._get_lane_x(hist)
		lane_x2 = self._get_lane_x(hist)

		pts1 = self._sliding_window(frame, lane_x1, n_points, rect_w)
		pts2 = self._sliding_window(frame, lane_x2, n_points, rect_w)

		if lane_x1 > lane_x2:
			pts1, pts2 = pts2, pts1

		return pts1, pts2

	def _get_lane_x(self, histogram: np.ndarray) -> int:
		peak = int(np.argmax(histogram))
		start = max(0, peak - 50)
		end = min(len(histogram), peak + 50)
		histogram[start:end] = 0
		return peak

	def _sliding_window(
		self,
		frame: np.ndarray,
		start_x: int,
		n_points: int,
		rect_w: int,
	) -> list[tuple[int, int]]:
		h, w = frame.shape
		step_y = h // n_points
		x = start_x
		dx = 0.0  # horizontal momentum (px per window step), learned from detections
		y = h - step_y

		row_ys: list[int] = []          # y-centre of every window, bottom-to-top
		det_x: list[int] = []           # x of windows that actually found pixels
		det_y: list[int] = []           # matching y for those detections

		for _ in range(n_points):
			# Place the window where momentum predicts the lane continues, so the
			# search follows the curve into rows the previous window couldn't see.
			x_pred = int(round(x + dx))
			x_pred = max(0, min(w - 1, x_pred))

			x1 = max(0, x_pred - rect_w // 2)
			x2 = min(w, x_pred + rect_w // 2)
			y1 = max(0, y)
			y2 = min(h, y + step_y)
			yc = y + step_y // 2

			roi = frame[y1:y2, x1:x2]
			white_cols = np.where(roi == 1)[1]

			if len(white_cols) > 0:
				measured_x = x1 + int(np.mean(white_cols))
				dx = measured_x - x      # update velocity from real movement
				x = measured_x
				det_x.append(measured_x)
				det_y.append(yc)
			else:
				# No evidence: coast along the learned curve and fade momentum.
				dx *= self._MOMENTUM_DECAY
				x = x_pred

			row_ys.append(yc)
			y -= step_y

		return self._fit_and_generate(det_x, det_y, row_ys, w)

	def _fit_and_generate(
		self,
		det_x: list[int],
		det_y: list[int],
		row_ys: list[int],
		w: int,
	) -> list[tuple[int, int]]:
		"""
		Fit x = f(y) to the real detections and evaluate it at every window row.
		Empty rows are filled by the *curve* (extrapolated), not a straight line.
		Degree drops to 1 when detections are too few to trust a quadratic.
		"""
		n = len(det_x)
		if n >= self._MIN_PTS_QUADRATIC:
			degree = 2
		elif n >= self._MIN_PTS_LINEAR:
			degree = 1
		else:
			return []  # not enough evidence to claim a lane

		try:
			coeffs = np.polyfit(np.asarray(det_y, dtype=float),
			                    np.asarray(det_x, dtype=float), degree)
		except (np.linalg.LinAlgError, ValueError):
			return []

		poly = np.poly1d(coeffs)
		points: list[tuple[int, int]] = []
		for yc in row_ys:
			xv = int(round(float(poly(yc))))
			xv = max(0, min(w - 1, xv))
			points.append((xv, yc))
		return points
