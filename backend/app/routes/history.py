"""History endpoints scoped by X-Client-Id."""

from fastapi import APIRouter, Header

from app.core.errors import bad_request, not_found
from app.schemas import HistoryDetail, HistoryItem
from app.services import supabase_store

router = APIRouter(prefix="/history", tags=["history"])


def _require_client_id(x_client_id: str | None) -> str:
    if not x_client_id:
        raise bad_request("X-Client-Id header is required")
    return x_client_id


@router.get("", response_model=list[HistoryItem])
async def list_history(
    x_client_id: str | None = Header(default=None, alias="X-Client-Id"),
    limit: int = 20,
) -> list[HistoryItem]:
    client_id = _require_client_id(x_client_id)
    return supabase_store.list_recent_analyses(client_id, limit=limit)


@router.get("/{analysis_id}", response_model=HistoryDetail)
async def get_history_item(
    analysis_id: str,
    x_client_id: str | None = Header(default=None, alias="X-Client-Id"),
) -> HistoryDetail:
    client_id = _require_client_id(x_client_id)
    detail = supabase_store.get_analysis(client_id, analysis_id)
    if not detail:
        raise not_found("Analysis not found")
    return detail
