"""Transcode phone videos (e.g. iPhone HEVC .MOV) to a small MP4 for fast OpenCV/MediaPipe analysis."""

from __future__ import annotations

import logging
import subprocess
from pathlib import Path

logger = logging.getLogger(__name__)


def prepare_video_for_analysis(source: Path, max_seconds: int = 45) -> Path:
    """
    Trim to max_seconds, scale to 480p, H.264 — keeps uploads fast to process and avoids
  OpenCV struggling with large iPhone MOV files.
    """
    dest = source.with_name(f"{source.stem}_prep.mp4")
    if dest.exists():
        dest.unlink()

    cmd = [
        "ffmpeg",
        "-y",
        "-loglevel",
        "error",
        "-i",
        str(source),
        "-t",
        str(max_seconds),
        "-vf",
        "scale=480:-2",
        "-c:v",
        "libx264",
        "-preset",
        "ultrafast",
        "-crf",
        "28",
        "-movflags",
        "+faststart",
        "-an",
        str(dest),
    ]

    try:
        subprocess.run(cmd, check=True, capture_output=True, timeout=180)
        logger.info("ffmpeg prep ok %s -> %s (%s bytes)", source.name, dest.name, dest.stat().st_size)
        return dest
    except FileNotFoundError:
        logger.warning("ffmpeg not installed; analyzing original file")
        return source
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as exc:
        stderr = getattr(exc, "stderr", b"")
        if isinstance(stderr, bytes):
            stderr = stderr.decode("utf-8", errors="replace")[:500]
        logger.warning("ffmpeg prep failed (%s); using original", stderr or exc)
        return source
