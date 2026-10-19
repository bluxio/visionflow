"""Debug drawing for the visualiser."""

from __future__ import annotations

import _cambc as _bc

from . import Vec2


def draw_dot(position: Vec2, rgb: int) -> None:
    """Draw a debug dot in the replay.

    Args:
        position: Tile to mark.
        rgb: Packed 0xRRGGBB colour.
    """
    _bc.debug_dot(position.x, position.y, rgb)


def draw_line(start: Vec2, end: Vec2, rgb: int) -> None:
    """Draw a debug line in the replay.

    Args:
        start: Start tile.
        end: End tile.
        rgb: Packed 0xRRGGBB colour.
    """
    _bc.debug_line(start.x, start.y, end.x, end.y, rgb)
