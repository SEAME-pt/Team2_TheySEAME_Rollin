import numpy as np

from .lane_model import curvature, eval_curve, nearest_widths


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


	# Parameters for the Lane Fitting EMA
	_EMA_PREV_LANE_WEIGHT = 0.5  # weight for the previous lane in the EMA

	# Horizontal momentum carried into windows that find no pixels. Decayed each
	# empty step so a long blind stretch coasts to a stop instead of flying off.
	_MOMENTUM_DECAY = 0.7
	# Point-count thresholds for the polynomial degree. A quadratic needs enough
	# real detections or it overfits a near-straight stub into a wild curl.
	_MIN_PTS_QUADRATIC = 4
	_MIN_PTS_LINEAR = 2

	# validate_lanes thresholds
	_MAX_CURVATURE   = 1.0 / 12   # reject lane if turn radius < 12 px
	_MAX_WIDTH_CV    = 0.35        # reject lane pair if width CV exceeds this
	_COINCIDENT_FRAC = 0.08        # treat lanes as same line if bottom sep < 8% of W

	def __init__(self, frame_width: int):
		self._prev_left_pts = None
		self._prev_right_pts = None
		self._prev_left_coeffs = None
		self._prev_right_coeffs = None
		self.frame_width = frame_width

	def get_lane_points(
		self,
		frame: np.ndarray,
		n_points: int,
	) -> tuple[list[tuple[int, int]], list[tuple[int, int]], list[tuple[int, int]], list[tuple[int, int]]]:
		"""
		Returns (left_pts, right_pts) — each a list of up to n_points (x, y) tuples
		in BEV-frame coordinates, ordered bottom-to-top. A lane with too few
		detections to fit a curve returns an empty list.
		"""
		# Use only the bottom third of the frame so peaks reflect lane positions
		# near the car. A full-frame sum misplaces the peak on sharp curves,
		# causing the right lane to be labeled left and wrong-side virtual synthesis.
		search_rows = frame.shape[0] // 1
		histogram = frame[-search_rows:].sum(axis=0).astype(np.int64)
		rect_w = frame.shape[1] // 12

		hist = histogram.copy()
		x1 = self._get_lane_x(hist)
		x2 = self._get_lane_x(hist)
		# left_x, right_x = (x1, x2) if x1 <= x2 else (x2, x1)
		if (x1 <= self.frame_width // 2 and x2 > self.frame_width // 2):
			left_x, right_x = x1, x2
		elif (x2 <= self.frame_width // 2 and x1 > self.frame_width // 2):
			left_x, right_x = x2, x1
		elif x1 <= self.frame_width // 2 and x2 <= self.frame_width // 2:
			(left_x, right_x) = (x1, None) if x1 >= x2 else (x2	, None)
		elif x1 > self.frame_width // 2 and x2 > self.frame_width // 2:
			(left_x, right_x) = (None, x1) if x1 <= x2 else (None, x2)
		else:
			x1, x2 = None, None

		if left_x:
			left_pts, left_coeffs, left_det_pts = self._sliding_window(frame, left_x, n_points, rect_w, self._prev_left_coeffs)
		else:
			left_pts, left_coeffs, left_det_pts = [], None, []
		if right_x:
			right_pts, right_coeffs, right_det_pts = self._sliding_window(frame, right_x, n_points, rect_w, self._prev_right_coeffs)
		else:
			right_pts, right_coeffs, right_det_pts = [], None, []

		self._prev_left_coeffs = left_coeffs
		self._prev_right_coeffs = right_coeffs
		return left_pts, right_pts, left_det_pts, right_det_pts

	def _smooth_lane(self, coeffs, prev_coeffs):
		if prev_coeffs is None or len(prev_coeffs) != len(coeffs):
			return coeffs
		ema_coeffs = (1 - self._EMA_PREV_LANE_WEIGHT) * np.array(coeffs) + self._EMA_PREV_LANE_WEIGHT * np.array(prev_coeffs)
		return ema_coeffs

	def _get_lane_x(self, histogram: np.ndarray) -> int:
		peak = int(np.argmax(histogram))
		peak_val = histogram[peak]
		if peak_val == 0:
			return peak
		# Walk outward from the peak until the histogram falls to 10% of peak height,
		# then clear the whole lobe + 20px padding so a curved lane (whose projection
		# spans many columns) doesn't get detected twice.
		threshold = max(1, int(peak_val * 0.1))
		left = peak
		while left > 0 and histogram[left - 1] >= threshold:
			left -= 1
		right = peak
		while right < len(histogram) - 1 and histogram[right + 1] >= threshold:
			right += 1
		pad = 20
		histogram[max(0, left - pad):min(len(histogram), right + pad + 1)] = 0
		return peak

	def _sliding_window(
		self,
		frame: np.ndarray,
		start_x: int,
		n_points: int,
		rect_w: int,
		prev_coeffs: "np.ndarray | None" = None,
	) -> "tuple[list[tuple[int, int]], np.ndarray | None]":
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

		pts, coeffs = self._fit_and_generate(det_x, det_y, row_ys, w, prev_coeffs)
		det_pts = list(zip(det_x, det_y))
		return pts, coeffs, det_pts

	def _fit_and_generate(
		self,
		det_x: list[int],
		det_y: list[int],
		row_ys: list[int],
		w: int,
		prev_coeffs: "np.ndarray | None" = None,
	) -> "tuple[list[tuple[int, int]], np.ndarray | None]":
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
			return [], None

		try:
			coeffs = np.polyfit(np.asarray(det_y, dtype=float),
			                    np.asarray(det_x, dtype=float), degree)
			ema_coeffs = self._smooth_lane(coeffs, prev_coeffs)
		except (np.linalg.LinAlgError, ValueError):
			return [], None

		poly = np.poly1d(ema_coeffs)
		points: list[tuple[int, int]] = []
		for yc in row_ys:
			xv = int(round(float(poly(yc))))
			xv = max(0, min(w - 1, xv))
			points.append((xv, yc))
		return points, ema_coeffs

	def validate_lanes(
		self,
		left: "np.ndarray | None",
		right: "np.ndarray | None",
		h: int,
		w: int,
	) -> "tuple[np.ndarray | None, np.ndarray | None]":
		"""
		Filter implausible lane fits.  Returns (left, right) with None for any
		lane that fails a geometric sanity check.

		Gates (applied in order — a lane nulled early skips later gates):
		1. Curvature: reject if turn radius < 12 px.
		2. Coincidence: if both curves share the same physical line, drop the
		   weaker one so VirtualLane can synthesise the missing side (Bug 1).
		3. Crossing: drop the lane that ends up on the wrong side of the other.
		4. Width consistency: drop the lane causing diverging lane width.
		"""
		# Gate 1: per-lane curvature sanity
		if left is not None:
			if self._max_curvature(np.asarray(left, dtype=float)) > self._MAX_CURVATURE:
				print("Rejecting left lane for excessive curvature")
				left = None
		if right is not None:
			if self._max_curvature(np.asarray(right, dtype=float)) > self._MAX_CURVATURE:
				print("Rejecting right lane for excessive curvature")
				right = None

		# Gate 2: coincidence — same physical line tracked twice
		if left is not None and right is not None:
			lc = np.asarray(left, dtype=float)
			rc = np.asarray(right, dtype=float)
			lx0 = float(eval_curve(lc, 0.0)[0])
			rx0 = float(eval_curve(rc, 0.0)[0])
			if abs(rx0 - lx0) < self._COINCIDENT_FRAC * w:
				if self._max_curvature(lc) > self._max_curvature(rc):
					print("Rejecting left lane for coincidence with right lane")
					left = None
				else:
					print("Rejecting right lane for coincidence with left lane")
					right = None

		# Gate 3: crossing
		if left is not None and right is not None:
			lc = np.asarray(left, dtype=float)
			rc = np.asarray(right, dtype=float)
			ts = np.linspace(0.0, 1.0, 7)
			lxs = eval_curve(lc, ts)[0]
			rxs = eval_curve(rc, ts)[0]
			if np.any(rxs < lxs):
				if self._max_curvature(lc) > self._max_curvature(rc):
					print("Rejecting left lane for crossing right lane")
					left = None
				else:
					print("Rejecting right lane for crossing left lane")
					right = None

		# Gate 4: width consistency
		if left is not None and right is not None:
			lc = np.asarray(left, dtype=float)
			rc = np.asarray(right, dtype=float)
			ts = np.linspace(0.0, 1.0, 10)
			lx, ly = eval_curve(lc, ts)
			rx, ry = eval_curve(rc, ts)
			widths = nearest_widths(lx, ly, rx, ry)
			if len(widths) > 1 and float(widths.mean()) > 0:
				cv = float(widths.std() / widths.mean())
				if cv > self._MAX_WIDTH_CV:
					l_range = float(np.max(lx) - np.min(lx))
					r_range = float(np.max(rx) - np.min(rx))
					if l_range >= r_range:
						print("Rejecting left lane for width inconsistency")
						left = None
					else:
						print("Rejecting right lane for width inconsistency")
						right = None

		return left, right

	@staticmethod
	def _max_curvature(coeffs: np.ndarray) -> float:
		k = curvature(coeffs, np.linspace(0.0, 1.0, 11))
		finite = np.abs(k[np.isfinite(k)])
		return float(finite.max()) if len(finite) > 0 else 0.0
