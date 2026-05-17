"""
Workout Form Coach API.

Architecture:
- `routes/` — HTTP handlers, validation, headers
- `services/` — business logic (pose analysis, storage, mocks)
- `core/` — config and shared errors
- `schemas.py` — Pydantic contracts shared with clients
"""

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from app.core.config import get_settings
from app.routes import analyze, history

settings = get_settings()

app = FastAPI(
    title="Workout Form Coach API",
    description="Upload-based workout form analysis with coaching feedback.",
    version="1.0.0",
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.cors_origin_list,
    allow_origin_regex=r"https://.*\.vercel\.app",
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(analyze.router)
app.include_router(history.router)


@app.get("/health")
def health() -> dict[str, str]:
    return {"status": "ok"}
