import numpy as np


class SlidingWindow:
    """
    Sliding window lane point extractor.
    Port of SlidingWindow.cpp — vectorized with NumPy instead of pixel-by-pixel loops.
    Expects a binary frame with WHITE_PIXEL == 1 (matching C++ Frame.hpp constant).
    """

    def get_lane_points(
        self,
        frame: np.ndarray,
        n_points: int,
    ) -> tuple[list[tuple[int, int]], list[tuple[int, int]]]:
        """
        Returns (left_pts, right_pts) — each a list of n_points (x, y) tuples
        in BEV-frame coordinates, ordered bottom-to-top.
        """
        histogram = frame.sum(axis=0).astype(np.int64)
        rect_w = frame.shape[1] // 8

        hist = histogram.copy()
        lane_x1 = self._get_lane_x(hist)
        lane_x2 = self._get_lane_x(hist)

        pts1 = self._sliding_window(frame, lane_x1, n_points, rect_w)
        pts2 = self._sliding_window(frame, lane_x2, n_points, rect_w)

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
        rect_w: int,
    ) -> list[tuple[int, int]]:
        h, w = frame.shape
        step_y = h // n_points
        x = start_x
        y = h - step_y
        points = []

        for _ in range(n_points):
            x1 = max(0, x - rect_w // 2)
            x2 = min(w, x + rect_w // 2)
            y1 = max(0, y)
            y2 = min(h, y + step_y)

            roi = frame[y1:y2, x1:x2]
            white_cols = np.where(roi == 1)[1]
            if len(white_cols) > 0:
                x = x1 + int(np.mean(white_cols))

            points.append((x, y + step_y // 2))
            y -= step_y

        return points
