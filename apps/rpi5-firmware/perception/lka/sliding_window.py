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

	# ── Lane-seed segmentation ────────────────────────────────────────────────
	# Seeds come from *contiguous* histogram segments, not a blind argmax-twice,
	# so one wide/curved lane stays a single lane instead of being split into two
	# phantom seeds that both track the same mask blob.
	# A column joins a segment when its mass clears this fraction of the strongest
	# column — separates real lane mass from speckle.
	_COL_ACTIVE_FRAC = 0.15
	# Column gaps narrower than this fraction of the width are bridged, so a
	# dashed lane or a small break stays one segment.
	_SEG_MERGE_FRAC = 0.05
	# A second lane is only seeded when its segment carries at least this fraction
	# of the dominant segment's mass AND sits at least this far from it. Together
	# they stop a single broad lane from being read as two.
	_SECOND_SEG_MASS_FRAC = 0.20
	_SECOND_SEG_GAP_FRAC = 0.15

	# ── Lane-validity thresholds ──────────────────────────────────────────────
	# A fitted lane is discarded when its shape is physically implausible for a
	# road lane viewed in BEV. Thresholds are expressed relative to the frame
	# size so they stay valid if the BEV canvas is resized.
	#
	# Max departure of the fitted parabola from the straight chord joining its
	# endpoints (mid-ordinate = |a| * height^2 / 8), as a fraction of width. A
	# real lane bends gently; a fit that bows out further than this is mask
	# speckle curled into a hook rather than a lane.
	_MAX_BEND_FRAC = 0.75
	# Max horizontal back-tracking once the parabola passes its vertex, as a
	# fraction of width. A road lane does not reverse direction within the few
	# metres of near field the camera sees, so a curve whose vertex sits inside
	# the visible rows and then doubles back this far is a bad fit.
	_MAX_REVERSAL_FRAC = 0.20
	# Rows sampled when checking two lanes for intersection.
	_N_VALIDATE_SAMPLES = 20

	def get_lane_points(
		self,
		frame: np.ndarray,
		n_points: int,
	) -> tuple[list[tuple[int, int]], list[tuple[int, int]]]:
		"""
		Returns (left_pts, right_pts) — each a list of up to n_points (x, y) tuples
		in BEV-frame coordinates, ordered bottom-to-top. When only one lane is
		present its points come back in the slot matching its side of the frame
		(left if its seed is left of centre, else right) and the other slot is
		empty, so the caller can synthesize the missing lane.

		Every candidate goes through a validation phase: lanes with an impossible
		shape (excessive curvature or a direction reversal) are dropped, and a
		surviving pair that intersects keeps only the better-supported lane.
		"""
		h, w = frame.shape
		histogram = frame.sum(axis=0).astype(np.int64)
		rect_w = w // 12

		seeds = self._find_lane_seeds(histogram, w)

		# Run the window walk from each seed. Keep [seed_x, points, coeffs,
		# n_detections] only for lanes that survive the per-lane shape check.
		candidates = []
		for sx in seeds:
			pts, coeffs, n_det = self._sliding_window(frame, sx, n_points, rect_w)
			if pts and self._shape_valid(coeffs, h, w):
				candidates.append([sx, pts, coeffs, n_det])

		if not candidates:
			return [], []

		# Cross-lane validation: two surviving lanes must not cross within the
		# visible rows. If they do, keep the lane with more real detections and
		# drop the other so a forked/crossed fit can't poison the pair — the
		# caller then synthesizes the dropped side at the learned lane width.
		if len(candidates) == 2:
			candidates.sort(key=lambda r: r[0])
			left, right = candidates
			if not self._separation_valid(left[2], right[2], h, w):
				candidates = [left] if left[3] >= right[3] else [right]

		if len(candidates) == 1:
			sx, pts = candidates[0][0], candidates[0][1]
			# Place the lone lane on its geometric side so the caller knows which
			# side is missing and can offset the virtual lane the right way.
			return (pts, []) if sx <= w // 2 else ([], pts)

		candidates.sort(key=lambda r: r[0])
		return candidates[0][1], candidates[1][1]

	def _find_lane_seeds(self, histogram: np.ndarray, w: int) -> list[int]:
		"""
		Seed x-positions from contiguous histogram segments. Returns 0, 1 or 2
		seeds: a single broad lane gives one segment (one seed); two genuinely
		separated lanes give two.
		"""
		peak = int(histogram.max())
		if peak <= 0:
			return []

		active = histogram > max(1, int(peak * self._COL_ACTIVE_FRAC))
		idx = np.where(active)[0]
		if idx.size == 0:
			return []

		# Split active columns into segments, bridging small gaps (dashes/noise).
		merge_gap = max(1, int(w * self._SEG_MERGE_FRAC))
		splits = np.where(np.diff(idx) > merge_gap)[0]
		groups = np.split(idx, splits + 1)

		# (mass, seed_x) per segment; seed is the strongest column in the segment.
		segs = []
		for g in groups:
			mass = int(histogram[g].sum())
			seed = int(g[int(np.argmax(histogram[g]))])
			segs.append((mass, seed))
		segs.sort(reverse=True)  # strongest segment first

		seeds = [segs[0][1]]
		if len(segs) > 1:
			second_mass, second_seed = segs[1]
			far_enough = abs(second_seed - segs[0][1]) >= self._SECOND_SEG_GAP_FRAC * w
			strong_enough = second_mass >= segs[0][0] * self._SECOND_SEG_MASS_FRAC
			if far_enough and strong_enough:
				seeds.append(second_seed)
		return seeds

	def _sliding_window(
		self,
		frame: np.ndarray,
		start_x: int,
		n_points: int,
		rect_w: int,
	) -> tuple[list[tuple[int, int]], "np.ndarray | None", int]:
		"""
		Walk windows up the image from ``start_x`` and fit a curve to the real
		detections. Returns ``(points, coeffs, n_detections)`` where ``coeffs``
		is the fitted polynomial (or ``None`` when the lane is dropped) and
		``n_detections`` is how many windows actually saw pixels — used to rank
		lanes during cross-lane validation.
		"""
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
