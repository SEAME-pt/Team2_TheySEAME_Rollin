import cv2
import numpy as np

from .bev import Bev
from .lane_model import fit_parametric, sample_xy


def fit_lane(points: list[tuple[int, int]]) -> "np.ndarray | None":
	"""
	Fit a *parametric* quadratic lane curve to ordered lane points and return the
	(2, 3) coefficient array [[ax, bx, cx], [ay, by, cy]] (see lane_model), or
	None if there are too few points.

	Parametric (x(t), y(t)) replaces the older x = f(y) fit so steep / nearly
	horizontal lanes are represented faithfully. Same convention used across the
	lka package and run_ai_pipeline.py.
	"""
	if points is None or len(points) < 3:
		return None
	return fit_parametric(points, degree=2)


def _bev_to_frame(pts: np.ndarray, M_inv: np.ndarray, roi_sy: int) -> np.ndarray:
	"""Map BEV-space (x, y) points back to original camera frame coordinates."""
	warped = cv2.perspectiveTransform(pts.reshape(-1, 1, 2).astype(np.float32), M_inv)
	warped = warped.reshape(-1, 2)
	warped[:, 1] += roi_sy
	return warped.astype(np.int32)


def draw_lane_overlay(
	frame: np.ndarray,
	bev: Bev,
	left_pts: list[tuple[int, int]],
	right_pts: list[tuple[int, int]],
	left_coeffs: "np.ndarray | None",
	right_coeffs: "np.ndarray | None",
) -> np.ndarray:
	"""
	Draw fitted lane curves and polynomial coefficients onto the camera frame.
	Left lane: green. Right lane: blue. Sliding window centres: smaller circles.
	"""
	_, roi_sy, roi_w, roi_h = bev.roi
	M_inv = bev.reverse_matrix
	out = frame.copy()
	frame_h, frame_w = frame.shape[:2]

	# Fitted curves: sample the parametric curve over t and warp back to camera.
	for coeffs, color in ((left_coeffs, (0, 255, 0)), (right_coeffs, (255, 0, 0))):
		if coeffs is None:
			continue
		x_bev, y_bev = sample_xy(coeffs, 120)
		x_bev = np.clip(x_bev, 0, roi_w - 1)
		y_bev = np.clip(y_bev, 0, roi_h - 1)
		pts_cam = _bev_to_frame(np.column_stack((x_bev, y_bev)), M_inv, roi_sy)
		pts_cam[:, 0] = np.clip(pts_cam[:, 0], 0, frame_w - 1)
		pts_cam[:, 1] = np.clip(pts_cam[:, 1], 0, frame_h - 1)
		cv2.polylines(out, [pts_cam.reshape(-1, 1, 2)], isClosed=False, color=color, thickness=4)

	# Sliding window centre points (smaller circles, slightly dimmer)
	for pts, color in ((left_pts, (0, 180, 0)), (right_pts, (180, 0, 0))):
		if not pts:
			continue
		bev_arr = np.array(pts, dtype=float)
		cam_pts = _bev_to_frame(bev_arr, M_inv, roi_sy)
		for cx, cy in cam_pts:
			if 0 <= cx < frame_w and 0 <= cy < frame_h:
				cv2.circle(out, (int(cx), int(cy)), 5, color, -1)

	return out
