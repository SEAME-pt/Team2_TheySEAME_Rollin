#!/bin/bash

set -e

echo "=== Installing PostgreSQL ==="
sudo apt update
sudo apt install postgresql postgresql-contrib -y
sudo systemctl start postgresql
sudo systemctl enable postgresql

echo "=== Creating database user and database ==="
sudo -u postgres psql <<EOF
CREATE USER incidents_user WITH PASSWORD 'changeme';
CREATE DATABASE incidents_db OWNER incidents_user;
EOF

echo "=== Loading schema ==="
sudo -u postgres psql -d incidents_db <<EOF
CREATE TABLE incident_type (
    id          SERIAL PRIMARY KEY,
    name        VARCHAR(100) NOT NULL UNIQUE
);

INSERT INTO incident_type (name) VALUES
    ('pothole'),
    ('sidewalk_parking'),
    ('road_damage');

CREATE TABLE incident (
    id           SERIAL PRIMARY KEY,
    type_id      INT          NOT NULL REFERENCES incident_type(id),
    marker_id    INT          NOT NULL,
    status       VARCHAR(20)  NOT NULL DEFAULT 'open'
                     CHECK (status IN ('open', 'resolved')),
    reported_at  TIMESTAMP    NOT NULL DEFAULT NOW(),
    resolved_at  TIMESTAMP
);

CREATE INDEX idx_incident_type    ON incident(type_id);
CREATE INDEX idx_incident_status  ON incident(status);
CREATE INDEX idx_incident_marker  ON incident(marker_id);
EOF

echo "=== Granting permissions ==="
sudo -u postgres psql -d incidents_db <<EOF
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO incidents_user;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO incidents_user;
EOF

echo "=== Setting up Python virtual environment ==="
cd incidents-api
python3 -m venv venv
source venv/bin/activate
pip install fastapi uvicorn asyncpg paho-mqtt

echo "=== Opening firewall port 8000 ==="
sudo ufw allow 8000

echo ""
echo "=== Setup complete! ==="
echo "To start the API run:"
echo "  cd incidents-api && source venv/bin/activate && uvicorn main:app --reload --host 0.0.0.0 --port 8000"
