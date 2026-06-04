"""
Shared lane-curve primitives.

A lane is represented as a *parametric* quadratic in normalized arc-length
``t in [0, 1]``::

    x(t) = ax*t**2 + bx*t + cx
    y(t) = ay*t**2 + by*t + cy

stored as an ``np.ndarray`` of shape ``(2, 3)`` — row 0 = x coefficients
``[ax, bx, cx]`` (np.polyval order, highest power first), row 1 = y coefficients
— or ``None`` when the lane is undetected.

Why parametric instead of the older ``x = f(y)`` quadratic: the parametric form
can represent a lane of *any* orientation, including the near-horizontal /
vertical-tangent curves produced by steep turns in the bird's-eye view, and it
yields an analytic tangent and curvature (used by the sliding-window tangent
follower, the validation checks, and the virtual-lane perpendicular offset).

``t = 0`` is the bottom (ego) end of the lane, ``t = 1`` the far end.
"""

import numpy as np


def arc_t(xs: np.ndarray, ys: np.ndarray) -> np.ndarray:
	"""Normalized cumulative chord length (0..1) for an ordered point sequence."""
	dx = np.diff(xs)
	dy = np.diff(ys)
	seg = np.sqrt(dx * dx + dy * dy)
	cum = np.concatenate(([0.0], np.cumsum(seg)))
	total = cum[-1]
	if total <= 0:
		# All points coincide — spread them evenly so polyfit stays defined.
		return np.linspace(0.0, 1.0, len(xs))
	return cum / total


def fit_parametric(points, degree: int = 2) -> "np.ndarray | None":
	"""
	Fit ``x(t)`` and ``y(t)`` to an ordered list of ``(x, y)`` points and return
	the ``(2, 3)`` coefficient array. ``degree`` may be 1 (line) or 2 (quadratic);
	a linear fit is padded with a leading zero so the shape is always ``(2, 3)``.
	Returns ``None`` if the fit is degenerate.
	"""
	if points is None or len(points) < 2:
		return None
	arr = np.asarray(points, dtype=float)
	xs, ys = arr[:, 0], arr[:, 1]
	t = arc_t(xs, ys)
	try:
		cx = np.polyfit(t, xs, degree)
		cy = np.polyfit(t, ys, degree)
	except (np.linalg.LinAlgError, ValueError):
		return None
	if degree < 2:
		cx = np.concatenate(([0.0], cx))
		cy = np.concatenate(([0.0], cy))
	coeffs = np.vstack((cx, cy))
	if not np.all(np.isfinite(coeffs)):
		return None
	return coeffs


def eval_curve(coeffs: np.ndarray, t) -> tuple:
	"""Return ``(x, y)`` (arrays or scalars) at parameter value(s) ``t``."""
	x = np.polyval(coeffs[0], t)
	y = np.polyval(coeffs[1], t)
	return x, y


def tangent(coeffs: np.ndarray, t):
	"""Return ``(dx/dt, dy/dt)`` at ``t`` — the un-normalized tangent vector."""
	dcx = np.polyder(coeffs[0])
	dcy = np.polyder(coeffs[1])
	return np.polyval(dcx, t), np.polyval(dcy, t)


def curvature(coeffs: np.ndarray, t) -> np.ndarray:
	"""
	Signed curvature kappa at ``t``::

	    kappa = (x'*y'' - y'*x'') / (x'**2 + y'**2)**1.5

	The radius of the turn is ``1 / |kappa|``.
	"""
	dx, dy = tangent(coeffs, t)
	ddx = np.polyval(np.polyder(coeffs[0], 2), t)
	ddy = np.polyval(np.polyder(coeffs[1], 2), t)
	denom = (dx * dx + dy * dy) ** 1.5
	with np.errstate(divide="ignore", invalid="ignore"):
		k = (dx * ddy - dy * ddx) / denom
	return np.asarray(k)


def unit_normal(coeffs: np.ndarray, t):
	"""
	Left-hand unit normal ``(nx, ny)`` at ``t`` (the tangent rotated +90 deg).
	Degenerate (zero-length tangent) samples fall back to ``(1, 0)``.
	"""
	dx, dy = tangent(coeffs, t)
	mag = np.sqrt(dx * dx + dy * dy)
	mag = np.where(mag <= 1e-9, 1.0, mag)
	nx = -dy / mag
	ny = dx / mag
	# Where the tangent vanished, point laterally so an offset still moves.
	bad = (np.abs(dx) + np.abs(dy)) <= 1e-9
	nx = np.where(bad, 1.0, nx)
	ny = np.where(bad, 0.0, ny)
	return nx, ny


def nearest_widths(lx, ly, rx, ry) -> np.ndarray:
	"""
	Orientation-agnostic spacing between two sampled curves: the nearest-neighbour
	distance from each left sample ``(lx, ly)`` to the right curve ``(rx, ry)``.
	Used both for width learning and width-consistency validation.
	"""
	lx = np.asarray(lx); ly = np.asarray(ly)
	rx = np.asarray(rx); ry = np.asarray(ry)
	dx = lx[:, None] - rx[None, :]
	dy = ly[:, None] - ry[None, :]
	dist = np.sqrt(dx * dx + dy * dy)
	return np.min(dist, axis=1)


def sample_xy(coeffs: np.ndarray, n: int) -> tuple:
	"""Evaluate the curve at ``n`` uniform ``t`` in [0, 1]; return float (xs, ys)."""
	t = np.linspace(0.0, 1.0, n)
	return eval_curve(coeffs, t)


def resample(coeffs: np.ndarray, n: int, w: "int | None" = None,
             h: "int | None" = None) -> list:
	"""
	Sample ``n`` integer ``(x, y)`` points along the curve, bottom (t=0) to top
	(t=1). Optionally clip to a ``w`` x ``h`` canvas.
	"""
	xs, ys = sample_xy(coeffs, n)
	if w is not None:
		xs = np.clip(xs, 0, w - 1)
	if h is not None:
		ys = np.clip(ys, 0, h - 1)
	return [(int(round(x)), int(round(y))) for x, y in zip(xs, ys)]
