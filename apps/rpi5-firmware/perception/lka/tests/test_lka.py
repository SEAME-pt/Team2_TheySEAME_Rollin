"""
Pure-numpy unit tests for the LKA lane pipeline — no camera / Hailo / cv2 needed.

Run from the perception directory:

    python -m pytest lka/tests/ -v
    # or, without pytest:
    python -m lka.tests.test_lka
"""

import math

import numpy as np

from lka.lane_model import (
	arc_t,
	curvature,
	fit_parametric,
	resample,
	sample_xy,
)
from lka.sliding_window import SlidingWindow
from lka.virtual_lane import VirtualLane

H, W = 440, 640


# ── helpers ──────────────────────────────────────────────────────────────────

def _blank():
	return np.zeros((H, W), dtype=np.uint8)


def _paint(frame, xs, ys, r=3):
	"""Paint a thick band of 1s along the given pixel path."""
	for x, y in zip(xs, ys):
		xi, yi = int(round(x)), int(round(y))
		x0, x1 = max(0, xi - r), min(W, xi + r + 1)
		y0, y1 = max(0, yi - r), min(H, yi + r + 1)
		frame[y0:y1, x0:x1] = 1


def _vertical_points(x, n=8, y_top=30, y_bot=430):
	ys = np.linspace(y_bot, y_top, n)
	return [(float(x), float(y)) for y in ys]


def _populated(pts_pair):
	"""Return whichever of the (left, right) lists has points."""
	left, right = pts_pair
	return left if len(left) >= len(right) else right


# ── lane_model ───────────────────────────────────────────────────────────────

def test_arc_t_monotonic_normalized():
	xs = np.array([0.0, 3.0, 3.0])
	ys = np.array([0.0, 0.0, 4.0])  # lengths 3 then 4 -> total 7
	t = arc_t(xs, ys)
	assert t[0] == 0.0 and abs(t[-1] - 1.0) < 1e-9
	assert np.all(np.diff(t) > 0)
	assert abs(t[1] - 3.0 / 7.0) < 1e-9


def test_fit_parametric_roundtrip():
	# Sample a known arc, fit, and check the fitted curve reproduces it
	# *geometrically* (arc_t re-parameterizes by chord length, so point indices
	# need not line up — the shape must).
	t = np.linspace(0, 1, 100)
	xs = 200 + 150 * t
	ys = 430 - 380 * t + 40 * t ** 2
	coeffs = fit_parametric(list(zip(xs, ys)), degree=2)
	assert coeffs is not None and coeffs.shape == (2, 3)
	rx, ry = sample_xy(coeffs, 25)
	# Each fitted point lies on the original dense curve.
	for fx, fy in zip(rx, ry):
		d = np.min(np.sqrt((xs - fx) ** 2 + (ys - fy) ** 2))
		assert d < 4.0, d


def test_curvature_matches_circle():
	# A circular arc of radius R has curvature ~ 1/R.
	R = 100.0
	phi = np.linspace(-math.pi / 4, math.pi / 4, 40)
	xs = 320 + R * np.cos(phi)
	ys = 220 + R * np.sin(phi)
	coeffs = fit_parametric(list(zip(xs, ys)), degree=2)
	k = np.abs(curvature(coeffs, np.linspace(0, 1, 11)))
	assert abs(np.median(k) - 1.0 / R) < 0.005


# ── sliding window: tangent following ────────────────────────────────────────

def test_straight_vertical_lane_tracks():
	frame = _blank()
	_paint(frame, [200] * 50, np.linspace(430, 20, 50), r=4)
	pts = _populated(SlidingWindow().get_lane_points(frame, n_points=8))
	assert len(pts) >= 6
	xs = [p[0] for p in pts]
	ys = [p[1] for p in pts]
	assert max(abs(x - 200) for x in xs) < 20          # stays on the line
	assert max(ys) - min(ys) > 250                       # spans the height


