"""Analysis endpoints: URL-based mock and multipart upload."""

import asyncio
import logging
import os
import shutil
import uuid
from pathlib import Path

from fastapi import APIRouter, File, Form, Header, UploadFile

from app.core.config import get_settings
from app.core.errors import bad_request, quota_exceeded
from app.schemas import AnalyzeRequest, AnalyzeResponse, ExerciseType
from app.services import analyzers, supabase_store

router = APIRouter(prefix="", tags=["analyze"])
logger = logging.getLogger(__name__)


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

    upload_root = Path(get_settings().upload_dir)
    upload_root.mkdir(parents=True, exist_ok=True)

    suffix = Path(file.filename).suffix or ".mp4"
    temp_path = upload_root / f"{uuid.uuid4()}{suffix}"

    settings = get_settings()
    logger.info("analyze-upload start client=%s exercise=%s file=%s", x_client_id[:8], exercise_type, file.filename)

    prep_path: Path | None = None
    try:
        with temp_path.open("wb") as out:
            shutil.copyfileobj(file.file, out)

        max_bytes = settings.max_upload_mb * 1024 * 1024
        size = temp_path.stat().st_size
        logger.info("upload saved %s bytes", size)
        if size > max_bytes:
            raise bad_request(
                f"Video too large ({size // (1024 * 1024)}MB). "
                f"Maximum upload is {settings.max_upload_mb}MB."
            )

        analyze_path = temp_path
        if exercise_type == ExerciseType.squat:
            from app.services.video_prep import prepare_video_for_analysis

            prep_path = await asyncio.to_thread(
                prepare_video_for_analysis, temp_path, settings.max_analyze_seconds
            )
            analyze_path = prep_path

        if exercise_type == ExerciseType.squat:
            try:
                from app.services.squat_pose_analyzer import analyze_squat_video

                result = await asyncio.to_thread(analyze_squat_video, str(analyze_path))
            except Exception as exc:
                logger.exception("squat analysis failed")
                raise bad_request(f"Squat analysis failed: {exc}") from exc
        else:
            result = analyzers.mock_analyze(exercise_type)

        logger.info(
            "analyze-upload done client=%s score=%s reps=%s",
            x_client_id[:8],
            result.overall_score,
            result.rep_count,
        )
        return _persist(x_client_id, result, str(temp_path))
    finally:
        for path in (prep_path, temp_path):
            if path and path.exists():
                try:
                    os.remove(path)
                except OSError:
                    pass
