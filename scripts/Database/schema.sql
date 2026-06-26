CREATE TABLE incident_type (
    id          SERIAL PRIMARY KEY,
    name        VARCHAR(100) NOT NULL UNIQUE
);

INSERT INTO incident_type (name) VALUES
    ('pothole'),
    ('sidewalk_parking'),

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
