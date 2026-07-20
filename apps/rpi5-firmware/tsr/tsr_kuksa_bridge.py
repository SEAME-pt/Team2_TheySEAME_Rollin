#!/usr/bin/env python3
"""Read TSR detections and publish stable vehicle state to Kuksa.

Fixes for detector noise:
  - Reject garbage / full-frame / edge-ghost boxes
  - Sticky facing (no 0↔180 yaw snaps on class flips)
  - Per-update clamps on distance / lateral / yaw
  - Sliding-window average + publish EMA
  - Track association so one car does not multiply
"""

from __future__ import annotations

import math
import os
import struct
import sys
import time
from collections import deque
from dataclasses import dataclass, field

from kuksa_client.grpc import DataEntry, Datapoint, EntryUpdate, Field, VSSClient

FRAME_NMBR = 77000
MSG_FMT = ">IHHfIIIII"
MSG_SIZE = struct.calcsize(MSG_FMT)

# Oriented car classes only (exclude obstacle=11 — it caused edge ghosts).
VEHICLE_CLASSES = frozenset({6, 7, 8, 9})  # front, back, left, right
CLASS_FRONT = 6
CLASS_BACK = 7
FRAME_W = 640.0
FRAME_H = 640.0

CONF_MIN = 0.40
NMS_IOU = 0.40
NMS_CENTER_DIST = 0.16

MAX_TRACKS = 1  # one stable lead car (stops twin/flicker clones)
HISTORY_LEN = 8
MIN_HITS_TO_SHOW = 2
HOLD_S = 1.2
MATCH_LATERAL = 0.40
MATCH_DIST_RATIO = 0.65
MATCH_LATERAL_SOFT = 0.50
# If a new det is clearly more centered than the live track, replace it.
REPLACE_LATERAL_IMPROVEMENT = 0.25

# Geometry sanity (rejects the BAD_BOX / full-frame junk from the model).
MAX_BOX_H = 380.0
MAX_BOX_W = 450.0
MAX_BOX_AREA_FRAC = 0.48
MIN_BOX_H = 40.0
MIN_BOX_W = 40.0
EDGE_MARGIN_PX = 8.0
# Thin border-hugging strips (classic ghosts). Wide left/right cars are kept.
GHOST_STRIP_W = 120.0
# Height used for distance is capped so oversized boxes don't pin at 8 m forever.
DIST_HEIGHT_CAP = 200.0

# Motion limits per accepted update (stops teleports / yaw snaps).
MAX_LAT_STEP = 0.12
MAX_DIST_STEP_M = 4.0
MAX_YAW_STEP_DEG = 12.0
FACING_FLIP_HITS = 4  # consecutive opposite-class votes before flipping 0↔180

DIST_LUT = [
    (180.0, 8.0),
    (120.0, 15.0),
    (80.0, 25.0),
    (55.0, 35.0),
    (38.0, 50.0),
    (28.0, 65.0),
    (20.0, 80.0),
]

ASPECT_END_ON = 1.0
ASPECT_SIDE_ON = 2.2
MAX_SIDE_YAW = 35.0
LATERAL_REF_DIST_M = 20.0

PUBLISH_ALPHA = 0.22
PUBLISH_HOLD_S = 0.9
TIMEOUT_S = 1.5


def _clamp(v: float, lo: float, hi: float) -> float:
    return max(lo, min(hi, v))


def _normalize_yaw(yaw: float) -> float:
    while yaw > 180.0:
        yaw -= 360.0
    while yaw < -180.0:
        yaw += 360.0
    return yaw


def _yaw_delta(a: float, b: float) -> float:
    return _normalize_yaw(b - a)


def _ema(prev: float | None, new: float, alpha: float) -> float:
    if prev is None:
        return new
    return (1.0 - alpha) * prev + alpha * new


def _ema_yaw(prev: float | None, new: float, alpha: float) -> float:
    if prev is None:
        return new
    return _normalize_yaw(prev + alpha * _yaw_delta(prev, new))


