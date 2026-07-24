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

    def __init__(self, fov: int, roi: tuple, trapz: tuple, margin: "int | None" = None):
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

        hu, hb, wu, wu2, wb, wb2 = trapz

        src = np.float32([
            [wb, hb], # Left bottom
            [wb2, hb], # Right Bottom
            [wu, hu], # Left Up
            [wu2, hu], # Right Up
        ])

        srcPer = np.float32([
            [0, 0.625],
            [1, 0.625],
            [0.156, 0.468],
            [0.843, 0.468],
        ])
        # dst corners are shifted right by `margin` so the warped content is
        # centred in the enlarged canvas with `margin` px of head-room on both
        # sides for lateral curvature.
        m = float(self._margin)
        dst = np.float32([
            [96, 640],
            [544, 640],
            [96, 0],
            [544, 0],
        ])
        dstPer = np.float32([
            [0.15, 1],
            [0.85, 1],
            [0.15, 0],
            [0.85, 0],
        ])

        mult = np.float32([1280, 640])
        srcPer = srcPer * mult
        dstPer = dstPer * mult
        self._MP = cv2.getPerspectiveTransform(srcPer, dstPer)
        self._M = cv2.getPerspectiveTransform(src, dst)
        self._M_inv = cv2.getPerspectiveTransform(dst, src)
        self._MP_inv = cv2.getPerspectiveTransform(dstPer, srcPer)

    def apply(self, frame: np.ndarray) -> np.ndarray:
        """Crop to ROI and apply perspective warp. Returns BEV-space image."""
        # INTER_NEAREST keeps the mask binary (0/1) — INTER_LINEAR would create
        # fractional edge pixels that the sliding window's pixel test misses.
        return cv2.warpPerspective(
            frame, self._M, (640, 640), flags=cv2.INTER_NEAREST
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
