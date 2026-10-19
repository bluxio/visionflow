"""Queries about what this bot's team can currently see."""

from __future__ import annotations

from typing import Optional

import _cambc as _bc

from . import AltarLock, Bitboard, Camp, Champion, Error, ErrorCode, Team, Teleport, Vec2, Ward, _absolute_team
from . import game_map as _map


def champion_tiles(team: Optional[Team] = None) -> Bitboard:
    """Visible champions of a team, or of both teams.

    Args:
        team: Team to include, or None for both teams.
    """
    if team is None:
        return _bc.ally_occupancy() | _bc.enemy_occupancy()
    if team == Team.ALLY:
        return _bc.ally_occupancy()
    if team == Team.ENEMY:
        return _bc.enemy_occupancy()
    raise ValueError("invalid team")


def champion_tiles_into(out: Bitboard, team: Optional[Team] = None) -> None:
    """Like champion_tiles, but writes into out.

    Args:
        out: Destination Bitboard; its previous contents are overwritten.
        team: Team to include, or None for both teams.
    """
    if team is None:
        _bc.ally_occupancy_into(out)
        out |= _bc.enemy_occupancy()
    elif team == Team.ALLY:
        _bc.ally_occupancy_into(out)
    elif team == Team.ENEMY:
        _bc.enemy_occupancy_into(out)
    else:
        raise ValueError("invalid team")


def camp_tiles() -> Bitboard:
    """Return the tiles of visible camps."""
    return _bc.camp_occupancy()


def camp_tiles_into(out: Bitboard) -> None:
    """Like camp_tiles, but writes into out.

    Args:
        out: Destination Bitboard; its previous contents are overwritten.
    """
    _bc.camp_occupancy_into(out)


def ward_tiles(team: Optional[Team] = None) -> Bitboard:
    """Return the tiles of visible wards.

    Args:
        team: Team to include, or None for both teams.
    """
    if team is None:
        return _bc.ally_ward_occupancy() | _bc.enemy_ward_occupancy()
    _absolute_team(team)
    return _bc.ally_ward_occupancy() if team == Team.ALLY else _bc.enemy_ward_occupancy()


def ward_tiles_into(out: Bitboard, team: Optional[Team] = None) -> None:
    """Like ward_tiles, but writes into out.

    Args:
        out: Destination Bitboard; its previous contents are overwritten.
        team: Team to include, or None for both teams.
    """
    if team is None:
        _bc.ally_ward_occupancy_into(out)
        out |= _bc.enemy_ward_occupancy()
        return
    _absolute_team(team)
    (_bc.ally_ward_occupancy_into if team == Team.ALLY else _bc.enemy_ward_occupancy_into)(out)


def wards(team: Optional[Team] = None) -> list[Ward]:
    """Snapshots of visible wards, optionally of one team.

    Args:
        team: Team to include, or None for both teams.
    """
    return _bc.wards(-1 if team is None else _absolute_team(team))


def ward_at(position: Vec2, team: Optional[Team] = None) -> Ward:
    """Snapshot of the ward on a tile, optionally of one team.

    Args:
        position: Tile to query.
        team: Team to restrict to, or None for either team.
    """
    if team is None:
        handle = _bc.ward_at(position.x, position.y)
    else:
        _absolute_team(team)
        handle = (_bc.ally_ward_at if team == Team.ALLY else _bc.enemy_ward_at)(position.x, position.y)
    return _bc.ward_info(handle)


def has_ward(position: Vec2, team: Optional[Team] = None) -> bool:
    """Return whether a visible ward is on a tile; hidden wards return False.

    Args:
        position: Tile to query.
        team: Team to restrict to, or None for either team.
    """
    if team is not None:
        _absolute_team(team)
    return _bc.has_ward(position.x, position.y, -1 if team is None else team)


def teleport_tiles() -> Bitboard:
    """Return the tiles of visible teleport reservations."""
    return _bc.teleport_occupancy()


def teleport_tiles_into(out: Bitboard) -> None:
    """Like teleport_tiles, but writes into out.

    Args:
        out: Destination Bitboard; its previous contents are overwritten.
    """
    _bc.teleport_occupancy_into(out)


