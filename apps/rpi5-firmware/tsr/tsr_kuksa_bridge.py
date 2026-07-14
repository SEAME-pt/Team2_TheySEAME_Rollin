#!/usr/bin/env python3
"""Read TSR detections from NamedPipeTsr and publish vehicle state to Kuksa."""

import os
import struct
import sys
import time

from kuksa_client.grpc import DataEntry, Datapoint, EntryUpdate, Field, VSSClient

FRAME_NMBR = 77000
MSG_FMT = ">IHHfIIII"
MSG_SIZE = struct.calcsize(MSG_FMT)

CAR_CLASS = 6
OBSTACLE_CLASS = 8
FRAME_W = 640.0

DIST_LUT = [
    (120.0, 20.0),
    (80.0, 30.0),
    (55.0, 42.0),
    (38.0, 60.0),
    (28.0, 80.0),
]

TIMEOUT_S = 0.5


def lookup_distance(bbox_px: float) -> float:
    if bbox_px >= DIST_LUT[0][0]:
        return DIST_LUT[0][1]
    if bbox_px <= DIST_LUT[-1][0]:
        return DIST_LUT[-1][1]
    for i in range(len(DIST_LUT) - 1):
        a, b = DIST_LUT[i], DIST_LUT[i + 1]
        if bbox_px <= a[0] and bbox_px >= b[0]:
            t = (bbox_px - a[0]) / (b[0] - a[0])
            return a[1] + t * (b[1] - a[1])
    return -1.0


def classify_lane(center_x: float) -> str:
    if center_x < FRAME_W * 0.35:
        return "left"
    if center_x > FRAME_W * 0.65:
        return "right"
    return "center"


def is_vehicle(class_id: int) -> bool:
    return class_id in (CAR_CLASS, OBSTACLE_CLASS)


def read_frame(pipe) -> list[dict]:
    raw = pipe.read(MSG_SIZE)
    if len(raw) != MSG_SIZE:
        return []

    frame_nbr, num_detections, class_id, accuracy, x, y, w, h = struct.unpack(
        MSG_FMT, raw
    )
    if frame_nbr != FRAME_NMBR:
        print(f"Sync problem (got {frame_nbr}, expected {FRAME_NMBR})", flush=True)
        return []

    if num_detections == 0:
        return []

    detections = [
        {
            "class_id": class_id,
            "accuracy": accuracy,
            "x": x,
            "y": y,
            "w": w,
            "h": h,
        }
    ]

    for _ in range(num_detections - 1):
        raw = pipe.read(MSG_SIZE)
        if len(raw) != MSG_SIZE:
            break
        _, _, class_id, accuracy, x, y, w, h = struct.unpack(MSG_FMT, raw)
        detections.append(
            {
                "class_id": class_id,
                "accuracy": accuracy,
                "x": x,
                "y": y,
                "w": w,
                "h": h,
            }
        )

    return detections


def process_vehicles(detections: list[dict]) -> tuple[float, float, float]:
    front_m = -1.0
    left_m = -1.0
    right_m = -1.0

    for det in detections:
        if not is_vehicle(det["class_id"]):
            continue
        if det["w"] < 8 or det["h"] < 8:
            continue

        dist_m = lookup_distance(float(max(det["w"], det["h"])))
        if dist_m < 0:
            continue

        center_x = float(det["x"]) + float(det["w"]) * 0.5
        lane = classify_lane(center_x)

        if lane == "center":
            if front_m < 0 or dist_m < front_m:
                front_m = dist_m
        elif lane == "left":
            if left_m < 0 or dist_m < left_m:
                left_m = dist_m
        elif right_m < 0 or dist_m < right_m:
            right_m = dist_m

    return front_m, left_m, right_m


def _kuksa_set(client: VSSClient, path: str, value):
    """Publish using Kuksa v1 API (v2 client call is broken on this Pi image)."""
    client.set(
        updates=[EntryUpdate(DataEntry(path, value=Datapoint(value)), (Field.VALUE,))],
        try_v2=False,
    )


def publish_vehicles(client: VSSClient, front_m: float, left_m: float, right_m: float):
    front = front_m if front_m >= 0 else 0.0
    left_occ = left_m >= 0
    right_occ = right_m >= 0
    left_dist = left_m if left_m >= 0 else 0.0
    right_dist = right_m if right_m >= 0 else 0.0

    _kuksa_set(client, "Vehicle.ADAS.AdaptiveCruiseControl.LeadVehicleDistance", front)
    _kuksa_set(client, "Vehicle.ADAS.BlindSpotDetection.LeftOccupied", left_occ)
    _kuksa_set(client, "Vehicle.ADAS.BlindSpotDetection.RightOccupied", right_occ)
    _kuksa_set(client, "Vehicle.ADAS.BlindSpotDetection.LeftDistance", left_dist)
    _kuksa_set(client, "Vehicle.ADAS.BlindSpotDetection.RightDistance", right_dist)


def clear_vehicles(client: VSSClient):
    publish_vehicles(client, -1.0, -1.0, -1.0)


def main():
    pipe_path = sys.argv[1] if len(sys.argv) > 1 else "NamedPipeTsr"
    kuksa_host = os.environ.get("KUKSA_HOST", "127.0.0.1")
    kuksa_port = int(os.environ.get("KUKSA_PORT", "55555"))

    print(f"Opening {pipe_path} ...", flush=True)
    with open(pipe_path, "rb", buffering=0) as pipe:
        client = VSSClient(kuksa_host, kuksa_port)
        client.connect()
        print(f"Connected to Kuksa at {kuksa_host}:{kuksa_port}", flush=True)
        clear_vehicles(client)

        last_detection = time.monotonic()
        try:
            while True:
                detections = read_frame(pipe)
                if not detections:
                    if time.monotonic() - last_detection > TIMEOUT_S:
                        clear_vehicles(client)
                        last_detection = time.monotonic()
                    continue

                last_detection = time.monotonic()
                front_m, left_m, right_m = process_vehicles(detections)
                publish_vehicles(client, front_m, left_m, right_m)

                if front_m >= 0 or left_m >= 0 or right_m >= 0:
                    print(
                        f"vehicles front={front_m:.1f}m left={left_m:.1f}m right={right_m:.1f}m",
                        flush=True,
                    )
        except KeyboardInterrupt:
            pass
        finally:
            clear_vehicles(client)
            client.disconnect()


if __name__ == "__main__":
    main()
