import json
import time
import paho.mqtt.client as mqtt
import psycopg2

MQTT_HOST = "localhost"        # mosquitto container, published as 1883:1883 on this host
MQTT_PORT = 1883
MQTT_TOPIC = "#"

PG_CONFIG = dict(
    host="10.21.100.2",
    dbname="incidents_db",
    user="incidents_user",
    password="changeme",
)

def get_conn():
    while True:
        try:
            return psycopg2.connect(**PG_CONFIG)
        except psycopg2.OperationalError as e:
            print(f"Postgres not ready, retrying... ({e})")
            time.sleep(2)

conn = get_conn()
conn.autocommit = True
cur = conn.cursor()

# cache of type name -> type_id, so we don't hit the DB on every message
_type_id_cache = {}

def get_type_id(type_name):
    if type_name in _type_id_cache:
        return _type_id_cache[type_name]
    cur.execute("SELECT id FROM incident_type WHERE name = %s", (type_name,))
    row = cur.fetchone()
    if row is None:
        return None
    _type_id_cache[type_name] = row[0]
    return row[0]

def on_connect(client, userdata, flags, rc, properties=None):
    print(f"Connected to MQTT broker, rc={rc}")
    client.subscribe(MQTT_TOPIC)
    print(f"Subscribed to topic: {MQTT_TOPIC}")

def on_message(client, userdata, msg):
    print("Received message on topic:", msg.topic)
    try:
        payload = msg.payload.decode()
        try:
            payload_json = json.loads(payload)
            print(f"Received message on topic {msg.topic}: {payload_json}")
        except json.JSONDecodeError:
            print(f"Non-JSON payload on {msg.topic}, ignoring: {payload}")
            return

        type_name = payload_json.get("type")
        marker_id = payload_json.get("marker_id")

        if marker_id is None or type_name is None:
            print(f"Missing marker_id/type in payload, ignoring: {payload_json}")
            return

        type_id = get_type_id(type_name)
        if type_id is None:
            print(f"Unknown incident type '{type_name}', ignoring message")
            return

        cur.execute(
            "INSERT INTO incident (type_id, marker_id) VALUES (%s, %s)",
            (type_id, marker_id),
        )
    except Exception as e:
        print(f"Error handling message: {e}")

client = mqtt.Client(callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message
client.connect(MQTT_HOST, MQTT_PORT, 60)
client.loop_forever()
