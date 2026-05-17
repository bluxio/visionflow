"""Shared analyze pipeline after video is on disk."""

from __future__ import annotations

import asyncio
import gc
import logging
from pathlib import Path

from app.core.config import Settings
from app.schemas import AnalyzeResponse, ExerciseType
from app.services import analyzers

logger = logging.getLogger(__name__)


async def run_analysis(
    video_path: Path,
    exercise_type: ExerciseType,
    settings: Settings,
    *,
    skip_prep: bool = False,
) -> AnalyzeResponse:
    prep_path: Path | None = None
    analyze_path = video_path

    try:
        if exercise_type == ExerciseType.squat and not skip_prep:
            from app.services.video_prep import prepare_video_for_analysis

            prep_path = await asyncio.to_thread(
                prepare_video_for_analysis,
                video_path,
                settings.max_analyze_seconds,
                delete_source=True,
            )
            analyze_path = prep_path
            gc.collect()

        if exercise_type == ExerciseType.squat:
            from app.services.squat_pose_analyzer import analyze_squat_video

            result = await asyncio.to_thread(analyze_squat_video, str(analyze_path))
        else:
            result = analyzers.mock_analyze(exercise_type)

        logger.info("analysis done score=%s reps=%s", result.overall_score, result.rep_count)
        return result
    finally:
        for path in (prep_path,):
            if path and path.exists() and path != video_path:
                try:
                    path.unlink()
                except OSError:
                    pass
        if (
            exercise_type == ExerciseType.squat
            and not skip_prep
            and video_path.exists()
            and video_path != prep_path
        ):
            try:
                video_path.unlink()
            except OSError:
                pass
