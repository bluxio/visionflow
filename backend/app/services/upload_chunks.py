"""Assemble multipart chunk uploads on disk."""

from __future__ import annotations

import shutil
from pathlib import Path


def chunk_dir(upload_root: Path, upload_id: str) -> Path:
    return upload_root / "chunks" / upload_id


def save_chunk(upload_root: Path, upload_id: str, part_index: int, data) -> None:
    directory = chunk_dir(upload_root, upload_id)
    directory.mkdir(parents=True, exist_ok=True)
    part_path = directory / f"part_{part_index:05d}"
    with part_path.open("wb") as out:
        shutil.copyfileobj(data, out)


def merge_chunks(upload_root: Path, upload_id: str, total_parts: int, dest: Path) -> None:
    directory = chunk_dir(upload_root, upload_id)
    with dest.open("wb") as out:
        for i in range(total_parts):
            part_path = directory / f"part_{i:05d}"
            if not part_path.exists():
                raise FileNotFoundError(f"Missing upload part {i} of {total_parts}")
            with part_path.open("rb") as inp:
                shutil.copyfileobj(inp, out)


def cleanup_chunks(upload_root: Path, upload_id: str) -> None:
    directory = chunk_dir(upload_root, upload_id)
    if directory.exists():
        shutil.rmtree(directory, ignore_errors=True)
