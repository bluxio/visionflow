"""Analysis endpoints: URL-based mock and multipart upload."""

import os
import shutil
import uuid
from pathlib import Path

from fastapi import APIRouter, File, Form, Header, UploadFile

from app.core.config import get_settings
from app.core.errors import bad_request, quota_exceeded
from app.schemas import AnalyzeRequest, AnalyzeResponse, ExerciseType
from app.services import analyzers, supabase_store
from app.services.squat_pose_analyzer import analyze_squat_video

router = APIRouter(prefix="", tags=["analyze"])


def _check_quota(client_id: str | None) -> None:
    if not client_id:
        return
    settings = get_settings()
    count = supabase_store.count_recent_analyses(client_id, settings.quota_window_hours)
    if count >= settings.daily_analysis_limit:
        raise quota_exceeded(settings.daily_analysis_limit, settings.quota_window_hours)


def _persist(client_id: str | None, result: AnalyzeResponse, video_path: str | None) -> AnalyzeResponse:
    if client_id:
        supabase_store.save_analysis(client_id, result, video_path=video_path)
    return result


@router.post("/analyze", response_model=AnalyzeResponse)
async def analyze_json(body: AnalyzeRequest) -> AnalyzeResponse:
    """Mock analysis from a video URL (no pose pipeline yet)."""
    _ = body.video_url
    return analyzers.mock_analyze(body.exercise_type)


@router.post("/analyze-upload", response_model=AnalyzeResponse)
async def analyze_upload(
    file: UploadFile = File(...),
    exercise_type: ExerciseType = Form(default=ExerciseType.squat),
    x_client_id: str | None = Header(default=None, alias="X-Client-Id"),
) -> AnalyzeResponse:
    if not x_client_id:
        raise bad_request("X-Client-Id header is required")

    _check_quota(x_client_id)

    if not file.filename:
        raise bad_request("Uploaded file must have a filename")

    settings = get_settings()
    upload_root = Path(settings.upload_dir)
    upload_root.mkdir(parents=True, exist_ok=True)

    suffix = Path(file.filename).suffix or ".mp4"
    temp_path = upload_root / f"{uuid.uuid4()}{suffix}"

    try:
        with temp_path.open("wb") as out:
            shutil.copyfileobj(file.file, out)

        if exercise_type == ExerciseType.squat:
            try:
                result = analyze_squat_video(str(temp_path))
            except Exception as exc:
                raise bad_request(f"Squat analysis failed: {exc}") from exc
        else:
            result = analyzers.mock_analyze(exercise_type)

        return _persist(x_client_id, result, str(temp_path))
    finally:
        if temp_path.exists():
            try:
                os.remove(temp_path)
            except OSError:
                pass
