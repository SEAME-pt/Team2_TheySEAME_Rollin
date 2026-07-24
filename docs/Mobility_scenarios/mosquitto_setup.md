# MQTT Setup Guide — Road Hazard Reporting

This guide covers two things: setting up the Mosquitto **broker** on the central Pi, and what the **publisher** and **subscriber** code on each Piracer needs to do.

---

## 1. Broker Setup (Central Pi)

The broker is not something you write — it's the Mosquitto daemon, installed and run as a service. Every Piracer connects to it; it never talks to a Piracer directly back, it only routes messages.

### Install

```bash
sudo apt update
sudo apt install mosquitto mosquitto-clients
```

`mosquitto-clients` gives you `mosquitto_pub` / `mosquitto_sub` command-line tools, useful for testing without writing any code.

### Configure

Create a config file, e.g. `/etc/mosquitto/conf.d/hazard.conf`:

```
listener 1883
allow_anonymous true
persistence true
persistence_location /var/lib/mosquitto/
log_dest file /var/log/mosquitto/mosquitto.log
```

`allow_anonymous true` is fine for a closed demo network. If this ever leaves a controlled Wi-Fi hotspot, switch to a password file (`mosquitto_passwd`) instead — anonymous + open Wi-Fi means anyone on the network can publish fake hazards.

### Run it

As a systemd service (recommended, survives reboot):

```bash
sudo systemctl enable mosquitto
sudo systemctl restart mosquitto
sudo systemctl status mosquitto
```

Or manually in the foreground for debugging:

```bash
mosquitto -c /etc/mosquitto/conf.d/hazard.conf -v
```

### Verify it works

In one terminal:
```bash
mosquitto_sub -h localhost -t "hazards/#" -v
```

In another:
```bash
mosquitto_pub -h localhost -t "hazards/test" -m "hello"
```

If the subscriber terminal prints `hazards/test hello`, the broker is working. Do this test **before** writing any Piracer code — it isolates network/broker problems from application bugs.

### Network notes

- All Piracers and the central Pi must be on the same Wi-Fi network (dedicated hotspot, per the design doc).
- Piracers connect using the central Pi's IP address (not `localhost` — that only works for the FastAPI backend running on the same machine as the broker).
- Find the Pi's IP with `hostname -I` or `ip addr`.
- Port 1883 is plain MQTT (no TLS). Fine for an isolated local network; not fine for anything internet-facing.

---

## 2. Topic Structure

Before writing pub/sub code, agree on topics. A reasonable scheme:

```
hazards/stopped_car
hazards/object_on_track
```

Or, if you want zone/location built into the topic for filtering:

```
hazards/<zone_id>/stopped_car
hazards/<zone_id>/object_on_track
```

Either way, put the actual data in a **JSON payload**, not the topic, so it's easy to extend later:

```json
{
  "type": "stopped_car",
  "zone": "B3",
  "reported_by": "piracer-02",
  "timestamp": 1750350000,
  "confidence": 0.91
}
```

Subscribers can use the wildcard `hazards/#` to catch everything regardless of type or zone.

---

## 3. Publisher — What Each Piracer Needs to Do

The publisher side runs after the detection logic and Qt confirmation popup (per the decision trees in the design doc). It does **not** publish automatically on detection — only after human confirmation.

### Responsibilities

