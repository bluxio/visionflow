"""
Supabase persistence layer.

When Supabase env vars are missing, falls back to an in-memory store so local
development works without a database. Production should always configure Supabase.
"""

from __future__ import annotations

import uuid
from collections import defaultdict
from datetime import datetime, timedelta, timezone
from typing import Any

from app.core.config import get_settings
from app.schemas import AnalyzeResponse, ExerciseType, HistoryDetail, HistoryItem, Severity

# In-memory fallback: client_id -> list of records
_memory: dict[str, list[dict[str, Any]]] = defaultdict(list)


def _severity_max(feedback: list[dict]) -> str:
    order = {Severity.critical.value: 3, Severity.warning.value: 2, Severity.info.value: 1}
    best = Severity.info.value
    for item in feedback:
        sev = item.get("severity", Severity.info.value)
        if order.get(sev, 0) > order.get(best, 0):
            best = sev
    return best


def _client():
    settings = get_settings()
    if not settings.supabase_configured:
        return None
    from supabase import create_client

    return create_client(settings.supabase_url, settings.supabase_service_role_key)


def save_analysis(
    client_id: str,
    response: AnalyzeResponse,
    video_path: str | None = None,
) -> str:
    analysis_id = str(uuid.uuid4())
    now = datetime.now(timezone.utc)
    row = {
        "id": analysis_id,
        "client_id": client_id,
        "exercise_type": response.exercise_type.value,
        "overall_score": response.overall_score,
        "rep_count": response.rep_count,
        "feedback": [f.model_dump() for f in response.feedback],
        "recommendations": response.recommendations,
        "severity_max": _severity_max([f.model_dump() for f in response.feedback]),
        "video_path": video_path,
        "created_at": now.isoformat(),
    }

    sb = _client()
    if sb is None:
        _memory[client_id].insert(0, row)
        return analysis_id

    sb.table("analyses").insert(
        {
            "id": analysis_id,
            "client_id": client_id,
            "exercise_type": row["exercise_type"],
            "overall_score": row["overall_score"],
            "rep_count": row["rep_count"],
            "feedback": row["feedback"],
            "recommendations": row["recommendations"],
            "severity_max": row["severity_max"],
            "video_path": video_path,
            "created_at": row["created_at"],
        }
    ).execute()
    return analysis_id


def count_recent_analyses(client_id: str, window_hours: int) -> int:
    cutoff = datetime.now(timezone.utc) - timedelta(hours=window_hours)
    sb = _client()
    if sb is None:
        return sum(
            1
            for r in _memory.get(client_id, [])
            if datetime.fromisoformat(r["created_at"].replace("Z", "+00:00")) >= cutoff
        )

    result = (
        sb.table("analyses")
        .select("id", count="exact")
        .eq("client_id", client_id)
        .gte("created_at", cutoff.isoformat())
        .execute()
    )
    return result.count or 0


def list_recent_analyses(client_id: str, limit: int = 20) -> list[HistoryItem]:
    sb = _client()
    if sb is None:
        rows = _memory.get(client_id, [])[:limit]
    else:
        result = (
            sb.table("analyses")
            .select("id, exercise_type, overall_score, rep_count, severity_max, created_at")
            .eq("client_id", client_id)
            .order("created_at", desc=True)
            .limit(limit)
            .execute()
        )
        rows = result.data or []

    items: list[HistoryItem] = []
    for row in rows:
        created = row["created_at"]
        if isinstance(created, str):
            created_dt = datetime.fromisoformat(created.replace("Z", "+00:00"))
        else:
            created_dt = created
        items.append(
            HistoryItem(
                id=row["id"],
                exercise_type=ExerciseType(row["exercise_type"]),
                overall_score=float(row["overall_score"]),
                rep_count=int(row["rep_count"]),
                severity_max=Severity(row["severity_max"]),
                created_at=created_dt,
            )
        )
    return items


def get_analysis(client_id: str, analysis_id: str) -> HistoryDetail | None:
    sb = _client()
    if sb is None:
        row = next(
            (r for r in _memory.get(client_id, []) if r["id"] == analysis_id),
            None,
        )
    else:
        result = (
            sb.table("analyses")
            .select("*")
            .eq("client_id", client_id)
            .eq("id", analysis_id)
            .limit(1)
            .execute()
        )
        rows = result.data or []
        row = rows[0] if rows else None

    if not row:
        return None

    created = row["created_at"]
    if isinstance(created, str):
        created_dt = datetime.fromisoformat(created.replace("Z", "+00:00"))
    else:
        created_dt = created

    from app.schemas import FormFeedback

    return HistoryDetail(
        id=row["id"],
        exercise_type=ExerciseType(row["exercise_type"]),
        overall_score=float(row["overall_score"]),
        rep_count=int(row["rep_count"]),
        severity_max=Severity(row["severity_max"]),
        created_at=created_dt,
        feedback=[FormFeedback.model_validate(f) for f in row["feedback"]],
        recommendations=list(row["recommendations"]),
        video_path=row.get("video_path"),
    )
