import numpy as np

from .lane_model import eval_curve, fit_parametric, nearest_widths, unit_normal


class VirtualLane:
	"""
	Synthesizes a missing lane by offsetting the surviving lane by one lane
	width. Operates in BEV space, where the perspective is removed and the two
	real lanes are parallel curves of near-identical curvature — so the missing
	lane shares the present lane's shape and differs only in lateral position.

	Lanes are *parametric* quadratics ``(x(t), y(t))`` stored as ``np.ndarray`` of
	shape (2, 3) (see :mod:`lane_model`), or ``None`` when undetected — the
	convention used by ``lane_viz.fit_lane`` / ``sliding_window``. The parametric
	form lets the offset stay perpendicular even when the lane turns steep.

	The class is stateful across frames so it can:
	  - learn the lane width from frames where both lanes are visible (EMA),
	  - debounce loss so a brief miss does not flicker a lane in and out,
	  - de-duplicate the phantom second curve that ``SlidingWindow`` can emit when
	    only one real lane exists.
	"""

	# EMA weight for newly measured widths. Low => slow, stable width estimate.
	_EMA_ALPHA = 0.1
	# A lane must be absent this many consecutive frames before it is synthesized,
	# so intermittent detection does not flicker the virtual lane on and off.
	_LOST_FRAMES = 3
	# Two fits whose bottom-end separation is below this fraction of the lane
	# width are treated as the same physical lane found twice (phantom peak).
	_MIN_WIDTH_FRAC = 0.2
	# Cold-start fallback width as a fraction of the BEV canvas width, used until
	# both lanes have been seen together at least once.
	_DEFAULT_WIDTH_FRAC = 0.375
	# Samples along t used when measuring width / refitting an offset curve.
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
			lbx = self._eval(left, 0.0)[0]
			rbx = self._eval(right, 0.0)[0]
			sep = abs(rbx - lbx)
			min_sep = self._MIN_WIDTH_FRAC * self._current_width(out_w)
			if sep < min_sep:
				# Keep the curve closer to its expected side of the frame.
				if lbx <= rbx:
					right = None
				else:
					left = None

		# 2. Learn width when two genuine lanes remain.
		if left is not None and right is not None:
			self._learn_width(left, right)
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
	def _eval(coeffs: np.ndarray, t: float) -> tuple:
		x, y = eval_curve(coeffs, t)
		return float(x), float(y)

	def _current_width(self, out_w: int) -> float:
		if self._width_ema is not None:
			return self._width_ema
		return self._DEFAULT_WIDTH_FRAC * out_w

	def _learn_width(self, left: np.ndarray, right: np.ndarray) -> None:
		ts = np.linspace(0.0, 1.0, self._N_SAMPLES)
		lx, ly = eval_curve(left, ts)
		rx, ry = eval_curve(right, ts)
		measured = float(np.mean(nearest_widths(lx, ly, rx, ry)))
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
		Offset ``present`` by a constant perpendicular distance ``width`` along its
		per-sample unit normal, then refit a parametric quadratic so the result
		stays a plain (2, 3) like every other lane. ``sign`` = +1 offsets to the
		right (left lane present), -1 to the left (right lane present).
		"""
		ts = np.linspace(0.0, 1.0, self._N_SAMPLES)
		x, y = eval_curve(present, ts)
		nx, ny = unit_normal(present, ts)
		xo = np.clip(x + sign * width * nx, 0, out_w - 1)
		yo = np.clip(y + sign * width * ny, 0, out_h - 1)
		coeffs = fit_parametric(np.column_stack((xo, yo)), degree=2)
		if coeffs is None:
			# Degenerate fit: fall back to a flat lateral shift of the curve.
			shifted = present.copy()
			shifted[0][2] += sign * width
			return shifted
		return coeffs
