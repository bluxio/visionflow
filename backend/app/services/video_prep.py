"""Transcode phone videos to small MP4 for low-RAM servers (Render 512MB)."""

from __future__ import annotations

import gc
import logging
import subprocess
from pathlib import Path

from app.services.upload_chunks import chunk_dir, cleanup_chunks

logger = logging.getLogger(__name__)

_FFMPEG_BASE = [
    "ffmpeg",
    "-y",
    "-loglevel",
    "error",
    "-nostdin",
    "-threads",
    "1",
]
_SCALE = "scale=360:-2"


def _run_ffmpeg(cmd: list[str]) -> None:
    subprocess.run(cmd, check=True, capture_output=True, timeout=180)


def prepare_video_for_analysis(
    source: Path,
    max_seconds: int = 45,
    *,
    delete_source: bool = False,
) -> Path:
    """Trim, downscale to 360p H.264. Optionally delete the source file to save disk/RAM."""
    dest = source.with_name(f"{source.stem}_prep.mp4")
    if dest.exists():
        dest.unlink()

    cmd = [
        *_FFMPEG_BASE,
        "-i",
        str(source),
        "-t",
        str(max_seconds),
        "-vf",
        _SCALE,
        "-c:v",
        "libx264",
        "-preset",
        "ultrafast",
        "-crf",
        "30",
        "-movflags",
        "+faststart",
        "-an",
        str(dest),
    ]

    try:
        _run_ffmpeg(cmd)
        logger.info(
            "ffmpeg prep ok %s -> %s (%s bytes)",
            source.name,
            dest.name,
            dest.stat().st_size,
        )
        if delete_source and source.exists() and source != dest:
            source.unlink()
        gc.collect()
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


def prepare_video_from_chunks(
    upload_root: Path,
    upload_id: str,
    total_parts: int,
    max_seconds: int = 45,
) -> Path:
    """
    Stream chunks into ffmpeg via concat demuxer — avoids duplicating a 100MB+ merged file.
    """
    parts_path = chunk_dir(upload_root, upload_id)
    list_file = parts_path / "concat.txt"
    with list_file.open("w", encoding="utf-8") as handle:
        for i in range(total_parts):
            part = (parts_path / f"part_{i:05d}").resolve()
            if not part.exists():
                raise FileNotFoundError(f"Missing upload part {i}")
            handle.write(f"file '{part}'\n")

    dest = upload_root / f"{upload_id}_prep.mp4"
    if dest.exists():
        dest.unlink()

    cmd = [
        *_FFMPEG_BASE,
        "-f",
        "concat",
        "-safe",
        "0",
        "-i",
        str(list_file),
        "-t",
        str(max_seconds),
        "-vf",
        _SCALE,
        "-c:v",
        "libx264",
        "-preset",
        "ultrafast",
        "-crf",
        "30",
        "-movflags",
        "+faststart",
        "-an",
        str(dest),
    ]

    try:
        _run_ffmpeg(cmd)
        logger.info("ffmpeg concat prep ok -> %s (%s bytes)", dest.name, dest.stat().st_size)
    finally:
        cleanup_chunks(upload_root, upload_id)

    gc.collect()
    return dest