1. Connect to the broker on startup (using the Pi's IP, not localhost).
2. Wait for the detection + decision logic to fire an alert.
3. Show the Qt confirmation popup.
4. **Only if confirmed:** build the JSON payload and publish it.
5. If dismissed or timed out: discard, don't publish.

### Minimal C example (libmosquitto)

```c
#include <mosquitto.h>
#include <stdio.h>
#include <string.h>

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
    printf("Publisher connected (rc=%d)\n", rc);
}

void on_publish(struct mosquitto *mosq, void *obj, int mid) {
    printf("Hazard published, mid=%d\n", mid);
}

void publish_hazard(struct mosquitto *mosq, const char *json_payload) {
    int rc = mosquitto_publish(mosq, NULL, "hazards/stopped_car",
                                (int)strlen(json_payload), json_payload,
                                1 /* QoS 1 */, false);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Publish failed: %s\n", mosquitto_strerror(rc));
    }
}

int main(void) {
    mosquitto_lib_init();
    struct mosquitto *mosq = mosquitto_new("piracer-02", true, NULL);

    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_publish_callback_set(mosq, on_publish);

    mosquitto_connect(mosq, "192.168.1.10" /* central Pi IP */, 1883, 60);
    mosquitto_loop_start(mosq);

    /* called only after Qt popup confirmation */
    publish_hazard(mosq, "{\"type\":\"stopped_car\",\"zone\":\"B3\",\"reported_by\":\"piracer-02\"}");

    mosquitto_loop_stop(mosq, true);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
```

### Things to get right

- **Unique client ID per Piracer** (`"piracer-02"` above) — two clients with the same ID on the same broker will kick each other off.
- **QoS 1** ensures the message is delivered at least once — appropriate for hazard alerts where a dropped message matters. QoS 0 is fire-and-forget; QoS 2 is overkill here.
- **Connect once at startup**, not per-publish. Reconnecting for every hazard adds latency and connection overhead.
- **Don't block the detection loop** — publishing happens on the Qt main thread or wherever the confirm button handler runs, while `mosquitto_loop_start` runs the network I/O in its own background thread.

---

## 4. Subscriber — What Each Piracer (and the Backend) Needs to Do

Every Piracer subscribes too — to hear about hazards reported by *other* cars. The FastAPI backend subscribes as well, separately, to persist confirmed hazards into SQLite.

### Responsibilities (Piracer side)

1. Connect to the broker.
2. Subscribe to `hazards/#` on connect.
3. In the message callback, parse the JSON payload.
4. React: reduce throttle, stop, or reroute depending on hazard type and zone.
5. Ignore hazards reported by itself, if `reported_by` matches its own ID (avoid reacting to your own confirmed report).

### Minimal C example (libmosquitto)

```c
#include <mosquitto.h>
#include <stdio.h>
#include <string.h>

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
    if (rc == 0) {
        mosquitto_subscribe(mosq, NULL, "hazards/#", 1 /* QoS 1 */);
    }
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    printf("Hazard on %s: %.*s\n", msg->topic, msg->payloadlen, (char *)msg->payload);
    /* parse JSON payload here (e.g. with cJSON), then act:
       - check "reported_by" != our own ID
       - check "zone" relevance
       - reduce throttle / trigger reroute logic */
}

int main(void) {
    mosquitto_lib_init();
    struct mosquitto *mosq = mosquitto_new("piracer-02-sub", true, NULL);

    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);

    mosquitto_connect(mosq, "192.168.1.10", 1883, 60);
    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
```

In practice, a single Piracer process is **both publisher and subscriber on the same `mosquitto` connection** — you don't need two separate client objects. Set both callbacks, subscribe to `hazards/#` on connect, and call `mosquitto_publish` from wherever the confirm button handler lives. One `mosquitto_loop_start` background thread handles all the network traffic for both directions.

### Backend subscriber (conceptual)

The FastAPI backend's job is simpler: subscribe to `hazards/#`, and on every message, write a row to SQLite. It doesn't need to react to hazards (it's not driving), just persist them. This can be a small standalone Python script using `paho-mqtt`, run alongside FastAPI, or integrated into FastAPI's startup event.

---

## 5. Quick Checklist

- [ ] Mosquitto installed and running as a service on the central Pi
- [ ] Verified with `mosquitto_pub` / `mosquitto_sub` before writing any code
- [ ] Topic scheme agreed (`hazards/<type>` or `hazards/<zone>/<type>`)
- [ ] Each Piracer uses a unique client ID
- [ ] Publish only fires after Qt popup confirmation, never on raw detection
- [ ] QoS 1 used for hazard messages
- [ ] Subscribers ignore their own published hazards (`reported_by` check)
- [ ] Backend subscriber writes confirmed hazards to SQLite
- [ ] Piracers connect to the Pi's actual IP, not `localhost`