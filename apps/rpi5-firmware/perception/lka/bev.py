import cv2
import numpy as np


class Bev:
    """
    Bird's-Eye-View perspective transform.
    Port of Bev.cpp — identical src/dst corner layout and math.
    """

    def __init__(self, fov: int, roi: tuple):
        """
        fov: horizontal FOV compression for BEV (same as C++ fov parameter)
        roi: (sx, sy, w, h) — region of interest cropped before warping
        """
        sx, sy, w, h = roi
        self._roi = roi
        frame_h = float(h)
        frame_w = float(w)

        src = np.float32([
            [0,       frame_h],
            [frame_w, frame_h],
            [0,       0      ],
            [frame_w, 0      ],
        ])
        dst = np.float32([
            [frame_w / 2 - fov / 2, frame_h],
            [frame_w / 2 + fov / 2, frame_h],
            [0,                     0      ],
            [frame_w,               0      ],
        ])

        self._M = cv2.getPerspectiveTransform(src, dst)
        self._M_inv = cv2.getPerspectiveTransform(dst, src)

    def apply(self, frame: np.ndarray) -> np.ndarray:
        """Crop to ROI and apply perspective warp. Returns BEV-space image."""
        sx, sy, w, h = self._roi
        cropped = frame[sy:sy + h, sx:sx + w]
        return cv2.warpPerspective(cropped, self._M, (w, h))

    @property
    def reverse_matrix(self) -> np.ndarray:
        return self._M_inv

    @property
    def roi(self) -> tuple:
        return self._roi
