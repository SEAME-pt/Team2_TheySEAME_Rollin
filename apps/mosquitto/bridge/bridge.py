import os
import json
import time
import paho.mqtt.client as mqtt
import psycopg2

MQTT_HOST = os.environ["MQTT_HOST"]
MQTT_PORT = int(os.environ.get("MQTT_PORT", 1883))
MQTT_TOPIC = os.environ.get("MQTT_TOPIC", "#")

PG_CONFIG = dict(
    host=os.environ["PG_HOST"],
    dbname=os.environ["PG_DB"],
    user=os.environ["PG_USER"],
    password=os.environ["PG_PASSWORD"],
)

def get_conn():
    while True:
        try:
            return psycopg2.connect(**PG_CONFIG)
        except psycopg2.OperationalError:
            print("Postgres not ready, retrying...")
            time.sleep(2)

conn = get_conn()
conn.autocommit = True
cur = conn.cursor()

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
            payload_json = {"raw": payload}

        cur.execute(
            "INSERT INTO mqtt_messages (marker_id, type) VALUES (%s, %s)",
            (payload_json.get("marker_id"), payload_json.get("type")),
        )
    except Exception as e:
        print(f"Error handling message: {e}")

client = mqtt.Client(callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message
client.connect(MQTT_HOST, MQTT_PORT, 60)
client.loop_forever()