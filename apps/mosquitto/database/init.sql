CREATE TABLE mqtt_messages (
    id SERIAL PRIMARY KEY,
    marker_id TEXT NOT NULL,
    type TEXT NOT NULL,
    received_at TIMESTAMPTZ DEFAULT now()
);

CREATE INDEX idx_mqtt_marker_id ON mqtt_messages (marker_id);
CREATE INDEX idx_mqtt_type ON mqtt_messages (type);
CREATE INDEX idx_mqtt_received_at ON mqtt_messages (received_at);