def visible_tiles() -> Bitboard:
    """Return every tile visible to the bot's champion."""
    return _bc.vision()


def visible_tiles_into(out: Bitboard) -> None:
    """Like visible_tiles, but writes into out.

    Args:
        out: Destination Bitboard; its previous contents are overwritten.
    """
    _bc.vision_into(out)


def has_teleport(position: Vec2) -> bool:
    """Return whether a visible teleport reservation is on a tile.

    Args:
        position: Tile to query.
    """
    return _bc.has_teleport(position.x, position.y)


def teleports() -> list[Teleport]:
    """Return snapshots of visible teleport reservations."""
    return _bc.teleports()


def teleport_at(position: Vec2) -> Teleport:
    """The teleport reservation on a tile.

    Args:
        position: Tile to query.
    """
    return _bc.teleport_at(position.x, position.y)

def altar_lock_tiles(team: Optional[Team] = None) -> Bitboard:
    """Return the tiles of visible altar locks.

    Args:
        team: Team to include, or None for both teams.
    """
    if team is None:
        return _bc.ally_altar_lock_occupancy() | _bc.enemy_altar_lock_occupancy()
    _absolute_team(team)
    return _bc.ally_altar_lock_occupancy() if team == Team.ALLY else _bc.enemy_altar_lock_occupancy()


def altar_lock_tiles_into(out: Bitboard, team: Optional[Team] = None) -> None:
    """Like altar_lock_tiles, but writes into out.

    Args:
        out: Destination Bitboard; its previous contents are overwritten.
        team: Team to include, or None for both teams.
    """
    if team is None:
        _bc.ally_altar_lock_occupancy_into(out)
        out |= _bc.enemy_altar_lock_occupancy()
        return
    _absolute_team(team)
    (_bc.ally_altar_lock_occupancy_into if team == Team.ALLY else _bc.enemy_altar_lock_occupancy_into)(out)


def altar_lock_at(position: Vec2) -> AltarLock:
    """The altar lock on a tile, ally first.

    Args:
        position: Tile to query.
    """
    return _bc.altar_lock(position.x, position.y)


def has_altar_lock(position: Vec2) -> bool:
    """Return whether a visible altar lock of either team is on a tile.

    Args:
        position: Tile to query.
    """
    return _bc.has_altar_lock(position.x, position.y)


def altar_locks(team: Optional[Team] = None) -> list[AltarLock]:
    """Return snapshots of visible altar locks.

    Args:
        team: Team to include, or None for both teams.
    """
    return _bc.altar_locks(-1 if team is None else _absolute_team(team))


def camp_at(position: Vec2) -> Camp:
    """Return the visible camp currently on a tile; raises Error if there is none or the tile is hidden.

    Args:
        position: Tile to query.
    """
    return _map._camp(_bc.camp_at(position.x, position.y))


def champion_at(position: Vec2) -> Champion:
    """Return the visible champion on a tile; raises Error if there is none or the tile is hidden.

    Args:
        position: Tile to query.
    """
    return Champion._new(_bc.champion_at(position.x, position.y))


def has_camp(position: Vec2) -> bool:
    """Return whether a visible camp is on a tile.

    Args:
        position: Tile to query.
    """
    return _bc.has_camp(position.x, position.y)


def has_champion(position: Vec2) -> bool:
    """Return whether a visible champion is on a tile.

    Args:
        position: Tile to query.
    """
    return _bc.has_champion(position.x, position.y)


def champions(team: Optional[Team] = None) -> list[Champion]:
    """Return the visible champions.

    Args:
        team: Team to include, or None for both teams.
    """
    return _bc.champions_in(champion_tiles(team), Champion._new)


def camps() -> list[Camp]:
    """Return the visible camps."""
    return [camp_at(position) for position in camp_tiles().positions()]


def camp_groups() -> list[_map.CampGroup]:
    """Camp groups with at least one spawn tile in vision."""
    return _bc.in_vision(_map.camp_groups())