def lookup_distance(bbox_h_px: float) -> float:
    # Cap height so oversized boxes don't pin everything at 8 m.
    h = _clamp(bbox_h_px, DIST_LUT[-1][0], DIST_LUT[0][0])
    if h >= DIST_LUT[0][0]:
        return DIST_LUT[0][1]
    if h <= DIST_LUT[-1][0]:
        return DIST_LUT[-1][1]
    for i in range(len(DIST_LUT) - 1):
        a, b = DIST_LUT[i], DIST_LUT[i + 1]
        if h <= a[0] and h >= b[0]:
            t = (h - a[0]) / (b[0] - a[0])
            return a[1] + t * (b[1] - a[1])
    return -1.0


def lateral_from_bbox(center_x: float) -> float:
    return _clamp((center_x - FRAME_W * 0.5) / (FRAME_W * 0.5), -1.0, 1.0)


def perspective_lateral(image_lateral: float, distance_m: float) -> float:
    # Far cars pull slightly toward center; never amplify beyond image lateral
    # (amplification was slamming near cars to ±1.0).
    scale = _clamp(LATERAL_REF_DIST_M / max(distance_m, 5.0), 0.35, 1.0)
    return _clamp(image_lateral * scale, -1.0, 1.0)


def facing_from_class(class_id: int) -> float:
    """0 = facing camera (front class), 180 = ahead / rear-ish."""
    return 0.0 if class_id == CLASS_FRONT else 180.0


def yaw_from_facing(facing: float, center_x: float, bbox_w: float, bbox_h: float) -> float:
    """Continuous yaw around sticky facing; aspect adds small side lean — no class snaps."""
    aspect = float(bbox_w) / max(float(bbox_h), 1.0)
    side_factor = _clamp((aspect - ASPECT_END_ON) / (ASPECT_SIDE_ON - ASPECT_END_ON), 0.0, 1.0)
    image_lat = lateral_from_bbox(center_x)
    return _normalize_yaw(facing + image_lat * side_factor * MAX_SIDE_YAW)


def box_is_valid(det: dict) -> bool:
    x, y = float(det["x"]), float(det["y"])
    w, h = float(det["w"]), float(det["h"])
    score = float(det["accuracy"])

    if w < MIN_BOX_W or h < MIN_BOX_H:
        return False
    if w > MAX_BOX_W or h > MAX_BOX_H:
        return False
    if (w * h) / (FRAME_W * FRAME_H) > MAX_BOX_AREA_FRAC:
        return False
    # Reject boxes that clearly explode outside the frame.
    if y < -25.0 or x < -25.0:
        return False
    if y + h > FRAME_H + 60.0 or x + w > FRAME_W + 60.0:
        return False

    # Classic ghost: skinny strip glued to the left/right border.
    if x <= EDGE_MARGIN_PX and w < GHOST_STRIP_W:
        return False
    if (x + w) >= (FRAME_W - EDGE_MARGIN_PX) and w < GHOST_STRIP_W:
        return False
    # Near-zero confidence junk on the rim.
    cx = x + w * 0.5
    if (cx < FRAME_W * 0.10 or cx > FRAME_W * 0.90) and score < 0.55:
        return False
    return True


def _iou(a: dict, b: dict) -> float:
    ax1, ay1 = float(a["x"]), float(a["y"])
    ax2, ay2 = ax1 + float(a["w"]), ay1 + float(a["h"])
    bx1, by1 = float(b["x"]), float(b["y"])
    bx2, by2 = bx1 + float(b["w"]), by1 + float(b["h"])
    ix1, iy1 = max(ax1, bx1), max(ay1, by1)
    ix2, iy2 = min(ax2, bx2), min(ay2, by2)
    iw, ih = max(0.0, ix2 - ix1), max(0.0, iy2 - iy1)
    inter = iw * ih
    if inter <= 0:
        return 0.0
    area_a = max(0.0, ax2 - ax1) * max(0.0, ay2 - ay1)
    area_b = max(0.0, bx2 - bx1) * max(0.0, by2 - by1)
    union = area_a + area_b - inter
    return inter / union if union > 0 else 0.0


def _center_lateral(det: dict) -> float:
    return lateral_from_bbox(float(det["x"]) + float(det["w"]) * 0.5)