def test_steep_curve_followed_into_horizontal():
	"""Arc that is vertical at the bottom and bends to horizontal at the top.

	The old vertical-only walk would stay near the bottom x and never reach the
	horizontal left end; the tangent follower should track all the way around.
	"""
	frame = _blank()
	x_b, y_b, R = 480, 430, 300
	phi = np.linspace(0.0, -math.pi / 2, 120)         # vertical -> horizontal
	xs = x_b - R + R * np.cos(phi)                      # 480 -> 180
	ys = y_b + R * np.sin(phi)                          # 430 -> 130
	_paint(frame, xs, ys, r=4)

	pts = _populated(SlidingWindow().get_lane_points(frame, n_points=8))
	assert len(pts) >= 6
	px = [p[0] for p in pts]
	py = [p[1] for p in pts]
	# Reached the far, near-horizontal end of the arc (top-left), proving the
	# search followed the tangent sideways instead of marching straight up.
	assert min(px) < 300
	assert min(py) < 230


# ── validate_lanes ───────────────────────────────────────────────────────────

def _coeffs(points):
	return fit_parametric(points, degree=2)


def test_validate_parallel_lanes_pass():
	sw = SlidingWindow()
	left = _coeffs(_vertical_points(200))
	right = _coeffs(_vertical_points(400))
	lo, ro = sw.validate_lanes(left, right, H, W)
	assert lo is not None and ro is not None


def test_validate_rejects_curvature_spike():
	sw = SlidingWindow()
	# Tight half-circle, radius 5 px -> curvature 0.2 >> 1/12.
	phi = np.linspace(0, math.pi, 30)
	xs = 300 + 5 * np.cos(phi)
	ys = 300 + 5 * np.sin(phi)
	spike = _coeffs(list(zip(xs, ys)))
	lo, ro = sw.validate_lanes(spike, None, H, W)
	assert lo is None


def test_validate_rejects_crossing():
	sw = SlidingWindow()
	left = _coeffs([(200, 430), (300, 230), (400, 30)])    # rises to the right
	right = _coeffs([(400, 430), (300, 230), (200, 30)])   # rises to the left
	lo, ro = sw.validate_lanes(left, right, H, W)
	assert (lo is None) != (ro is None)                    # exactly one dropped


def test_validate_rejects_inconsistent_width():
	sw = SlidingWindow()
	left = _coeffs(_vertical_points(200))
	right = _coeffs([(260, 430), (410, 230), (560, 30)])   # strongly diverging
	lo, ro = sw.validate_lanes(left, right, H, W)
	assert (lo is None) != (ro is None)


def test_validate_drops_coincident():
	sw = SlidingWindow()
	a = _coeffs(_vertical_points(200))
	b = _coeffs(_vertical_points(208))   # 8 px apart << 0.08 * 640 = 51 px
	lo, ro = sw.validate_lanes(a, b, H, W)
	assert (lo is None) != (ro is None)  # exactly one survives; VirtualLane takes over


# ── virtual lane ─────────────────────────────────────────────────────────────

def test_virtual_offset_synthesizes_missing_lane():
	vl = VirtualLane()
	left = _coeffs(_vertical_points(200))
	out = (None, None, None)
	for _ in range(VirtualLane._LOST_FRAMES):
		out = vl.update(left, None, H, W)
	l_out, r_out, side = out
	assert side == "right" and r_out is not None
	# Default width = 0.375 * W = 240, so the synthesized right lane sits near 440.
	x0_right = float(np.polyval(r_out[0], 0.0))
	assert abs(x0_right - (200 + 0.375 * W)) < 40


def test_virtual_dedup_drops_duplicate():
	vl = VirtualLane()
	a = _coeffs(_vertical_points(200))
	b = _coeffs(_vertical_points(205))   # essentially the same lane
	l_out, r_out, side = vl.update(a, b, H, W)
	assert (l_out is None) != (r_out is None)   # one collapsed away


# ── plain-python runner ──────────────────────────────────────────────────────

if __name__ == "__main__":
	fns = [v for k, v in sorted(globals().items()) if k.startswith("test_")]
	failed = 0
	for fn in fns:
		try:
			fn()
			print(f"PASS {fn.__name__}")
		except AssertionError as e:
			failed += 1
			print(f"FAIL {fn.__name__}: {e}")
		except Exception as e:  # noqa: BLE001
			failed += 1
			print(f"ERROR {fn.__name__}: {type(e).__name__}: {e}")
	print(f"\n{len(fns) - failed}/{len(fns)} passed")
	raise SystemExit(1 if failed else 0)
