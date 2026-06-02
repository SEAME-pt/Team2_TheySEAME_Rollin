#
# Script to handle AI and camera pipelines
#

import cv2
import numpy as np
import argparse
import sys
from hailo_lib import PostProcessor
from hailo_lib import Inference
from hailo_lib import Camera
from hailo_lib.CameraCarla import CARLACamera
from hailo_lib.NamedPipeWriter import NamedPipeWriter
from lka.bev import Bev
from lka.sliding_window import SlidingWindow
from lka.lane_viz import fit_lane

MODEL_PATH = "./trained_models/yolov8s_40e.hef"
CAM_HEIGHT = 640
CAM_WIDTH = 640
MODEL_HEIGHT = 640
MODEL_WIDTH = 640
DISPLAY_WIDTH = 1280
DISPLAY_HEIGHT = 640
LANE_CLASS_ID = 0
LANE_CONFIDENCE_THRESHOLD = 0.6
MODEL_FAMILY = "yolov8"
DEBUG_EVERY_N_FRAMES = 10

# ── Colour palette ─────────────────────────────────────────────────────────────
_C_LEFT      = (60, 220, 60)    # bright green — left lane
_C_RIGHT     = (60, 140, 255)   # blue-orange  — right lane
_C_LEFT_DIM  = (0, 150, 0)
_C_RIGHT_DIM = (0, 80, 180)
_C_ROI       = (0, 200, 255)    # cyan — ROI boundary
_C_MASK      = (200, 0, 80)     # green — segmentation mask tint
_C_BAR       = (20, 20, 20)     # dark header/footer bar
_C_LABEL     = (190, 190, 190)  # light gray label text
_C_GRID      = (28, 28, 28)     # subtle BEV grid
_C_GOOD      = (60, 220, 60)    # status: lane found
_C_BAD       = (50, 50, 200)    # status: no lane
_C_EGO       = (220, 220, 220)  # ego-vehicle marker


def polyfit_lines(tensor):
	tensor = np.asarray(tensor)
	if tensor.ndim == 4:
		if tensor.shape[0] == 0:
			return np.empty((0, 1, 2), dtype=np.int32)
		lane_mask = tensor[0, :, :, 0] > 0
	elif tensor.ndim == 2:
		lane_mask = tensor > 0
	else:
		return np.empty((0, 1, 2), dtype=np.int32)

	y_coords, x_coords = np.where(lane_mask)
	if x_coords.size < 3:
		return np.empty((0, 1, 2), dtype=np.int32)

	coefficients = np.polyfit(y_coords, x_coords, 2)
	y_values = np.linspace(0, lane_mask.shape[0] - 1, 100)
	x_values = coefficients[0] * y_values**2 + coefficients[1] * y_values + coefficients[2]
	x_values = np.clip(x_values, 0, lane_mask.shape[1] - 1)

	points = np.column_stack((x_values, y_values)).astype(np.int32)
	return points.reshape(-1, 1, 2)


def _bev_to_cam(pts: np.ndarray, M_inv: np.ndarray, roi_sy: int) -> np.ndarray:
	"""Map BEV-space (x, y) points back to original camera frame coordinates."""
	warped = cv2.perspectiveTransform(pts.reshape(-1, 1, 2).astype(np.float32), M_inv)
	warped = warped.reshape(-1, 2)
	warped[:, 1] += roi_sy
	return warped.astype(np.int32)


