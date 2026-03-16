from typing import Literal, Optional

from pydantic import BaseModel, Field


ActionType = Literal[
    "click_text",
    "click_bbox",
    "type_text",
    "press",
    "scroll",
    "confirm_submit",
    "done",
]


class Action(BaseModel):
    action: ActionType
    target: Optional[str] = None
    x: Optional[float] = None
    y: Optional[float] = None
    value: Optional[str] = None
    key: Optional[str] = None
    direction: Optional[Literal["up", "down", "left", "right"]] = None
    amount: Optional[int] = None
    confidence: float = Field(..., ge=0.0, le=1.0)
    rationale: str


class StepResponse(BaseModel):
    step_id: int
    action: Action
