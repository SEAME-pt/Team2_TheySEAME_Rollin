import math

import numpy as np

from .lane_model import curvature, eval_curve, fit_parametric, nearest_widths, resample


class SlidingWindow:
	"""
	Sliding window lane point extractor with a *tangent-following* search.

	The window walk does *association* only — it decides which white pixels belong
	to each lane and follows the lane up the image. Unlike the original vertical
	walk (which stepped straight up and searched a thin horizontal slab, losing
	lanes that turn steep), this version steps along the lane's local **tangent**:
	after each detection the search direction is updated from the movement vector,
	so the windows curve sideways to chase near-horizontal lanes produced by sharp
	turns in the bird's-eye view.

	The detections are then fit to a *parametric* quadratic ``(x(t), y(t))`` (see
	:mod:`lane_model`) and resampled, so the returned points trace the real lane
	curvature regardless of orientation.

	Expects a binary frame with WHITE_PIXEL == 1 (matching C++ Frame.hpp constant).
	"""

	# Point-count thresholds for the polynomial degree. A quadratic needs enough
	# real detections or it overfits a near-straight stub into a wild curl.
	_MIN_PTS_QUADRATIC = 6
	_MIN_PTS_LINEAR = 3
	# Walk control.
	_MAX_TURN = math.radians(45.0)   # max heading change per step (rejects jumps)
	_TURN_SMOOTH = 0.5               # 0 = snap to new heading, 1 = keep old heading
	_MAX_MISSES = 3                  # consecutive empty windows before giving up
	_STEP_OVERSHOOT = 3             # walk up to this * n_points windows along a curve

	# ── validation thresholds (heuristic, BEV pixel space — calibrate on footage) ─
	_N_SAMPLES = 20
	# Smallest believable turn radius in BEV pixels; tighter => curvature spike.
	_R_MIN_PX = 12.0
	# Lanes whose lateral separation drops below this (px) are treated as crossing.
	_MIN_SEP = 4.0
	# Max coefficient of variation of lane width before the pair is inconsistent.
	_WIDTH_CV_TOL = 0.35

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

		hist = histogram.copy()
		lane_x1 = self._get_lane_x(hist)
		lane_x2 = self._get_lane_x(hist)

		pts1 = self._sliding_window(frame, lane_x1, n_points)
		pts2 = self._sliding_window(frame, lane_x2, n_points)

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
	) -> list[tuple[int, int]]:
		h, w = frame.shape
		step = max(8, h // n_points)
		half = step // 2

		# Position and heading (unit vector). Start at the bottom, pointing up.
		px, py = float(start_x), float(h - 1)
		dx, dy = 0.0, -1.0
		det: list[tuple[float, float]] = []
		misses = 0

		# Anchor t=0 by sampling the seed location at the bottom first.
		seed = self._window_centroid(frame, px, py, half)
		if seed is not None:
			px, py = seed
			det.append(seed)

		max_steps = self._STEP_OVERSHOOT * n_points
		for _ in range(max_steps):
			# Predict where the lane continues along the current tangent.
			cx = px + step * dx
			cy = py + step * dy
			if cx < -half or cx > w - 1 + half or cy < -half or cy > h - 1 + half:
				break

			centroid = self._window_centroid(frame, cx, cy, half)
			if centroid is not None:
				mx, my = centroid
				vx, vy = mx - px, my - py
				vmag = math.hypot(vx, vy)
				if vmag > 1e-6:
					dx, dy = self._steer(dx, dy, vx / vmag, vy / vmag)
				px, py = mx, my
				det.append(centroid)
				misses = 0
			else:
				# No evidence: coast along the current heading and count the miss.
				px, py = cx, cy
				misses += 1
				if misses >= self._MAX_MISSES:
					break

		return self._fit_and_resample(det, n_points, w, h)

	def _window_centroid(
		self, frame: np.ndarray, cx: float, cy: float, half: int
	) -> "tuple[float, float] | None":
		"""Centroid of white pixels in a square window centred at (cx, cy)."""
		h, w = frame.shape
		x1 = int(max(0, round(cx - half)))
		x2 = int(min(w, round(cx + half) + 1))
		y1 = int(max(0, round(cy - half)))
		y2 = int(min(h, round(cy + half) + 1))
		if x2 <= x1 or y2 <= y1:
			return None
		roi = frame[y1:y2, x1:x2]
		ys_idx, xs_idx = np.where(roi == 1)
		if xs_idx.size == 0:
			return None
		return (x1 + float(xs_idx.mean()), y1 + float(ys_idx.mean()))

	def _steer(self, dx: float, dy: float, nx: float, ny: float) -> tuple[float, float]:
		"""
		Blend the current heading toward the measured movement direction, clamping
		the turn to ``_MAX_TURN`` per step so a stray pixel cluster cannot whip the
		search off the lane.
		"""
		a_old = math.atan2(dy, dx)
		a_new = math.atan2(ny, nx)
		da = math.atan2(math.sin(a_new - a_old), math.cos(a_new - a_old))
		da *= (1.0 - self._TURN_SMOOTH)
		da = max(-self._MAX_TURN, min(self._MAX_TURN, da))
		a = a_old + da
		return math.cos(a), math.sin(a)

	def _fit_and_resample(
		self,
		det: list[tuple[float, float]],
		n_points: int,
		w: int,
		h: int,
	) -> list[tuple[int, int]]:
		"""
		Fit a parametric quadratic to the ordered detections and resample it at
		``n_points`` points, bottom-to-top. Degree drops to 1 when detections are
		too few to trust a quadratic; too few even for a line returns [].
		"""
		n = len(det)
		if n >= self._MIN_PTS_QUADRATIC:
			degree = 2
		elif n >= self._MIN_PTS_LINEAR:
			degree = 1
		else:
			return []

		coeffs = fit_parametric(det, degree)
		if coeffs is None:
			return []
		return resample(coeffs, n_points, w, h)

	# ── validation ───────────────────────────────────────────────────────────

	def validate_lanes(
		self,
		left: "np.ndarray | None",
		right: "np.ndarray | None",
		out_h: int,
		out_w: int,
	) -> "tuple[np.ndarray | None, np.ndarray | None]":
		"""
		Reject geometrically impossible lanes, returning ``(left, right)`` with any
		failing lane replaced by ``None`` (so ``VirtualLane`` can re-synthesize it).

		Checks, in BEV pixel space:
		  * Curvature sanity — drop a lane whose turn radius dips below
		    ``_R_MIN_PX`` anywhere, or whose curvature is non-finite.
		  * Non-crossing — if the left/right boundaries touch or cross inside the
		    canvas, drop the more suspect (more sharply curved) lane.
		  * Width consistency — if the spacing between the lanes varies more than
		    ``_WIDTH_CV_TOL`` (coefficient of variation), drop the more suspect lane.
		"""
		left = None if left is None else np.asarray(left, dtype=float)
		right = None if right is None else np.asarray(right, dtype=float)

		# 1. Per-lane curvature sanity.
		if left is not None and not self._curvature_ok(left):
			left = None
		if right is not None and not self._curvature_ok(right):
			right = None

		if left is None or right is None:
			return left, right

		ts = np.linspace(0.0, 1.0, self._N_SAMPLES)
		lx, ly = eval_curve(left, ts)
		rx, ry = eval_curve(right, ts)

		# 2. Non-crossing: left must stay left of right across the canvas.
		sep = rx - lx
		if np.min(sep) < self._MIN_SEP:
			if self._more_suspect(left, right) == "left":
				return None, right
			return left, None

		# 3. Width consistency via nearest-point spacing (orientation-agnostic).
		widths = nearest_widths(lx, ly, rx, ry)
		mean = float(np.mean(widths))
		if mean > 1e-6:
			cv = float(np.std(widths)) / mean
			if cv > self._WIDTH_CV_TOL:
				if self._more_suspect(left, right) == "left":
					return None, right
				return left, None

		return left, right

	def _curvature_ok(self, coeffs: np.ndarray) -> bool:
		ts = np.linspace(0.0, 1.0, self._N_SAMPLES)
		k = curvature(coeffs, ts)
		if not np.all(np.isfinite(k)):
			return False
		return float(np.max(np.abs(k))) <= 1.0 / self._R_MIN_PX

	def _max_abs_curvature(self, coeffs: np.ndarray) -> float:
		ts = np.linspace(0.0, 1.0, self._N_SAMPLES)
		k = curvature(coeffs, ts)
		k = k[np.isfinite(k)]
		return float(np.max(np.abs(k))) if k.size else float("inf")

	def _more_suspect(self, left: np.ndarray, right: np.ndarray) -> str:
		"""The lane with the higher peak curvature is the more suspect one."""
		return "left" if self._max_abs_curvature(left) >= self._max_abs_curvature(right) else "right"
