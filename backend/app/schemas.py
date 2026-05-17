"""Pydantic models shared across routes and services."""

from datetime import datetime
from enum import Enum
from typing import Literal

from pydantic import BaseModel, Field


class ExerciseType(str, Enum):
    squat = "squat"
    deadlift = "deadlift"
    bench_press = "bench_press"
    barbell_row = "barbell_row"


class Severity(str, Enum):
    info = "info"
    warning = "warning"
    critical = "critical"


class FormFeedback(BaseModel):
    aspect: str
    score: float = Field(ge=0, le=100)
    feedback: str
    severity: Severity


class AnalyzeRequest(BaseModel):
    video_url: str
    exercise_type: ExerciseType = ExerciseType.squat


class AnalyzeResponse(BaseModel):
    exercise_type: ExerciseType
    overall_score: float = Field(ge=0, le=100)
    rep_count: int = Field(ge=0)
    feedback: list[FormFeedback]
    recommendations: list[str]


class HistoryItem(BaseModel):
    id: str
    exercise_type: ExerciseType
    overall_score: float
    rep_count: int
    severity_max: Severity
    created_at: datetime


class HistoryDetail(HistoryItem):
    feedback: list[FormFeedback]
    recommendations: list[str]
    video_path: str | None = None


class QuotaErrorDetail(BaseModel):
    code: Literal["quota_exceeded"] = "quota_exceeded"
    message: str
    limit: int
    window_hours: int
