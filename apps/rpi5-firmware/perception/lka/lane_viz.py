import cv2
import numpy as np
from .bev import Bev


def fit_lane(points: list[tuple[int, int]]) -> "np.ndarray | None":
    """
    Fit a degree-2 polynomial x = a*y^2 + b*y + c to lane points.
    Returns [a, b, c] or None if there are too few points.
    x is a function of y — same convention as polyfit_lines() in run_ai_pipeline.py.
    """
    if len(points) < 3:
        return None
    y_vals = np.array([p[1] for p in points], dtype=float)
    x_vals = np.array([p[0] for p in points], dtype=float)
    try:
        return np.polyfit(y_vals, x_vals, 2)
    except (np.linalg.LinAlgError, ValueError):
        return None


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

    # Fitted curves (dense points in BEV space → warp back to camera space)
    for coeffs, color in ((left_coeffs, (0, 255, 0)), (right_coeffs, (255, 0, 0))):
        if coeffs is None:
            continue
        y_bev = np.linspace(0, roi_h - 1, 120)
        x_bev = np.clip(np.polyval(coeffs, y_bev), 0, roi_w - 1)
        pts_cam = _bev_to_frame(np.column_stack((x_bev, y_bev)), M_inv, roi_sy)
        pts_cam[:, 0] = np.clip(pts_cam[:, 0], 0, frame_w - 1)
        pts_cam[:, 1] = np.clip(pts_cam[:, 1], 0, frame_h - 1)
        cv2.polylines(out, [pts_cam.reshape(-1, 1, 2)], isClosed=False, color=color, thickness=4)

    # Sliding window centre points (smaller circles, slightly dimmer)
    for pts, color in ((left_pts, (0, 180, 0)), (right_pts, (180, 0, 0))):
        bev_arr = np.array(pts, dtype=float)
        cam_pts = _bev_to_frame(bev_arr, M_inv, roi_sy)
        for cx, cy in cam_pts:
            if 0 <= cx < frame_w and 0 <= cy < frame_h:
                cv2.circle(out, (int(cx), int(cy)), 5, color, -1)

    # Polynomial coefficient text overlay
    y_text = 220
    for label, coeffs in (("L", left_coeffs), ("R", right_coeffs)):
        if coeffs is not None:
            a, b, c = coeffs
            text = f"{label}: a={a:.2e}  b={b:.3f}  c={c:.1f}"
            text_color = (100, 255, 100) if label == "L" else (255, 100, 100)
        else:
            text = f"{label}: no lane detected"
            text_color = (80, 80, 80)
        cv2.putText(out, text, (10, y_text), cv2.FONT_HERSHEY_SIMPLEX, 0.55, text_color, 2)
        y_text += 28

    return out
