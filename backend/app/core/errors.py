"""HTTP error helpers for consistent API responses."""

from fastapi import HTTPException, status

from app.schemas import QuotaErrorDetail


def quota_exceeded(limit: int, window_hours: int) -> HTTPException:
    return HTTPException(
        status_code=status.HTTP_429_TOO_MANY_REQUESTS,
        detail={
            "error": QuotaErrorDetail(
                code="quota_exceeded",
                message=(
                    "Daily free analysis limit reached. "
                    "Upgrade to Pro for unlimited analyses."
                ),
                limit=limit,
                window_hours=window_hours,
            ).model_dump()
        },
    )


def bad_request(message: str) -> HTTPException:
    return HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=message)


def not_found(message: str) -> HTTPException:
    return HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail=message)
