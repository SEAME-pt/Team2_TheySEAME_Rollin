import numpy as np


class LaneTracker:
    """
    Lightweight per-lane temporal filter.

    Holds the most recent polynomial fit (coeffs of x = a*y^2 + b*y + c) for the
    left and right lanes and:
    - EMA-smooths a new fit toward the previous one to remove frame-to-frame
      jitter (de-flicker of the control output), and
    - holds the last known fit for up to `max_age` frames when a lane is missing,
      so a single-frame dropout does not snap the lane away or let a phantom
      take its place.

    The smoothed coeffs are also fed back to the sliding window as seeds so each
    frame's search starts from where the lane was last seen.
    """

    def __init__(self, alpha: float = 0.4, max_age: int = 8):
        self.alpha = alpha          # weight of the new fit (higher = more responsive)
        self.max_age = max_age      # frames a missing lane is held before being dropped
        self.left = None            # smoothed coeffs or None
        self.right = None
        self._left_age = 0
        self._right_age = 0

    def update(self, left_fit, right_fit):
        """Feed this frame's raw fits; returns (left_coeffs, right_coeffs) to use."""
        self.left, self._left_age = self._update_one(self.left, self._left_age, left_fit)
        self.right, self._right_age = self._update_one(self.right, self._right_age, right_fit)
        return self.left, self.right

    @property
    def seeds(self) -> tuple:
        """Current coeffs to seed the next frame's sliding-window search."""
        return (self.left, self.right)

    def _update_one(self, prev, age, new):
        if new is not None:
            new = np.asarray(new, dtype=np.float64)
            if prev is None:
                return new, 0
            smoothed = self.alpha * new + (1.0 - self.alpha) * prev
            return smoothed, 0
        # No measurement this frame: hold the previous fit until it goes stale.
        if prev is None:
            return None, age
        age += 1
        if age > self.max_age:
            return None, age
        return prev, age