def nms(dets: list[dict]) -> list[dict]:
    ordered = sorted(dets, key=lambda d: float(d["accuracy"]), reverse=True)
    kept: list[dict] = []
    for det in ordered:
        cl = _center_lateral(det)
        duplicate = False
        for k in kept:
            if _iou(det, k) >= NMS_IOU:
                duplicate = True
                break
            if abs(cl - _center_lateral(k)) < NMS_CENTER_DIST:
                duplicate = True
                break
        if not duplicate:
            kept.append(det)
    return kept


@dataclass
class Sample:
    distance_m: float
    yaw_deg: float
    lateral: float
    raw_lateral: float


@dataclass
class Track:
    history: deque = field(default_factory=lambda: deque(maxlen=HISTORY_LEN))
    last_seen: float = 0.0
    hits: int = 0
    raw_lateral: float = 0.0
    distance_m: float = 0.0
    yaw_deg: float = 180.0
    facing: float = 180.0  # sticky 0 or 180
    facing_votes: int = 0  # consecutive votes against current facing

    def _apply_motion_limits(self, sample: Sample) -> Sample:
        if self.hits == 0:
            return sample
        lat = _clamp(sample.raw_lateral, self.raw_lateral - MAX_LAT_STEP, self.raw_lateral + MAX_LAT_STEP)
        dist = _clamp(sample.distance_m, self.distance_m - MAX_DIST_STEP_M, self.distance_m + MAX_DIST_STEP_M)
        dy = _yaw_delta(self.yaw_deg, sample.yaw_deg)
        dy = _clamp(dy, -MAX_YAW_STEP_DEG, MAX_YAW_STEP_DEG)
        yaw = _normalize_yaw(self.yaw_deg + dy)
        return Sample(
            distance_m=dist,
            yaw_deg=yaw,
            lateral=perspective_lateral(lat, dist),
            raw_lateral=lat,
        )

    def update_facing(self, class_id: int) -> None:
        proposed = facing_from_class(class_id)
        if abs(_yaw_delta(self.facing, proposed)) < 90.0:
            self.facing_votes = 0
            return
        self.facing_votes += 1
        if self.facing_votes >= FACING_FLIP_HITS:
            self.facing = proposed
            self.facing_votes = 0

    def push(self, sample: Sample, now: float, class_id: int | None = None) -> None:
        if class_id is not None and self.hits > 0:
            self.update_facing(class_id)
        limited = self._apply_motion_limits(sample)
        self.history.append(limited)
        self.last_seen = now
        self.hits += 1
        self.raw_lateral = limited.raw_lateral
        self.distance_m = limited.distance_m
        self.yaw_deg = limited.yaw_deg

    @property
    def confirmed(self) -> bool:
        return self.hits >= MIN_HITS_TO_SHOW and len(self.history) >= 1

    def averaged(self) -> Sample | None:
        if not self.history:
            return None
        n = float(len(self.history))
        # Median-ish distance via sorted mid (more robust than mean to height spikes).
        dists = sorted(s.distance_m for s in self.history)
        dist = dists[len(dists) // 2]
        lat = sum(s.lateral for s in self.history) / n
        raw = sum(s.raw_lateral for s in self.history) / n
        sx = sum(math.cos(math.radians(s.yaw_deg)) for s in self.history) / n
        sy = sum(math.sin(math.radians(s.yaw_deg)) for s in self.history) / n
        yaw = math.degrees(math.atan2(sy, sx))
        return Sample(distance_m=dist, yaw_deg=_normalize_yaw(yaw), lateral=lat, raw_lateral=raw)


@dataclass
class PublishState:
    front: Sample | None = None
    left: Sample | None = None
    right: Sample | None = None
    front_seen: float = 0.0
    left_seen: float = 0.0
    right_seen: float = 0.0

    def blend(self, slot: str, sample: Sample | None, now: float) -> Sample | None:
        prev = getattr(self, slot)
        seen_attr = f"{slot}_seen"
        if sample is None:
            if prev is not None and (now - getattr(self, seen_attr)) <= PUBLISH_HOLD_S:
                return prev
            setattr(self, slot, None)
            return None
        setattr(self, seen_attr, now)
        if prev is None:
            setattr(self, slot, sample)
            return sample
        blended = Sample(
            distance_m=_ema(prev.distance_m, sample.distance_m, PUBLISH_ALPHA),
            yaw_deg=_ema_yaw(prev.yaw_deg, sample.yaw_deg, PUBLISH_ALPHA),
            lateral=_ema(prev.lateral, sample.lateral, PUBLISH_ALPHA),
            raw_lateral=_ema(prev.raw_lateral, sample.raw_lateral, PUBLISH_ALPHA),
        )
        setattr(self, slot, blended)
        return blended


@dataclass
class Tracker:
    tracks: list[Track] = field(default_factory=list)

    def update(self, observations: list[dict], now: float) -> list[Track]:
        unused = list(range(len(self.tracks)))
        assigned: list[tuple[int, dict]] = []

        for obs in sorted(observations, key=lambda o: (abs(o["raw_lateral"]), o["distance_m"])):
            best_i = None
            best_score = 1e9
            for i in unused:
                tr = self.tracks[i]
                d_lat = abs(obs["raw_lateral"] - tr.raw_lateral)
                d_ratio = abs(obs["distance_m"] - tr.distance_m) / max(obs["distance_m"], tr.distance_m, 1.0)
                if d_lat <= MATCH_LATERAL and d_ratio <= MATCH_DIST_RATIO:
                    score = d_lat * 2.0 + d_ratio
                elif d_lat <= MATCH_LATERAL_SOFT:
                    score = 1.0 + d_lat * 2.0 + d_ratio
                else:
                    continue
                if score < best_score:
                    best_score = score
                    best_i = i
            if best_i is not None:
                unused.remove(best_i)
                assigned.append((best_i, obs))

        alive: list[Track] = []
        matched_obs = {id(obs) for _, obs in assigned}

        for i, obs in assigned:
            tr = self.tracks[i]
            # Rebuild yaw from sticky facing + this box aspect (class flip can't snap 180°).
            yaw = yaw_from_facing(tr.facing, obs["center_x"], obs["w"], obs["h"])
            tr.push(
                Sample(
                    distance_m=obs["distance_m"],
                    yaw_deg=yaw,
                    lateral=obs["lateral"],
                    raw_lateral=obs["raw_lateral"],
                ),
                now,
                class_id=obs["class_id"],
            )
            alive.append(tr)

        for i in unused:
            tr = self.tracks[i]
            if now - tr.last_seen <= HOLD_S:
                alive.append(tr)

        for obs in observations:
            if id(obs) in matched_obs:
                continue
            nearest = None
            nearest_d = 1e9
            for t in alive:
                d = abs(obs["raw_lateral"] - t.raw_lateral)
                if d < nearest_d:
                    nearest_d = d
                    nearest = t
            if nearest is not None and nearest_d < MATCH_LATERAL_SOFT:
                yaw = yaw_from_facing(nearest.facing, obs["center_x"], obs["w"], obs["h"])
                nearest.push(
                    Sample(
                        distance_m=obs["distance_m"],
                        yaw_deg=yaw,
                        lateral=obs["lateral"],
                        raw_lateral=obs["raw_lateral"],
                    ),
                    now,
                    class_id=obs["class_id"],
                )
                continue
            # Replace a stuck off-center track with a clearly better (more centered) car.
            if alive and abs(obs["raw_lateral"]) + REPLACE_LATERAL_IMPROVEMENT < abs(alive[0].raw_lateral):
                facing = facing_from_class(obs["class_id"])
                tr = Track(
                    last_seen=now,
                    hits=0,
                    raw_lateral=obs["raw_lateral"],
                    distance_m=obs["distance_m"],
                    yaw_deg=obs["yaw_deg"],
                    facing=facing,
                )
                tr.push(
                    Sample(
                        distance_m=obs["distance_m"],
                        yaw_deg=yaw_from_facing(facing, obs["center_x"], obs["w"], obs["h"]),
                        lateral=obs["lateral"],
                        raw_lateral=obs["raw_lateral"],
                    ),
                    now,
                    class_id=obs["class_id"],
                )
                alive = [tr]
                continue
            if len(alive) >= MAX_TRACKS:
                continue
            facing = facing_from_class(obs["class_id"])
            tr = Track(
                last_seen=now,
                hits=0,
                raw_lateral=obs["raw_lateral"],
                distance_m=obs["distance_m"],
                yaw_deg=obs["yaw_deg"],
                facing=facing,
            )
            tr.push(
                Sample(
                    distance_m=obs["distance_m"],
                    yaw_deg=yaw_from_facing(facing, obs["center_x"], obs["w"], obs["h"]),
                    lateral=obs["lateral"],
                    raw_lateral=obs["raw_lateral"],
                ),
                now,
                class_id=obs["class_id"],
            )
            alive.append(tr)

        alive.sort(key=lambda t: (abs(t.raw_lateral), t.averaged().distance_m if t.averaged() else t.distance_m))
        self.tracks = alive[:MAX_TRACKS]
        return self.tracks

    def confirmed_averaged(self, now: float) -> list[Sample]:
        out: list[Sample] = []
        for tr in self.tracks:
            if now - tr.last_seen > HOLD_S:
                continue
            if not tr.confirmed:
                continue
            avg = tr.averaged()
            if avg is not None:
                out.append(avg)
        return out[:MAX_TRACKS]


def read_frame(pipe) -> list[dict]:
    raw = pipe.read(MSG_SIZE)
    if len(raw) != MSG_SIZE:
        return []

    frame_nbr, num_detections, class_id, accuracy, x, y, w, h, _marker_id = struct.unpack(MSG_FMT, raw)
    if frame_nbr != FRAME_NMBR:
        print(f"Sync problem (got {frame_nbr}, expected {FRAME_NMBR})", flush=True)
        return []
    if num_detections == 0:
        return []

    detections = [{"class_id": class_id, "accuracy": accuracy, "x": x, "y": y, "w": w, "h": h}]
    for _ in range(num_detections - 1):
        raw = pipe.read(MSG_SIZE)
        if len(raw) != MSG_SIZE:
            break
        _, _, class_id, accuracy, x, y, w, h, _marker_id = struct.unpack(MSG_FMT, raw)
        detections.append({"class_id": class_id, "accuracy": accuracy, "x": x, "y": y, "w": w, "h": h})
    return detections


def detections_to_observations(detections: list[dict]) -> list[dict]:
    vehicles = [
        d
        for d in detections
        if d["class_id"] in VEHICLE_CLASSES
        and float(d["accuracy"]) >= CONF_MIN
        and box_is_valid(d)
    ]
    vehicles = nms(vehicles)

    observations: list[dict] = []
    for det in vehicles:
        w = float(det["w"])
        h = float(det["h"])
        # Cap height for distance; for side classes also consider width (less inflated).
        h_for_dist = min(h, DIST_HEIGHT_CAP)
        if int(det["class_id"]) in (8, 9):  # left/right
            h_for_dist = min(h_for_dist, max(w * 0.85, MIN_BOX_H))
        dist_m = lookup_distance(h_for_dist)
        if dist_m < 0:
            continue
        center_x = float(det["x"]) + w * 0.5
        raw_lat = lateral_from_bbox(center_x)
        facing = facing_from_class(int(det["class_id"]))
        observations.append(
            {
                "class_id": int(det["class_id"]),
                "center_x": center_x,
                "w": w,
                "h": h,
                "distance_m": dist_m,
                "yaw_deg": yaw_from_facing(facing, center_x, w, h),
                "lateral": perspective_lateral(raw_lat, dist_m),
                "raw_lateral": raw_lat,
            }
        )
    # Prefer centered cars (lead) over edge ghosts when ranking.
    observations.sort(key=lambda o: (abs(o["raw_lateral"]), o["distance_m"]))
    return observations[:3]


def samples_to_slots(samples: list[Sample], pub: PublishState, now: float) -> tuple:
    front = pub.blend("front", samples[0] if len(samples) > 0 else None, now)
    left = pub.blend("left", samples[1] if len(samples) > 1 else None, now)
    right = pub.blend("right", None, now)

    def pack(s: Sample | None):
        if s is None:
            return (-1.0, 180.0, 0.0)
        return (s.distance_m, s.yaw_deg, s.lateral)

    return (*pack(front), *pack(left), *pack(right))


def _kuksa_set(client: VSSClient, path: str, value):
    client.set(
        updates=[EntryUpdate(DataEntry(path, value=Datapoint(value)), (Field.VALUE,))],
        try_v2=False,
    )


def publish_slots(client, front_m, front_yaw, front_lat, left_m, left_yaw, left_lat, right_m, right_yaw, right_lat):
    _kuksa_set(client, "Vehicle.ADAS.AdaptiveCruiseControl.LeadVehicleDistance", front_m if front_m >= 0 else 0.0)
    _kuksa_set(
        client,
        "Vehicle.ADAS.AdaptiveCruiseControl.LeadVehicleOrientation",
        float(front_yaw) if front_m >= 0 else 180.0,
    )
    _kuksa_set(
        client,
        "Vehicle.ADAS.AdaptiveCruiseControl.LeadVehicleLateralOffset",
        float(front_lat) if front_m >= 0 else 0.0,
    )
    _kuksa_set(client, "Vehicle.ADAS.BlindSpotDetection.LeftOccupied", left_m >= 0)
    _kuksa_set(client, "Vehicle.ADAS.BlindSpotDetection.RightOccupied", right_m >= 0)
    _kuksa_set(client, "Vehicle.ADAS.BlindSpotDetection.LeftDistance", left_m if left_m >= 0 else 0.0)
    _kuksa_set(client, "Vehicle.ADAS.BlindSpotDetection.RightDistance", right_m if right_m >= 0 else 0.0)
    _kuksa_set(
        client,
        "Vehicle.ADAS.BlindSpotDetection.LeftVehicleOrientation",
        float(left_yaw) if left_m >= 0 else 180.0,
    )
    _kuksa_set(
        client,
        "Vehicle.ADAS.BlindSpotDetection.RightVehicleOrientation",
        float(right_yaw) if right_m >= 0 else 180.0,
    )
    _kuksa_set(
        client,
        "Vehicle.ADAS.BlindSpotDetection.LeftVehicleLateralOffset",
        float(left_lat) if left_m >= 0 else 0.0,
    )
    _kuksa_set(
        client,
        "Vehicle.ADAS.BlindSpotDetection.RightVehicleLateralOffset",
        float(right_lat) if right_m >= 0 else 0.0,
    )


_EMPTY = (-1.0, 180.0, 0.0, -1.0, 180.0, 0.0, -1.0, 180.0, 0.0)


def main():
    pipe_path = sys.argv[1] if len(sys.argv) > 1 else "NamedPipeTsr"
    client = VSSClient(os.environ.get("KUKSA_HOST", "127.0.0.1"), int(os.environ.get("KUKSA_PORT", "55555")))
    tracker = Tracker()
    pub = PublishState()
    with open(pipe_path, "rb", buffering=0) as pipe:
        client.connect()
        publish_slots(client, *_EMPTY)
        last_detection = time.monotonic()
        try:
            while True:
                detections = read_frame(pipe)
                now = time.monotonic()
                if not detections:
                    if now - last_detection > TIMEOUT_S:
                        tracker.tracks.clear()
                        pub = PublishState()
                        publish_slots(client, *_EMPTY)
                        last_detection = now
                    else:
                        samples = tracker.confirmed_averaged(now)
                        publish_slots(client, *samples_to_slots(samples, pub, now))
                    continue

                last_detection = now
                observations = detections_to_observations(detections)
                tracker.update(observations, now)
                samples = tracker.confirmed_averaged(now)
                publish_slots(client, *samples_to_slots(samples, pub, now))
        except KeyboardInterrupt:
            publish_slots(client, *_EMPTY)
            client.disconnect()


if __name__ == "__main__":
    main()
