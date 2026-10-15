"""Game constants, read once from the engine settings on first use."""

from __future__ import annotations

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    import _cambccache as _cache
else:
    from . import _cache

CAPTURE_TIME: int
CAMP_SLEEP_TIME: int
RESPAWN_TIME: int
RECALL_TIME: int
SUBMIT_GEM_DURATION_MULTIPLIER: float
SUBMIT_GEM_BASE_DURATION: int
SUBMIT_GEM_MINIMUM_DURATION: int
SUBMIT_GEMS_RANGE: int
MAX_ROUNDS: int
WINNING_GEMS: int
KILL_GOLD: int
ASSIST_GOLD: int
PASSIVE_GOLD: int
SPAWN_HEALING: float
ACTIVE_WARDS: int
SHARED_MEMORY_BYTES: int
DRAFT_MEMORY_BYTES: int
TILE_REVEAL_ROUNDS: int
WARD_COOLDOWN: int
WARD_LIFETIME: int
WARD_VISION_RADIUS: float
WARD_PLACEMENT_RADIUS: float
ALTAR_LOCK_HITS: int
TP_CRYSTAL_TIME: int

_SETTINGS = (
    "CAPTURE_TIME",
    "CAMP_SLEEP_TIME",
    "RESPAWN_TIME",
    "RECALL_TIME",
    "SUBMIT_GEM_DURATION_MULTIPLIER",
    "SUBMIT_GEM_BASE_DURATION",
    "SUBMIT_GEM_MINIMUM_DURATION",
    "SUBMIT_GEMS_RANGE",
    "MAX_ROUNDS",
    "WINNING_GEMS",
    "KILL_GOLD",
    "ASSIST_GOLD",
    "PASSIVE_GOLD",
    "SPAWN_HEALING",
    "ACTIVE_WARDS",
    "SHARED_MEMORY_BYTES",
    "DRAFT_MEMORY_BYTES",
    "TILE_REVEAL_ROUNDS",
    "WARD_COOLDOWN",
    "WARD_LIFETIME",
    "WARD_VISION_RADIUS",
    "WARD_PLACEMENT_RADIUS",
    "ALTAR_LOCK_HITS",
    "TP_CRYSTAL_TIME",
)
__all__ = [*_SETTINGS, "passive_abilities"]


def __getattr__(name: str) -> object:
    if name not in _SETTINGS:
        raise AttributeError(f"module 'cambc.constants' has no attribute {name!r}")
    globals().update(zip(_SETTINGS, _cache.settings))
    return globals()[name]


class passive_abilities:
    """Fixed passive-ability tuning values."""

    __slots__ = ()
    PRISM_FOCUS_PER_GEM = 0.2
    VARG_SPLASH_RADIUS = 1.0
    VESPER_VALE_BONUS_GOLD_PERCENT = 20
    RYTH_HEALTH_STRENGTH_RATIO = 0.01
    REN_RETALIATION_FOCUS_RATIO = 0.2
