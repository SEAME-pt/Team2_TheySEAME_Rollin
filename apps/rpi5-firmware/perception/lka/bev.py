import cv2
import numpy as np


class Bev:
    """
    Bird's-Eye-View perspective transform.
    Port of Bev.cpp — same src/dst corner layout and math, but the warp is
    rendered into a canvas that is *wider* than the ROI so curved lanes are not
    clipped at the output bounds (the original Bev.cpp warped into a canvas the
    exact size of the ROI, which discarded the upper, laterally-displaced part
    of curved lanes).
    """

    def __init__(self, fov: int, roi: tuple, margin: "int | None" = None):
        """
        fov: horizontal FOV compression for BEV (same as C++ fov parameter)
        roi: (sx, sy, w, h) — region of interest cropped before warping
        margin: extra pixels added to *each* side of the output canvas. Curved
                lanes that warp outside the ROI width are retained instead of
                clipped. Defaults to half the ROI width.
        """
        sx, sy, w, h = roi
        self._roi = roi
        frame_h = float(h)
        frame_w = float(w)

        if margin is None:
            margin = int(w * 0.5)
        self._margin = int(margin)

        out_w = w + 2 * self._margin
        out_h = h
        self._out_size = (out_w, out_h)

        src = np.float32([
            [0,       frame_h],
            [frame_w, frame_h],
            [0,       0      ],
            [frame_w, 0      ],
        ])
        # dst corners are shifted right by `margin` so the warped content is
        # centred in the enlarged canvas with `margin` px of head-room on both
        # sides for lateral curvature.
        m = float(self._margin)
        dst = np.float32([
            [m + frame_w / 2 - fov / 2, frame_h],
            [m + frame_w / 2 + fov / 2, frame_h],
            [m + 0,                     0      ],
            [m + frame_w,               0      ],
        ])

        self._M = cv2.getPerspectiveTransform(src, dst)
        self._M_inv = cv2.getPerspectiveTransform(dst, src)

    def apply(self, frame: np.ndarray) -> np.ndarray:
        """Crop to ROI and apply perspective warp. Returns BEV-space image."""
        sx, sy, w, h = self._roi
        cropped = frame[sy:sy + h, sx:sx + w]
        # INTER_NEAREST keeps the mask binary (0/1) — INTER_LINEAR would create
        # fractional edge pixels that the sliding window's pixel test misses.
        return cv2.warpPerspective(
            cropped, self._M, self._out_size, flags=cv2.INTER_NEAREST
        )

    @property
    def reverse_matrix(self) -> np.ndarray:
        return self._M_inv

    @property
    def roi(self) -> tuple:
        return self._roi

    @property
    def out_size(self) -> tuple:
        """(width, height) of the warped BEV canvas — wider than the ROI."""
        return self._out_size

    @property
    def margin(self) -> int:
        return self._margin
