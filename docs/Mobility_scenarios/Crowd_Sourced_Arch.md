# Crowdsourced Road Hazard Reporting
### Technical Scenario Report — SEAME / Piracer

---

## Table of Contents

- [Crowdsourced Road Hazard Reporting](#crowdsourced-road-hazard-reporting)
    - [Technical Scenario Report — SEAME / Piracer](#technical-scenario-report--seame--piracer)
  - [Table of Contents](#table-of-contents)
  - [1. Overview](#1-overview)
  - [2. Detection Logic — Stopped Car](#2-detection-logic--stopped-car)
    - [Logic Rationale](#logic-rationale)
  - [3. Detection Logic — Static Object](#3-detection-logic--static-object)
    - [Object Detection Parameters](#object-detection-parameters)
  - [4. Human Confirmation Flow (Qt Popup)](#4-human-confirmation-flow-qt-popup)
    - [Flow](#flow)
    - [Qt Implementation Notes](#qt-implementation-notes)
  - [5. System Architecture](#5-system-architecture)
    - [Component Overview](#component-overview)
  - [6. Technology Stack](#6-technology-stack)
    - [Edge (Per Piracer)](#edge-per-piracer)
    - [Central Infrastructure](#central-infrastructure)
    - [Network](#network)
  - [| Protocol | MQTT over TCP (port 1883), QoS 1 |](#-protocol--mqtt-over-tcp-port-1883-qos-1-)
  - [7. Kuksa — Fit Analysis](#7-kuksa--fit-analysis)
    - [Is Kuksa a good fit here?](#is-kuksa-a-good-fit-here)
    - [Architecture with Kuksa Integrated](#architecture-with-kuksa-integrated)
  - [8. Pros](#8-pros)
  - [9. Cons \& Risks](#9-cons--risks)
    - [Technical Risks](#technical-risks)
    - [Conceptual Limitations](#conceptual-limitations)
  - [10. Conclusion](#10-conclusion)

---

## 1. Overview

Crowdsourced Road Hazard Reporting is a cooperative system where multiple Piracer vehicles detect anomalies on the track — stopped cars and static objects — and report them to a shared central infrastructure. Other vehicles subscribe to those reports and adapt their behaviour accordingly.

The key principle is **collective intelligence**: instead of each car solving the problem alone, the fleet shares what it knows. This mirrors how Waze or Google Maps work at scale, but implemented in a miniaturised, controlled environment.

In this implementation, the hazard report is not sent automatically. A **human operator confirms** the detection via a Qt popup before the event is committed to the database — making the system hybrid: AI detects, human validates, fleet reacts.

---

## 2. Detection Logic — Stopped Car

The following decision tree describes how each Piracer determines whether to send a hazard report when it detects another car using the existing detection model.

> **Note:** The detection model (capable of detecting cars, TSR, etc.) is already available and does not need to be built from scratch.

```
Car detected in frame
│
├── Is OUR car currently moving?
│   │
│   ├── YES (our car is moving)
│   │   │
│   │   └── Is the detected car in the CENTER-TO-RIGHT zone of the frame?
│   │       │
│   │       ├── YES → How long has it been detected?
│   │       │         ├── SHORT time  → Trigger alert: STOPPED CAR
│   │       │         └── LONG time   → No alert (car is moving alongside us)
│   │       │
│   │       └── NO (left zone) → Ignore (is on the road)
│   │
│   └── NO (our car is stopped)
│       │
│       └── Is the detected car in the CENTER-TO-RIGHT zone?
│           │
│           ├── YES → How long has it been detected?
│           │         ├── SHORT time  →  No alert (other car is moving past us)
│           │         └── LONG time   →  Trigger alert: TWO STOPPED CARS
│           │
│           └── NO (left zone) → Trigger alert: OUR CAR IS STOPPED
```

### Logic Rationale

| Condition | Reasoning |
|---|---|
| Center-to-right zone | From the Piracer's camera perspective, objects in this zone are directly ahead or slightly to the right — on our path. Objects on the left are beside the road and less relevant for collision risk. |
| Short detection time + our car moving | A car appearing briefly in frame while we are moving means it is stationary relative to us — we drove up to it → stopped car |
| Long detection time + our car moving | Car stays in frame for a long time because it is moving alongside us at similar speed → not a hazard |
| Short detection time + our car stopped | Other car disappears quickly from our stopped perspective → it was moving past us → not a hazard |
| Long detection time + our car stopped | Both cars remain in the same relative position for extended time → both stopped |

---

## 3. Detection Logic — Static Object

When the detection model identifies a non-vehicle object on the track (cone, debris, box, etc.), a simpler logic applies — objects do not move, so we only need to determine whether the object is on our path and whether the detection is stable.

```
Object detected in frame (non-vehicle class)
│
├── Is the object in the CENTER-TO-left zone of the frame?
│   │
│   ├── NO → Ignore (object is off our path)
│   │
│   └── YES → Is the detection stable?
│             (detected for at least MIN_STABLE_FRAMES consecutive frames)
│             │
│             ├── NO → Ignore (likely noise or transient misdetection)
│             │
│             └── YES → Trigger alert: OBJECT ON TRACK
```

### Object Detection Parameters

| Parameter | Suggested Starting Value | Notes |
|---|---|---|
| `MIN_STABLE_FRAMES` | 5 frames (~0.5s at 10fps) | Filters out single-frame false positives |
| `FRAME_ZONE_CENTER_LEFT` | Same as car logic | Consistent boundary across both pipelines |
| `OBJECT_CLASSES` | cone, box, debris, barrier | Define which detection classes enter this pipeline |
| `MIN_CONFIDENCE` | 0.70 | Can be slightly lower than car threshold — objects are less ambiguous |

---

## 4. Human Confirmation Flow (Qt Popup)

Detection alone does not trigger a database write. A human operator must confirm the event via a Qt popup. This prevents false positives from polluting the shared hazard map.

### Flow

```
Detection logic fires → alert condition met (any hazard type)
         │
         ▼
Qt popup appears on operator screen
  ┌─────────────────────────────────────┐
  │  ⚠ Hazard Detected                  │
  │                                     │
  │  Stopped Car detected               │
  │  (or: Object on Track)              │
  │                                     │
  │  [  CONFIRM  ]    [  DISMISS  ]     │
  └─────────────────────────────────────┘
         │                   │
         ▼                   ▼
  Write to database      Discard event
  Publish MQTT alert     Log as false positive
```

### Qt Implementation Notes

- The popup is triggered by a signal emitted from the detection thread — use Qt's signal/slot mechanism across threads (`QMetaObject::invokeMethod` or `pyqtSignal`)
- Auto-dismiss the popup after a timeout (e.g. 10s) if the operator does not respond — avoid blocking the UI
- A keyboard shortcut (e.g. `Enter` to confirm, `Esc` to dismiss) speeds up operator response during a busy demo
- Both hazard types (car and object) use the same popup — only the description text changes

---

## 5. System Architecture

### Component Overview

```
┌─────────────────────────────────────────────────────────────┐
│                      PIRACER (Edge)                         │
│                                                             │
│  ┌─────────────┐    ┌──────────────┐    ┌───────────────┐   │
│  │  Camera +   │───▶│  Detection   │───▶│  Decision     │   │
│  │             │    │  Model       │    │  Logic Tree   │   │
│  │             │    │              │    │  (§2 and §3)  │   │
│  └─────────────┘    └──────────────┘    └──────┬────────┘   │
│                                                │            │
│                                        ┌──────▼────────┐    │
│                                        │  Qt Popup     │    │
│                                        │  Confirmation │    │
│                                        └──────┬────────┘    │
│                                               │             │
│                                        Confirmed            │
│                                               │             │
│                                       ┌──────▼────────┐     │
│                                       │  MQTT Publish │     │
│                                       │  paho-mqtt    │     │
│                                       └──────┬────────┘     │
└──────────────────────────────────────────────┼──────────────┘
                                               │
                              Wi-Fi / local network
                                               │
┌──────────────────────────────────────────────▼─────────────┐
│                   CENTRAL INFRASTRUCTURE (Pi)              │
│                                                            │
│  ┌──────────────┐   ┌─────────────┐   ┌────────────────┐   │
│  │  Mosquitto   │──▶│  FastAPI    │──▶│  SQLite / DB   │   │
│  │  MQTT Broker │   │  Backend    │   │  Hazard Log    │   │
│  └──────────────┘   └─────────────┘   └────────────────┘   │
└────────────────────────────────────────────────────────────┘
                          │
          Subscribes to MQTT alerts
                          │
┌─────────────────────────▼──────────────────────────────────┐
│               OTHER PIRACERS (Subscribers)                  │
│                                                             │
│  paho-mqtt subscriber → receive hazard alert               │
│  → reduce throttle / stop / reroute                        │
└────────────────────────────────────────────────────────────┘
```

---

## 6. Technology Stack

### Edge (Per Piracer)

| Component | Technology | Notes |
|---|---|---|
| Detection model | Existing model | Already trained — no extra work needed |
| Decision logic | Python | Runs as async thread alongside control loop |
| Qt popup | PyQt5 or PySide6 | Signal emitted from detection thread |
| MQTT client | paho-mqtt (Python) | Publish confirmed hazards, subscribe to others |

### Central Infrastructure

| Component | Technology | Notes |
|---|---|---|
| MQTT broker | Mosquitto | Lightweight, runs on a single Pi |
| Backend API | FastAPI (Python) | REST endpoints for event storage and query |
| Database | SQLite | Zero config, sufficient for this scale |

### Network

| Aspect | Approach |
|---|---|
| Transport | Wi-Fi (dedicated hotspot) |
| Protocol | MQTT over TCP (port 1883), QoS 1 |
---

## 7. Kuksa — Fit Analysis

**What Kuksa is:** Eclipse KUKSA is a vehicle data broker designed for Software Defined Vehicles (SDV). Its core component, the KUKSA Databroker, normalises raw sensor signals into standardised VSS (Vehicle Signal Specification) data points, accessible via gRPC or JSON APIs.

### Is Kuksa a good fit here?

| Question | Answer |
|---|---|
| Does this scenario need standardised VSS signals? | Not really — the hazard data (car detected, duration, zone) is application-level, not low-level vehicle signals |
| Does Kuksa help with inter-vehicle communication? | No — Kuksa is in-vehicle only, not vehicle-to-vehicle |
| Can Kuksa play a supporting role? | Yes — read `Vehicle.Speed` from the Kuksa Databroker to determine if our car is moving, instead of polling the motor controller directly |

This keeps each layer doing what it does best:
- **Kuksa** handles in-vehicle signal abstraction
- **MQTT** handles inter-vehicle messaging

### Architecture with Kuksa Integrated

```
Kuksa Databroker (in-vehicle)
    └── Vehicle.Speed
           │
           ▼
    Detection Logic (reads speed → determines our_car_moving)
           │
           ▼
    Qt Confirmation → MQTT Publish → Central Broker
```

---

## 8. Pros

- The detection model already exists — the hardest part is done
- MQTT is mature, well-documented, and native to the Raspberry Pi ecosystem
- The human-in-the-loop confirmation is a deliberate design choice — it prevents bad data from entering the shared map and makes the system more trustworthy
- Full stack is open-source, zero licensing cost
- the most easy and simple scneario to implement
---

## 9. Cons & Risks

### Technical Risks

| Risk | Impact | Mitigation |
|---|---|---|
| False positives from detection model | Medium — operator fatigue, bad data in DB | Confidence threshold + stability frame check |
| Detection logic edge cases (e.g. partially visible car or object) | Medium — wrong alert type triggered | Add `uncertain` state → always requires confirmation, never auto-sends |
| Wi-Fi instability | High — entire inter-vehicle system breaks | Dedicated hotspot; each Piracer still reacts locally if broker is unreachable |

### Conceptual Limitations

- No GPS — position estimates are approximate, based on encoder dead reckoning or visual landmarks on the track

---

## 10. Conclusion

This scenario is technically well-suited to the SEAME context because the hardest component — the detection model — is already built. The remaining work is integration, logic, and interface.

The system covers two distinct hazard types: stopped cars (with a movement-based decision tree) and static objects (with a frame stability check). Both feed into the same Qt confirmation flow and MQTT pipeline, keeping the implementation coherent.

The human confirmation step is not just a safety measure — it is a deliberate design decision that keeps bad data out of the shared hazard map and makes the overall system more reliable.

Kuksa fits as a data source for reading vehicle speed inside the detection logic, but should not replace MQTT for inter-vehicle communication — they solve different problems and work well together.