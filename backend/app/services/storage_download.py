"""Download uploaded videos from Supabase Storage to disk (streamed, low RAM)."""

from __future__ import annotations

import logging
import shutil
import urllib.request
from pathlib import Path

logger = logging.getLogger(__name__)

BUCKET = "workout-videos"


def _storage_client():
    from app.core.config import get_settings
    from supabase import create_client

    settings = get_settings()
    if not settings.supabase_configured:
        raise RuntimeError("Supabase is not configured on the backend")
    return create_client(settings.supabase_url, settings.supabase_service_role_key)


def _signed_url(client, storage_path: str, expires_in: int = 3600) -> str:
    result = client.storage.from_(BUCKET).create_signed_url(storage_path, expires_in)
    if isinstance(result, dict):
        return result.get("signedURL") or result.get("signedUrl") or ""
    raise RuntimeError("Could not create signed URL for storage object")


def download_storage_object(storage_path: str, dest: Path) -> None:
    """Stream object to dest without loading the full file into memory."""
    client = _storage_client()
    url = _signed_url(client, storage_path)
    if not url:
        raise RuntimeError("Signed URL was empty")

    dest.parent.mkdir(parents=True, exist_ok=True)
    logger.info("downloading storage object %s", storage_path)

    with urllib.request.urlopen(url, timeout=600) as response:
        with dest.open("wb") as out:
            shutil.copyfileobj(response, out, length=1024 * 1024)

    logger.info("downloaded %s bytes to %s", dest.stat().st_size, dest.name)


def remove_storage_object(storage_path: str) -> None:
    try:
        client = _storage_client()
        client.storage.from_(BUCKET).remove([storage_path])
    except Exception as exc:
        logger.warning("could not remove storage object %s: %s", storage_path, exc)