def _build_left_panel(
	frame: np.ndarray,
	lane_mask,
	bev: "Bev | None",
	left_pts: list,
	right_pts: list,
	left_coeffs,
	right_coeffs,
	lane_score: float,
) -> np.ndarray:
	"""Camera view: mask overlay, ROI boundary, polyfit curves, status bar, coeff corner."""
	out = frame.copy()
	h, w = out.shape[:2]

	# Segmentation mask overlay
	if lane_mask is not None and lane_mask.size > 0:
		mask_bin = (lane_mask > 0).astype(np.uint8)
		if mask_bin.shape[:2] != (h, w):
			mask_bin = cv2.resize(mask_bin, (w, h), interpolation=cv2.INTER_NEAREST)
		overlay = np.zeros_like(out)
		overlay[mask_bin > 0] = _C_MASK
		out = cv2.addWeighted(out, 1.0, overlay, 0.4, 0)

	if bev is not None:
		sx, sy, rw, rh = bev.roi
		ow, oh = bev.out_size
		M_inv = bev.reverse_matrix

		# ROI boundary rectangle
		cv2.rectangle(out, (sx, sy), (sx + rw, sy + rh), _C_ROI, 2)
		cv2.putText(out, "ROI", (sx + 6, sy + 18),
		            cv2.FONT_HERSHEY_SIMPLEX, 0.45, _C_ROI, 1)

		# Fitted lane curves (BEV → camera space). x is clipped to the enlarged
		# BEV canvas width (ow), not the ROI width, so curved lanes are kept.
		for coeffs, color in ((left_coeffs, _C_LEFT), (right_coeffs, _C_RIGHT)):
			if coeffs is None:
				continue
			y_bev = np.linspace(0, oh - 1, 150)
			x_bev = np.clip(np.polyval(coeffs, y_bev), 0, ow - 1)
			pts_cam = _bev_to_cam(np.column_stack((x_bev, y_bev)), M_inv, sy)
			pts_cam[:, 0] = np.clip(pts_cam[:, 0], 0, w - 1)
			pts_cam[:, 1] = np.clip(pts_cam[:, 1], 0, h - 1)
			cv2.polylines(out, [pts_cam.reshape(-1, 1, 2)],
			              isClosed=False, color=color, thickness=4)

		# Sliding window center circles
		for pts_list, color in ((left_pts, _C_LEFT_DIM), (right_pts, _C_RIGHT_DIM)):
			if not pts_list:
				continue
			arr = np.array(pts_list, dtype=float)
			cam_pts = _bev_to_cam(arr, M_inv, sy)
			for cx, cy in cam_pts:
				if 0 <= cx < w and 0 <= cy < h:
					cv2.circle(out, (int(cx), int(cy)), 5, color, -1)

	# Top status bar
	bar_h = 40
	cv2.rectangle(out, (0, 0), (w, bar_h), _C_BAR, -1)
	cv2.putText(out, "CAMERA VIEW", (10, 27),
	            cv2.FONT_HERSHEY_SIMPLEX, 0.62, _C_LABEL, 1)
	if lane_score > 0:
		status_txt = f"LANE  {lane_score:.2f}"
		status_col = _C_GOOD
	else:
		status_txt = "NO LANE"
		status_col = _C_BAD
	(tw, _), _ = cv2.getTextSize(status_txt, cv2.FONT_HERSHEY_SIMPLEX, 0.62, 2)
	cv2.putText(out, status_txt, (w - tw - 10, 27),
	            cv2.FONT_HERSHEY_SIMPLEX, 0.62, status_col, 2)

	# Coefficient info box — bottom-left corner
	box_h = 52
	box_w = 350
	cv2.rectangle(out, (0, h - box_h), (box_w, h), (0, 0, 0), -1)
	y_text = h - box_h + 17
	for label, coeffs, color in (("L", left_coeffs, _C_LEFT), ("R", right_coeffs, _C_RIGHT)):
		if coeffs is not None:
			a, b, c = coeffs
			txt = f"{label}: a={a:.2e}  b={b:.3f}  c={c:.1f}"
		else:
			txt = f"{label}: --"
			color = (70, 70, 70)
		cv2.putText(out, txt, (8, y_text),
		            cv2.FONT_HERSHEY_SIMPLEX, 0.42, color, 1)
		y_text += 20

	return out


