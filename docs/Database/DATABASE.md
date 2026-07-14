# Incident Reporting Database

REST API for reporting road incidents (potholes, sidewalk parking, road damage) detected by the Piracer. Location is encoded via ArUco marker IDs instead of GPS coordinates.

---

## Stack

- **Database:** PostgreSQL
- **API:** FastAPI + asyncpg (Python)
- **Docs:** Swagger UI at `/docs`

---

## Schema

### `incident_type`
Lookup table for incident categories.

| Column | Type | Description |
|--------|------|-------------|
| `id` | SERIAL PK | Auto-incremented ID |
| `name` | VARCHAR | Unique name (e.g. `pothole`) |

Default types loaded on setup:
- `pothole` — Hole or damage in the road surface
- `sidewalk_parking` — Vehicle parked illegally on the sidewalk


### `incident`
One row per reported incident.

| Column | Type | Description |
|--------|------|-------------|
| `id` | SERIAL PK | Auto-incremented ID |
| `type_id` | INT FK | References `incident_type.id` |
| `marker_id` | INT | ArUco marker ID where the incident was detected |
| `status` | VARCHAR | `open` or `resolved` |
| `reported_at` | TIMESTAMP | Set automatically on insert |
| `resolved_at` | TIMESTAMP | Set automatically when status → `resolved` |

---

## API Endpoints

Base URL: `http://<host>:8000`

### Incident Types

| Method | Path | Description |
|--------|------|-------------|
| GET | `/types` | List all incident types |
| POST | `/types` | Create a new incident type |
| DELETE | `/types/{id}` | Delete an incident type |

### Incidents

| Method | Path | Description |
|--------|------|-------------|
| GET | `/incidents` | List all incidents (supports `?status=` and `?type_id=` filters) |
| GET | `/incidents/{id}` | Get a single incident |
| POST | `/incidents` | Report a new incident |
| PATCH | `/incidents/{id}` | Update incident status |
| DELETE | `/incidents/{id}` | Delete an incident |

### POST `/types` — Request body

```json
{
  "name": "fallen_tree"
}
```

### POST `/incidents` — Request body

```json
{
  "type_id": 1,
  "marker_id": 42,
}
```

### PATCH `/incidents/{id}` — Request body

```json
{
  "status": "resolved"
}
```

Valid status values: `open`, `resolved`.

---

## Running Locally

### 1. Run the setup script

A setup script is provided to automate the full installation on a new machine (PostgreSQL, database, schema, Python dependencies, firewall):

```bash
chmod +x setup.sh
./setup.sh
```

### 2. Configure the database URL

The default connection string is:
```
postgresql://incidents_user:changeme@localhost/incidents_db
```

Override it with the `DATABASE_URL` environment variable if needed:
```bash
export DATABASE_URL="postgresql://user:password@host/dbname"
```

### 3. Start the server

```bash
cd incidents-api
source venv/bin/activate
uvicorn main:app --reload --host 0.0.0.0 --port 8000
```

The interactive API docs will be available at **http://localhost:8000/docs**.

---

## Accessing from Another Machine

The server binds to `0.0.0.0`, so it is reachable from other machines on the same network. Make sure port `8000` is open on the firewall:

```bash
sudo ufw allow 8000
```

Then access it via the host's local IP:
```
http://<host-ip>:8000/docs
```
