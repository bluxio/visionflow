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
from app.services.analysis_runner import run_analysis
from app.services.upload_chunks import save_chunk

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


@router.post("/upload-chunk")
async def upload_chunk(
    upload_id: str = Form(...),
    part_index: int = Form(...),
    total_parts: int = Form(...),
    chunk: UploadFile = File(...),
    x_client_id: str | None = Header(default=None, alias="X-Client-Id"),
) -> dict[str, str]:
    if not x_client_id:
        raise bad_request("X-Client-Id header is required")
    if total_parts < 1 or total_parts > 80:
        raise bad_request("Invalid total_parts")
    if part_index < 0 or part_index >= total_parts:
        raise bad_request("Invalid part_index")

    settings = get_settings()
    upload_root = Path(settings.upload_dir)
    await asyncio.to_thread(save_chunk, upload_root, upload_id, part_index, chunk.file)
    logger.info("chunk %s part %s/%s", upload_id[:8], part_index + 1, total_parts)
    return {"status": "ok", "part_index": str(part_index)}


@router.post("/analyze-assembled", response_model=AnalyzeResponse)
async def analyze_assembled(
    upload_id: str = Form(...),
    total_parts: int = Form(...),
    filename: str = Form("video.mov"),
    exercise_type: ExerciseType = Form(default=ExerciseType.squat),
    x_client_id: str | None = Header(default=None, alias="X-Client-Id"),
) -> AnalyzeResponse:
    if not x_client_id:
        raise bad_request("X-Client-Id header is required")

    _check_quota(x_client_id)

    settings = get_settings()
    upload_root = Path(settings.upload_dir)
    upload_root.mkdir(parents=True, exist_ok=True)

    logger.info(
        "analyze-assembled start client=%s upload=%s parts=%s file=%s",
        x_client_id[:8],
        upload_id[:8],
        total_parts,
        filename,
    )

    prep_path: Path | None = None
    try:
        from app.services.video_prep import prepare_video_from_chunks

        prep_path = await asyncio.to_thread(
            prepare_video_from_chunks,
            upload_root,
            upload_id,
            total_parts,
            settings.max_analyze_seconds,
        )

        max_bytes = settings.max_upload_mb * 1024 * 1024
        size = prep_path.stat().st_size
        logger.info("prep video %s bytes", size)
        if size > max_bytes:
            raise bad_request(
                f"Video too large ({size // (1024 * 1024)}MB). "
                f"Maximum upload is {settings.max_upload_mb}MB."
            )

        result = await run_analysis(
            prep_path, exercise_type, settings, skip_prep=True
        )
        return _persist(x_client_id, result, str(prep_path))
    finally:
        if prep_path and prep_path.exists():
            try:
                os.remove(prep_path)
            except OSError:
                pass


@router.post("/analyze-upload", response_model=AnalyzeResponse)
async def analyze_upload(
    file: UploadFile = File(...),
    exercise_type: ExerciseType = Form(default=ExerciseType.squat),
    x_client_id: str | None = Header(default=None, alias="X-Client-Id"),
) -> AnalyzeResponse:
    """Single-request upload for smaller files."""
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

    logger.info(
        "analyze-upload start client=%s exercise=%s file=%s",
        x_client_id[:8],
        exercise_type,
        file.filename,
    )

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

        result = await run_analysis(temp_path, exercise_type, settings)
        return _persist(x_client_id, result, str(temp_path))
    finally:
        if temp_path.exists():
            try:
                os.remove(temp_path)
            except OSError:
                pass