def _build_right_panel(
	left_pts: list,
	right_pts: list,
	left_coeffs,
	right_coeffs,
	mask,
	bev: "Bev | None",
) -> np.ndarray:
	"""BEV panel: dark background with only sliding-window polynomial tracks."""
	panel_h = DISPLAY_HEIGHT
	panel_w = DISPLAY_WIDTH // 2

	panel = np.full((panel_h, panel_w, 3), 10, dtype=np.uint8)

	if bev is None:
		cv2.putText(panel, "LKA DISABLED",
		            (panel_w // 2 - 110, panel_h // 2),
		            cv2.FONT_HERSHEY_SIMPLEX, 0.9, (80, 80, 80), 2)
		return panel

	# Use the enlarged BEV canvas size (not the ROI) so the mask overlay and the
	# sliding-window points share the same coordinate scaling.
	out_w, out_h = bev.out_size
	header_h = 40
	draw_h = panel_h - header_h
	draw_w = panel_w

	sx_scale = draw_w / out_w
	sy_scale = draw_h / out_h

	def bev_to_panel(x, y):
		return (int(x * sx_scale), header_h + int(y * sy_scale))

	# Subtle grid
	for gx in range(0, out_w + 1, 80):
		px, _ = bev_to_panel(gx, 0)
		cv2.line(panel, (px, header_h), (px, panel_h), _C_GRID, 1)
	for gy in range(0, out_h + 1, 60):
		_, py = bev_to_panel(0, gy)
		cv2.line(panel, (0, py), (draw_w, py), _C_GRID, 1)

	# BEV mask overlay — resize to draw area, convert to BGR, tint in place
	if mask is not None and mask.size > 0:
		mask_resized = cv2.resize(mask, (draw_w, draw_h), interpolation=cv2.INTER_NEAREST)
		if mask_resized.ndim == 2:
			mask_resized = cv2.cvtColor((mask_resized > 0).astype(np.uint8) * 255, cv2.COLOR_GRAY2BGR)
		colored = np.zeros((draw_h, draw_w, 3), dtype=np.uint8)
		colored[mask_resized[:, :, 0] > 0] = _C_MASK
		panel[header_h:, :] = cv2.addWeighted(panel[header_h:, :], 1.0, colored, 0.5, 0)

	# for coeffs, color in ((left_coeffs, _C_LEFT), (right_coeffs, _C_RIGHT)):
	# 	if coeffs is None:
	# 		continue
	# 	y_vals = np.linspace(0, roi_h - 1, 200)
	# 	x_vals = np.clip(np.polyval(coeffs, y_vals), 0, roi_w - 1)
	# 	pts = np.array([bev_to_panel(x, y) for x, y in zip(x_vals, y_vals)], dtype=np.int32)
	# 	cv2.polylines(panel, [pts.reshape(-1, 1, 2)], isClosed=False, color=color, thickness=3)

	# Sliding window center circles
	for pts_list, fill, outline in (
		(left_pts,  _C_LEFT_DIM,  _C_LEFT),
		(right_pts, _C_RIGHT_DIM, _C_RIGHT),
	):
		for x, y in pts_list:
			px, py = bev_to_panel(x, y)
			if 0 <= px < draw_w and header_h <= py < panel_h:
				cv2.circle(panel, (px, py), 7, fill, -1)
				cv2.circle(panel, (px, py), 7, outline, 1)

	# Ego-vehicle marker at bottom-center
	ego_x = draw_w // 2
	ego_y = panel_h - 14
	tri = np.array([
		[ego_x,      ego_y - 16],
		[ego_x - 10, ego_y],
		[ego_x + 10, ego_y],
	], dtype=np.int32)
	cv2.fillPoly(panel, [tri], _C_EGO)
	cv2.putText(panel, "EGO", (ego_x - 14, ego_y + 12),
	            cv2.FONT_HERSHEY_SIMPLEX, 0.35, _C_EGO, 1)

	# Top header bar
	cv2.rectangle(panel, (0, 0), (panel_w, header_h), _C_BAR, -1)
	cv2.putText(panel, "BIRD'S EYE VIEW", (10, 27),
	            cv2.FONT_HERSHEY_SIMPLEX, 0.62, _C_LABEL, 1)

	return panel


def _write_ui(display_proc, left_panel: np.ndarray, right_panel: np.ndarray) -> bool:
	"""Compose side-by-side frame and push to GStreamer display pipe."""
	if display_proc is None or getattr(display_proc, "stdin", None) is None:
		return False
	ui = np.concatenate([left_panel, right_panel], axis=1)
	cv2.line(ui, (DISPLAY_WIDTH // 2, 0), (DISPLAY_WIDTH // 2, DISPLAY_HEIGHT), (60, 60, 60), 2)
	try:
		display_proc.stdin.write(ui.tobytes())
		display_proc.stdin.flush()
		return True
	except BrokenPipeError:
		return False


def _parse_args():
	parser = argparse.ArgumentParser(description="Run AI pipeline with either Pi camera or CARLA camera")
	parser.add_argument(
		"--stream-masks",
		action="store_true",
		help="Write binary mask frames to stdout for consumption by another program",
	)
	parser.add_argument(
		"--named-pipe",
		type=str,
		default=None,
		help="Write lane masks to a named pipe (FIFO) for consumption by another program (e.g., /tmp/lane_mask_pipe)",
	)
	parser.add_argument(
		"--use-carla-camera",
		action="store_true",
		help="Use CARLA TCP camera input instead of Raspberry Pi camera",
	)
	parser.add_argument(
		"--carla-port",
		type=int,
		default=5005,
		help="TCP port used by CARLA camera stream (default: 5005)",
	)
	parser.add_argument(
		"--enable-lka",
		action="store_true",
		help="Enable BEV + sliding window lane detection with polyfit overlay",
	)
	return parser.parse_args()


if __name__ == '__main__':
	args = _parse_args()
	camera = None
	pipe_writer = None
	printed_tensor_info = False
	frame_index = 0
	stream_masks = getattr(args, "stream_masks", False)
	try:
		if args.use_carla_camera:
			camera = CARLACamera(
				CAM_HEIGHT,
				CAM_WIDTH,
				MODEL_HEIGHT,
				MODEL_WIDTH,
				port=args.carla_port,
			)
			print(f"Using CARLA camera on port {args.carla_port}", file=sys.stderr if stream_masks else sys.stdout)
		else:
			camera = Camera(
				CAM_HEIGHT, CAM_WIDTH, MODEL_HEIGHT, MODEL_WIDTH,
				display_width=DISPLAY_WIDTH, display_height=DISPLAY_HEIGHT,
			)
			print("Using Raspberry Pi camera", file=sys.stderr if stream_masks else sys.stdout)

		# Initialize LKA perception modules if requested
		lka_bev = None
		lka_sw = None
		if args.enable_lka:
			# ROI and FOV matching mainLka.cpp: Lka(150, 0, 180, 640, 460, 8)
			lka_bev = Bev(fov=150, roi=(0, 250, CAM_WIDTH, CAM_HEIGHT - 200), margin=0)
			lka_sw = SlidingWindow()

		infer_engine = Inference(camera, MODEL_PATH)
		post_processor = PostProcessor(input_size=(MODEL_HEIGHT, MODEL_WIDTH), strides=(8, 16, 32))
		for frame, infer_results in infer_engine.run_inference():
			frame_index += 1
			lane_result = post_processor.decode(
				infer_results,
				quant_params=infer_engine.get_quant_params(),
				conf_th=LANE_CONFIDENCE_THRESHOLD,
				iou_th=0.5,
			)
			masks = lane_result.get("masks", None)
			scores = lane_result.get("scores", None)
			scales = lane_result.get("scales", None)

			if masks is None:
				lane_mask = None
			else:
				if isinstance(masks, list):
					if len(masks) == 0:
						lane_mask = None
					else:
						try:
							lane_mask = np.any(np.stack(masks, axis=0), axis=0).astype(np.uint8)
						except Exception:
							lane_mask = np.asarray(masks[0])
				else:
					lane_mask = np.asarray(masks)

			if scores is None or (hasattr(scores, "size") and scores.size == 0):
				lane_score = 0.0
			else:
				try:
					lane_score = float(scores[0])
				except Exception:
					lane_score = float(scores)

			if scales is None or (hasattr(scales, "size") and scales.size == 0):
				lane_scale = None
			else:
				try:
					lane_scale = int(scales[0])
				except Exception:
					lane_scale = int(scales)

			if pipe_writer is not None:
				pipe_writer.write_mask(lane_mask, frame_index, lane_score)

			# ── LKA path ──────────────────────────────────────────────────────
			left_pts: list = []
			right_pts: list = []
			left_coeffs = None
			right_coeffs = None
			bev_frame = None

			if lka_bev is not None and lane_mask is not None:
				bev_frame = lka_bev.apply(lane_mask)
				left_pts, right_pts = lka_sw.get_lane_points(bev_frame, n_points=8)
				left_coeffs = fit_lane(left_pts)
				right_coeffs = fit_lane(right_pts)
			elif lka_bev is None and lane_mask is not None:
				# Fallback simple polyfit on camera frame (no BEV)
				points = polyfit_lines(lane_mask)
				if points.size > 0:
					cv2.polylines(frame, [points], isClosed=False, color=_C_LEFT, thickness=5)

			# ── Compose and send UI ────────────────────────────────────────────
			left_panel  = _build_left_panel(
				frame, lane_mask, lka_bev,
				left_pts, right_pts, left_coeffs, right_coeffs, lane_score,
			)
			right_panel = _build_right_panel(
				left_pts, right_pts, left_coeffs, right_coeffs, bev_frame, lka_bev
			)

			ok = _write_ui(
				getattr(camera, "display_proc", None),
				left_panel,
				right_panel,
			)
			if not ok:
				break

	except Exception as e:
		print(f"FATAL ERROR: {e}", file=sys.stderr if stream_masks else sys.stdout)
	finally:
		if pipe_writer is not None:
			pipe_writer.cleanup()
		if camera:
			camera.terminate_camera()
			camera.terminate_display()
