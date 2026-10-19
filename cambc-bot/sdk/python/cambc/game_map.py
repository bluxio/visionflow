"""Static map queries: dimensions, terrain, symmetry, spawn regions and camp groups."""

from __future__ import annotations

from typing import TYPE_CHECKING, Iterator, Union

import _cambc as _bc

if TYPE_CHECKING:
    import _cambccache as _cache
else:
    from . import _cache

from . import (
    Bitboard,
    Camp,
    _CampGroupHandle,
    Error,
    ErrorCode,
    Symmetry,
    Team,
    Vec2,
)

_MASK_WALLS = 0
_MASK_BUSHES = 1
_MASK_HOLES = 2
_MASK_ALTARS = 3

def width() -> int:
    """Return the map width in tiles."""
    return _cache.width


def height() -> int:
    """Return the map height in tiles."""
    return _cache.height


def is_in_bounds(position: Vec2) -> bool:
    """Return whether a position lies on the map.

    Args:
        position: Position to check.
    """
    return 0 <= position.x < _cache.width and 0 <= position.y < _cache.height


def symmetry() -> Symmetry:
    """Return the symmetry the map was generated with."""
    return _cache.symmetry


def reflect_horizontal(position: Vec2) -> Vec2:
    """Return a position reflected across the map horizontally, reversing x.

    Args:
        position: Map position to reflect.
    """
    return _bc.reflect_horizontal_position(position.x, position.y)


def reflect_vertical(position: Vec2) -> Vec2:
    """Return a position reflected across the map vertically, reversing y.

    Args:
        position: Map position to reflect.
    """
    return _bc.reflect_vertical_position(position.x, position.y)


def rotate_180(position: Vec2) -> Vec2:
    """Return a position rotated 180 degrees within the map.

    Args:
        position: Map position to rotate.
    """
    return _bc.rotate_180_position(position.x, position.y)


def walls() -> Bitboard:
    """Cached wall mask; copy() before mutating."""
    return _cache.terrain[_MASK_WALLS]


def bushes() -> Bitboard:
    """Return the bush tiles, which hide champions from outside vision."""
    return _cache.terrain[_MASK_BUSHES]


def holes() -> Bitboard:
    """Return the hole tiles, which block movement but not vision."""
    return _cache.terrain[_MASK_HOLES]


def altars() -> Bitboard:
    """Return the altar tiles, where gems are tributed."""
    return _cache.terrain[_MASK_ALTARS]


def spawn_region(team: Team) -> Bitboard:
    """Return a team's spawn region.

    Args:
        team: Team whose spawn region to return.
    """
    if team == Team.ALLY:
        return _cache.spawns[0]
    if team == Team.ENEMY:
        return _cache.spawns[1]
    raise ValueError("invalid team")


class CampGroupAlive(_bc.CampGroupAlive):
    __slots__ = ()


class CampGroupRewardAvailable(_bc.CampGroupRewardAvailable):
    __slots__ = ()


class CampGroupRespawning(_bc.CampGroupRespawning):
    __slots__ = ()


CampGroupState = Union[CampGroupAlive, CampGroupRewardAvailable, CampGroupRespawning]
_ALIVE = CampGroupAlive()
_REWARD_AVAILABLE = CampGroupRewardAvailable()


class CampGroup:
    """A camp group; int(group) is its engine handle and iterating yields its camps."""

    __slots__ = ("_handle", "_gold", "_gems", "_respawn", "_camps", "_mask")
    _handle: _CampGroupHandle
    _gold: int
    _gems: int
    _respawn: int
    _camps: tuple[Camp, ...]
    _mask: Bitboard | None

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, CampGroup):
            return NotImplemented
        return self._handle == other._handle

    def __hash__(self) -> int:
        return hash(self._handle)

    def __init__(self) -> None:
        raise TypeError("CampGroup cannot be constructed directly")

    @classmethod
    def _new(cls, handle: _CampGroupHandle, gold: int, gems: int, respawn: int, camps: tuple[Camp, ...]) -> CampGroup:
        self = cls.__new__(cls)
        self._handle = handle
        self._gold = gold
        self._gems = gems
        self._respawn = respawn
        self._camps = camps
        self._mask = None
        return self

    def __index__(self) -> int:
        return self._handle

    def gold_value(self) -> int:
        """Return the gold awarded for clearing the group."""
        return self._gold

    def gem_value(self) -> int:
        """Return the gems awarded for clearing the group."""
        return self._gems

    def state(self) -> CampGroupState:
        """Return the group's current state."""
        if (_bc.vision() & self.spawn_tiles()).is_empty():
            raise Error(ErrorCode.API_NOT_IN_VISION, "camp group is not in vision")
        state = _bc.camp_group_state(self._handle)
        if state == 1:
            return _ALIVE
        if state == 2:
            return _REWARD_AVAILABLE
        if state == 3:
            return CampGroupRespawning(_bc.camp_group_cooldown(self._handle))
        if state == 0:
            raise Error(ErrorCode.API_NOT_IN_VISION, "camp group state is unknown")
        raise RuntimeError("unknown engine camp group state")

    def max_respawn_timer(self) -> int:
        """Return the number of rounds the group takes to respawn after being cleared."""
        return self._respawn

    def camps(self) -> tuple[Camp, ...]:
        """Return the camps in the group."""
        return self._camps

    def __iter__(self) -> Iterator[Camp]:
        return iter(self._camps)

    def __len__(self) -> int:
        return len(self._camps)

    def spawn_tiles(self) -> Bitboard:
        """Return the spawn tiles of the group's camps."""
        if self._mask is None:
            mask = Bitboard()
            for camp in self._camps:
                mask.set(camp.spawn_position())
            self._mask = mask
        return self._mask

    def spawn_tiles_into(self, out: Bitboard) -> None:
        """Like spawn_tiles, but writes into out.

        Args:
            out: Destination Bitboard; its previous contents are overwritten.
        """
        out.clear()
        out |= self.spawn_tiles()


def camp_groups() -> tuple[CampGroup, ...]:
    """Camp groups, also available during the draft."""
    groups: tuple[CampGroup, ...] = _bc.camp_groups(Camp._new, CampGroup._new)
    return groups


def camp(position: Vec2) -> Camp:
    """The camp that spawns at position; IndexError if none does.

    Args:
        position: Spawn tile of the camp.
    """
    handle = _bc.map_camp_at(position.x, position.y)
    if handle is None:
        raise IndexError("no camp on tile")
    return _camp(handle)


def _camp(handle: int) -> Camp:
    camp: Camp = _bc.camp(handle, Camp._new)
    return camp


def _camp_group(handle: _CampGroupHandle) -> CampGroup:
    group: CampGroup = _bc.camp_group(handle, Camp._new, CampGroup._new)
    return group
