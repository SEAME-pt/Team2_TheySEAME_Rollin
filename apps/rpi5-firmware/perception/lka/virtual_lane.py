import numpy as np


class VirtualLane:
	"""
	Synthesizes a missing lane by offsetting the surviving lane by one lane
	width. Operates in BEV space, where the perspective is removed and the two
	real lanes are parallel curves of near-identical curvature — so the missing
	lane shares the present lane's shape and differs only in lateral position.

	Lanes are quadratics ``x = a*y**2 + b*y + c`` stored as ``np.ndarray`` of
	shape (3,) = [a, b, c], or ``None`` when undetected (the convention used by
	``lane_viz.fit_lane`` / ``sliding_window``).

	The class is stateful across frames so it can:
	  - learn the lane width from frames where both lanes are visible (EMA),
	  - debounce loss so a brief miss does not flicker a lane in and out,
	  - de-duplicate the phantom second histogram peak that ``SlidingWindow``
	    can emit when only one real lane exists.
	"""

	# EMA weight for newly measured widths. Low => slow, stable width estimate.
	_EMA_ALPHA = 0.1
	# A lane must be absent this many consecutive frames before it is synthesized,
	# so intermittent detection does not flicker the virtual lane on and off.
	_LOST_FRAMES = 10
	# Two fits whose bottom-row separation is below this fraction of the lane
	# width are treated as the same physical lane found twice (phantom peak).
	_MIN_WIDTH_FRAC = 0.2
	# Cold-start fallback width as a fraction of the BEV canvas width, used until
	# both lanes have been seen together at least once.
	_DEFAULT_WIDTH_FRAC = 0.375
	# Rows sampled when measuring width / refitting an offset curve.
	_N_SAMPLES = 20

	def __init__(self):
		self._width_ema: "float | None" = None
		self._lost_left = 0
		self._lost_right = 0

	def update(
		self,
		left_coeffs,
		right_coeffs,
		out_h: int,
		out_w: int,
	):
		"""
		Returns ``(left_out, right_out, virtual_side)``.

		``virtual_side`` is ``None``, ``"left"`` or ``"right"`` and tells the
		caller which returned curve is synthesized (for distinct rendering).
		Real fits are passed through unchanged.
		"""
		left = None if left_coeffs is None else np.asarray(left_coeffs, dtype=float)
		right = None if right_coeffs is None else np.asarray(right_coeffs, dtype=float)

		# 1. De-dup: a "both lanes" frame whose curves sit on top of each other is
		#    really one lane found twice. Drop the duplicate so it can be rebuilt
		#    at the correct width below.
		if left is not None and right is not None:
			sep = abs(self._eval(right, out_h - 1) - self._eval(left, out_h - 1))
			min_sep = self._MIN_WIDTH_FRAC * self._current_width(out_w)
			if sep < min_sep:
				# Keep the curve closer to its expected side of the frame.
				if self._eval(left, out_h - 1) <= self._eval(right, out_h - 1):
					right = None
				else:
					left = None

		# 2. Learn width when two genuine lanes remain.
		if left is not None and right is not None:
			self._learn_width(left, right, out_h)
			self._lost_left = 0
			self._lost_right = 0
		else:
			self._lost_left = self._lost_left + 1 if left is None else 0
			self._lost_right = self._lost_right + 1 if right is None else 0

		# 3. Synthesize exactly one missing lane once it has been gone long enough.
		width = self._current_width(out_w)
		virtual_side = None
		if right is None and left is not None and self._lost_right >= self._LOST_FRAMES:
			right = self._offset(left, +1.0, width, out_h, out_w)
			virtual_side = "right"
		elif left is None and right is not None and self._lost_left >= self._LOST_FRAMES:
			left = self._offset(right, -1.0, width, out_h, out_w)
			virtual_side = "left"

		return left, right, virtual_side

	@property
	def width(self) -> "float | None":
		"""Learned perpendicular lane width in BEV pixels, or None at cold start."""
		return self._width_ema

	# ── internals ────────────────────────────────────────────────────────────

	@staticmethod
	def _eval(coeffs: np.ndarray, y: float) -> float:
		return float(np.polyval(coeffs, y))

	@staticmethod
	def _slope(coeffs: np.ndarray, y: np.ndarray) -> np.ndarray:
		# f'(y) = 2*a*y + b
		return 2.0 * coeffs[0] * y + coeffs[1]

	def _current_width(self, out_w: int) -> float:
		if self._width_ema is not None:
			return self._width_ema
		return self._DEFAULT_WIDTH_FRAC * out_w

	def _learn_width(self, left: np.ndarray, right: np.ndarray, out_h: int) -> None:
		ys = np.linspace(0, out_h - 1, self._N_SAMPLES)
		# Perpendicular spacing = horizontal spacing projected onto the lane
		# normal, using the slope of the midline so both lanes share one frame.
		mid = (left + right) / 2.0
		horiz = np.polyval(right, ys) - np.polyval(left, ys)
		perp = horiz / np.sqrt(1.0 + self._slope(mid, ys) ** 2)
		measured = float(np.mean(np.abs(perp)))
		if not np.isfinite(measured) or measured <= 0:
			return
		if self._width_ema is None:
			self._width_ema = measured
		else:
			self._width_ema += self._EMA_ALPHA * (measured - self._width_ema)

	def _offset(
		self,
		present: np.ndarray,
		sign: float,
		width: float,
		out_h: int,
		out_w: int,
	) -> np.ndarray:
		"""
		Slope-corrected lateral offset that keeps a constant *perpendicular*
		distance ``width`` from ``present``:

		    x_v(y) = f(y) + sign * width * sqrt(1 + f'(y)**2)

		Sampled over the rows and refit to a quadratic so the result stays a
		plain [a, b, c] like every other lane fit. ``sign`` = +1 offsets to the
		right (left lane present), -1 to the left (right lane present).
		"""
		ys = np.linspace(0, out_h - 1, self._N_SAMPLES)
		xs = np.polyval(present, ys) + sign * width * np.sqrt(1.0 + self._slope(present, ys) ** 2)
		xs = np.clip(xs, 0, out_w - 1)
		try:
			return np.polyfit(ys, xs, 2)
		except (np.linalg.LinAlgError, ValueError):
			# Degenerate fit: fall back to a flat horizontal shift.
			shifted = present.copy()
			shifted[2] += sign * width
			return shifted
