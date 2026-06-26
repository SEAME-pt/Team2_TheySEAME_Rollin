from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from typing import Optional
from datetime import datetime
import asyncpg
import os

DATABASE_URL = os.getenv(
    "DATABASE_URL",
    "postgresql://incidents_user:changeme@localhost/incidents_db"
)

app = FastAPI(title="Incidents API")
pool: asyncpg.Pool = None

@app.on_event("startup")
async def startup():
    global pool
    pool = await asyncpg.create_pool(DATABASE_URL)

@app.on_event("shutdown")
async def shutdown():
    await pool.close()


# --- Models ---

class IncidentCreate(BaseModel):
    type_id: int
    marker_id: int

class IncidentUpdate(BaseModel):
    status: str  # open | in_progress | resolved

class IncidentTypeCreate(BaseModel):
    name: str
    description: Option[str] = None

# --- Incident types ---

@app.get("/types")
async def list_types():
    rows = await pool.fetch("SELECT * FROM incident_type ORDER BY id")
    return [dict(r) for r in rows]

@app.post("/types", status_code=201)
async def create_type(body: IncidentTypeCreate):
    try:
        row = await pool.fetchrow(
                """
                INSERT INTO incident_type (name, description)
                VALUES($1, $2)
                RETURNING *
                """,
                body.name, body.description
                )
        return dict(row)
    except asyncpg.UniqueViolationError:
        raise HTTPException(status_code=409, detail=f"Type '{body.name}' already exists")

@app.delete("types/{types_id}", status_code=204)
async def delete_type(type_id: int)
    result = await pool.execute("DELETE FROM incident_type WHERE id = $1", type_id)
    if result == "DELETE 0":
        raise HTTPException(status_code=404, detail="Incident type not found")
# --- Incidents ---

@app.get("/incidents")
async def list_incidents(status: Optional[str] = None, type_id: Optional[int] = None):
    query = "SELECT * FROM incident WHERE 1=1"
    params = []
    if status:
        params.append(status)
        query += f" AND status = ${len(params)}"
    if type_id:
        params.append(type_id)
        query += f" AND type_id = ${len(params)}"
    query += " ORDER BY reported_at DESC"
    rows = await pool.fetch(query, *params)
    return [dict(r) for r in rows]

@app.get("/incidents/{incident_id}")
async def get_incident(incident_id: int):
    row = await pool.fetchrow("SELECT * FROM incident WHERE id = $1", incident_id)
    if not row:
        raise HTTPException(status_code=404, detail="Incident not found")
    return dict(row)

@app.post("/incidents", status_code=201)
async def create_incident(body: IncidentCreate):
    row = await pool.fetchrow(
        """
        INSERT INTO incident (type_id, marker_id)
        VALUES ($1, $2)
        RETURNING *
        """,
        body.type_id, body.marker_id
    )
    return dict(row)

@app.patch("/incidents/{incident_id}")
async def update_status(incident_id: int, body: IncidentUpdate):
    valid = {"open", "in_progress", "resolved"}
    if body.status not in valid:
        raise HTTPException(status_code=400, detail=f"status must be one of {valid}")
    resolved_at = datetime.utcnow() if body.status == "resolved" else None
    row = await pool.fetchrow(
        """
        UPDATE incident
        SET status = $1, resolved_at = $2
        WHERE id = $3
        RETURNING *
        """,
        body.status, resolved_at, incident_id
    )
    if not row:
        raise HTTPException(status_code=404, detail="Incident not found")
    return dict(row)

@app.delete("/incidents/{incident_id}", status_code=204)
async def delete_incident(incident_id: int):
    result = await pool.execute("DELETE FROM incident WHERE id = $1", incident_id)
    if result == "DELETE 0":
        aise HTTPException(status_code=404, detail="Incident not found")
