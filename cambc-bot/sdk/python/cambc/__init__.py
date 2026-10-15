"""Cambridge Battlecode Python SDK.

A Pythonic translation of the C++ bc API: namespaces become submodules (game_map, vision, draft,
debug, constants), overloads become optional parameters, ranges become iterables, and engine
failures raise Error.
"""

from __future__ import annotations

from typing import TYPE_CHECKING, ClassVar, Iterator, Optional, TypeVar, Union

import builtins as _builtins
import sys as _sys
import _cambc as _bc
from _cambc import (
    AbilityAoeDamage,
    AbilityAttackReset,
    AbilityCc,
    AbilityDamage,
    AbilityDash,
    AbilityHeal,
    AbilityShield,
    Bitboard,
    BuffEnhancedAttack,
    BuffShield,
    BuffStun,
    CampStats,
    ChampionStats,
    ConsumableInfo,
    DistanceField,
    ItemInfo,
    Vec2,
)

_COUNT_OMITTED = object()

if TYPE_CHECKING:
    import _cambccache as _cache
else:

    class _LazyCache:
        """Imports _cambccache, which reads the engine, on first attribute access.

        cambc is frozen into the Wizer snapshot, where engine calls trap, so the
        cache must be built at match time.
        """

        def __getattr__(self, name):
            import _cambccache

            self.__dict__.update((k, v) for k, v in vars(_cambccache).items() if not k.startswith("__"))
            return getattr(_cambccache, name)

    _cache = _LazyCache()

if TYPE_CHECKING:
    from enum import IntEnum
    from typing import NewType

    _ChampionHandle = NewType("_ChampionHandle", int)
    _CampHandle = NewType("_CampHandle", int)
    _CampGroupHandle = NewType("_CampGroupHandle", int)
else:
    _ChampionHandle = _CampHandle = _CampGroupHandle = int

_A = TypeVar("_A")

if TYPE_CHECKING:

    class Team(IntEnum):
        """Relative team of a unit or object."""

        @staticmethod
        def to_string(value: int) -> str: ...

        @staticmethod
        def opponent(team: Team) -> Team: ...

        ALLY = 0
        ENEMY = 1

    class Symmetry(IntEnum):
        """Map symmetry."""

        @staticmethod
        def to_string(value: int) -> str: ...

        HORIZONTAL = 0
        VERTICAL = 1
        ROTATIONAL180 = 2

    class CampType(IntEnum):
        """Neutral camp type."""

        @staticmethod
        def to_string(value: int) -> str: ...

        WOLF = 0
        ELK = 1
        GEM_CLUSTER = 2
        GOLD_CLUSTER = 3
        MONOLITH = 4

    class PassiveType(IntEnum):
        """Champion passive."""

        @staticmethod
        def to_string(value: int) -> str: ...

        NONE = 0
        BONUS_GOLD = 1
        REMOTE_SHOP = 2
        GEM_FOCUS = 3
        SPLASH_ATTACK = 4
        HEALTH_STRENGTH = 5
        RESET_COOLDOWNS_ON_KILL = 6
        RETALIATION = 7
        STUN_IMMUNITY = 8

    class ChampionType(IntEnum):
        """Playable champion type."""

        @staticmethod
        def to_string(value: int) -> str: ...

        RYTH = 0
        VESPER_VALE = 1
        VENDETTA = 2
        CASSI = 3
        ISSAC = 4
        STELLAR = 5
        PRISM = 6
        MARCELLA = 7
        WILLOW = 8
        REN = 9
        VARG = 10
        TAVI = 11

    class ChampionDraft(IntEnum):
        """Draft pick."""

        @staticmethod
        def to_string(value: int) -> str: ...

        RYTH = 0
        VESPER_VALE = 1
        VENDETTA = 2
        CASSI_AND_ISSAC = 3
        STELLAR = 4
        PRISM = 5
        MARCELLA = 6
        WILLOW = 7
        REN = 8
        VARG = 9
        TAVI = 10

    class AbilityKind(IntEnum):
        """Ability property kind."""

        @staticmethod
        def to_string(value: int) -> str: ...

        ATTACK_RESET = 1
        DAMAGE = 2
        AOE_DAMAGE = 3
        CC = 4
        HEAL = 5
        DASH = 6
        SHIELD = 7

    class _EntityKind(IntEnum):
        """Kind of a targetable entity."""

        CHAMPION = 1
        CAMP = 2
        ALTAR_LOCK = 6

    class BuffTypes(IntEnum):
        """Buff kind."""

        @staticmethod
        def to_string(value: int) -> str: ...

        SHIELD = 0
        STUN = 1
        ENHANCED_ATTACK = 2
        COUNT = 3

    class ChampionState(_bc.FlagSet):
        """Champion state bits."""

        def is_dead(self) -> bool: ...

        ALIVE: ClassVar[ChampionState]
        RECALLING: ClassVar[ChampionState]
        SUBMITTING_GEMS: ClassVar[ChampionState]
        CAN_MOVE: ClassVar[ChampionState]
        CAN_ATTACK: ClassVar[ChampionState]
        COMPLETED_TURN: ClassVar[ChampionState]
        RECALL_STARTED: ClassVar[ChampionState]
        SUBMIT_GEMS_STARTED: ClassVar[ChampionState]
        RECALL_CANCELLED: ClassVar[ChampionState]
        SUBMIT_GEMS_CANCELLED: ClassVar[ChampionState]
        TELEPORTING: ClassVar[ChampionState]
        CAN_CAST: ClassVar[ChampionState]

    class AbilityTargetMask(_bc.FlagSet):
        """Valid ability target bits."""

        CAMPS: ClassVar[AbilityTargetMask]
        ENEMIES: ClassVar[AbilityTargetMask]
        ALLIES: ClassVar[AbilityTargetMask]
        SELF: ClassVar[AbilityTargetMask]
        GROUND: ClassVar[AbilityTargetMask]


    class BuffRestrictions(_bc.FlagSet):
        """Actions blocked by buffs."""

        MOVE: ClassVar[BuffRestrictions]
        ATTACK: ClassVar[BuffRestrictions]
        CAST: ClassVar[BuffRestrictions]
        SHOP: ClassVar[BuffRestrictions]
        CONSUMABLE: ClassVar[BuffRestrictions]
        WARD: ClassVar[BuffRestrictions]


    class ItemPassive(IntEnum):
        """Item passive group."""

        @staticmethod
        def to_string(value: int) -> str: ...

        MONSTER = 1
        DURABILITY = 2
        TREASURE = 3
        JUNGLE = 4
        HOME = 5

else:

    class Team:
        __slots__ = ()

        ALLY = 0
        ENEMY = 1

        @staticmethod
        def opponent(team: int) -> int:
            return 1 - team

    class Symmetry:
        __slots__ = ()

        HORIZONTAL = 0
        VERTICAL = 1
        ROTATIONAL180 = 2

    class CampType:
        __slots__ = ()

        WOLF = 0
        ELK = 1
        GEM_CLUSTER = 2
        GOLD_CLUSTER = 3
        MONOLITH = 4

    class PassiveType:
        __slots__ = ()

        NONE = 0
        BONUS_GOLD = 1
        REMOTE_SHOP = 2
        GEM_FOCUS = 3
        SPLASH_ATTACK = 4
        HEALTH_STRENGTH = 5
        RESET_COOLDOWNS_ON_KILL = 6
        RETALIATION = 7
        STUN_IMMUNITY = 8

    class ChampionType:
        __slots__ = ()

        RYTH = 0
        VESPER_VALE = 1
        VENDETTA = 2
        CASSI = 3
        ISSAC = 4
        STELLAR = 5
        PRISM = 6
        MARCELLA = 7
        WILLOW = 8
        REN = 9
        VARG = 10
        TAVI = 11

    class ChampionDraft:
        __slots__ = ()

        RYTH = 0
        VESPER_VALE = 1
        VENDETTA = 2
        CASSI_AND_ISSAC = 3
        STELLAR = 4
        PRISM = 5
        MARCELLA = 6
        WILLOW = 7
        REN = 8
        VARG = 9
        TAVI = 10

    class AbilityKind:
        __slots__ = ()

        ATTACK_RESET = 1
        DAMAGE = 2
        AOE_DAMAGE = 3
        CC = 4
        HEAL = 5
        DASH = 6
        SHIELD = 7

    class _EntityKind:
        __slots__ = ()

        CHAMPION = 1
        CAMP = 2
        ALTAR_LOCK = 6

    class BuffTypes:
        __slots__ = ()

        SHIELD = 0
        STUN = 1
        ENHANCED_ATTACK = 2
        COUNT = 3

    class ChampionState(_bc.FlagSet):
        __slots__ = ()

        def is_dead(self) -> bool:
            return not self & 1

        ALIVE = 1
        RECALLING = 2
        SUBMITTING_GEMS = 4
        CAN_MOVE = 8
        CAN_ATTACK = 16
        COMPLETED_TURN = 32
        RECALL_STARTED = 64
        SUBMIT_GEMS_STARTED = 128
        RECALL_CANCELLED = 256
        SUBMIT_GEMS_CANCELLED = 512
        CAN_CAST = 2048
        TELEPORTING = 4096

    class AbilityTargetMask(_bc.FlagSet):
        __slots__ = ()

        CAMPS = 1
        ENEMIES = 2
        ALLIES = 4
        SELF = 8
        GROUND = 16


    class BuffRestrictions(_bc.FlagSet):
        __slots__ = ()

        MOVE = 1
        ATTACK = 2
        CAST = 4
        SHOP = 8
        CONSUMABLE = 16
        WARD = 32

    for _flags in (ChampionState, AbilityTargetMask, BuffRestrictions):
        _known = 0
        for _name, _value in list(vars(_flags).items()):
            if _name.isupper():
                setattr(_flags, _name, _flags(_value))
                _known |= _value
        _flags._known = _known
    del _flags, _known, _name, _value

    class ItemPassive:
        __slots__ = ()

        MONSTER = 1
        DURABILITY = 2
        TREASURE = 3
        JUNGLE = 4
        HOME = 5

if TYPE_CHECKING:

    class ErrorCode(IntEnum):
        """Engine error codes carried by Error.code."""

        @staticmethod
        def to_string(value: int) -> str: ...

        SUCCESS = 0
        INVALID_HANDLE = 1
        OUT_OF_MEMORY = 2
        FILE_NOT_FOUND = 3
        NULL_POINTER = 4
        INVALID_ARGUMENT = 5
        MAP_PARSE_ERROR = 6
        INVALID_MAP_FILE = 7
        INVALID_MAP_VERSION = 8
        REPLAY_PARSE_ERROR = 9
        INVALID_REPLAY_FILE = 10
        INVALID_REPLAY_VERSION = 11
        INVALID_DRAFT = 12
        DRAFT_NOT_DRAFTING = 13
        DRAFT_DUPLICATE_CHAMPION = 14
        DRAFT_INVALID_ROUND = 15
        DRAFT_UNAVAILABLE = 16
        ALREADY_INITIALIZED = 17
        INVALID_ACTION = 18
        INVALID_ACTION_MEMORY_BAD_RANGE = 19
        INVALID_ACTION_MEMORY_DRAFT_FROZEN = 20
        INVALID_ACTION_MOVE_CANT_MOVE = 21
        INVALID_ACTION_MOVE_BAD_DESTINATION = 22
        INVALID_ACTION_SPELL_CAST_INVALID_SOURCE = 23
        INVALID_ACTION_SPELL_CAST_INVALID_SLOT = 24
        INVALID_ACTION_SPELL_CAST_NO_ABILITY = 25
        INVALID_ACTION_SPELL_CAST_CANT_CAST = 26
        INVALID_ACTION_SPELL_CAST_ALREADY_CAST = 27
        INVALID_ACTION_SPELL_CAST_ON_COOLDOWN = 28
        INVALID_ACTION_SPELL_CAST_NO_CHARGES = 29
        INVALID_ACTION_SPELL_CAST_BAD_TARGET = 30
        INVALID_ACTION_SPELL_CAST_OUT_OF_RANGE = 31
        INVALID_ACTION_SPELL_CAST_BAD_DESTINATION = 32
        INVALID_ACTION_CANT_ACT = 33
        INVALID_ACTION_IS_RECALLING = 34
        INVALID_ACTION_IS_SUBMITTING_GEMS = 35
        INVALID_ACTION_MOVE_ALREADY_MOVED = 36
        INVALID_ACTION_MOVE_OUT_OF_RANGE = 37
        INVALID_ACTION_MOVE_UNREACHABLE = 38
        INVALID_ACTION_ATTACK_INVALID_SOURCE = 39
        INVALID_ACTION_ATTACK_CANT_ATTACK = 40
        INVALID_ACTION_ATTACK_ALREADY_ATTACKED = 41
        INVALID_ACTION_ATTACK_BAD_TARGET = 42
        INVALID_ACTION_ATTACK_OUT_OF_RANGE = 43
        INVALID_ACTION_RECALL_CANT_CHANNEL = 44
        INVALID_ACTION_SUBMIT_GEMS_CANT_CHANNEL = 45
        INVALID_ACTION_SUBMIT_GEMS_NO_GEMS = 46
        INVALID_ACTION_SUBMIT_GEMS_BAD_ALTAR = 47
        INVALID_ACTION_SUBMIT_GEMS_OUT_OF_RANGE = 48
        INVALID_ACTION_BUY_ITEM_NOT_ON_SPAWN = 49
        INVALID_ACTION_BUY_ITEM_INVALID_ITEM = 50
        INVALID_ACTION_BUY_ITEM_INVALID_COUNT = 51
        INVALID_ACTION_BUY_ITEM_NOT_ENOUGH_GOLD = 52
        INVALID_ACTION_BUY_ITEM_INVENTORY_FULL = 53
        INVALID_ACTION_BUY_ITEM_STATS_OVERFLOW = 54
        INVALID_ACTION_BUY_CONSUMABLE_NOT_ON_SPAWN = 55
        INVALID_ACTION_BUY_CONSUMABLE_INVALID_TYPE = 56
        INVALID_ACTION_BUY_CONSUMABLE_INVALID_COUNT = 57
        INVALID_ACTION_BUY_CONSUMABLE_INVENTORY_FULL = 58
        INVALID_ACTION_BUY_CONSUMABLE_NOT_ENOUGH_GOLD = 59
        INVALID_ACTION_USE_CONSUMABLE_INVALID_TYPE = 60
        INVALID_ACTION_USE_CONSUMABLE_NOT_OWNED = 61
        INVALID_ACTION_PLACE_WARD_NO_CHARGES = 62
        INVALID_ACTION_PLACE_WARD_CANT_PLACE = 63
        INVALID_ACTION_PLACE_WARD_BAD_DESTINATION = 64
        INVALID_ACTION_PLACE_WARD_OUT_OF_RANGE = 65
        MAP_SET_TEAM_ASPAWN_AREA_BAD_POSITION = 66
        MAP_ADD_BUSH_BAD_POSITION = 67
        MAP_REMOVE_BUSH_BAD_POSITION = 68
        MAP_ADD_WALL_BAD_POSITION = 69
        MAP_REMOVE_WALL_BAD_POSITION = 70
        MAP_ADD_HOLE_BAD_POSITION = 71
        MAP_REMOVE_HOLE_BAD_POSITION = 72
        MAP_ADD_ALTAR_BAD_POSITION = 73
        MAP_REMOVE_ALTAR_BAD_POSITION = 74
        MAP_GET_BUSH_BAD_POSITION = 75
        MAP_GET_WALL_BAD_POSITION = 76
        MAP_GET_HOLE_BAD_POSITION = 77
        MAP_GET_ALTAR_BAD_POSITION = 78
        MAP_GET_CAMP_HANDLE_AT_BAD_POSITION = 79
        MAP_ADD_CAMP_BAD_POSITION = 80
        MAP_SET_CAMP_BAD_POSITION = 81
        INVALID_ACTION_CHAMPION_TELEPORT_BAD_DESTINATION = 82
        INVALID_ACTION_RESPAWN_GROUP_BLOCKED_SPAWN = 83
        INVALID_ACTION_CAMP_TELEPORT_BAD_DESTINATION = 84
        TIMELINE_BAD_SEEK_POSITION = 85
        TIMELINE_BAD_SEEK_SUBSTEP = 86
        API_NO_MAP = 87
        MANAGED_RESOURCES_BAD_POSITION = 88
        MANAGED_RESOURCES_BAD_RADIUS = 89
        MANAGED_RESOURCES_ALIASING = 90
        MANAGED_RESOURCES_METADATA_MISMATCH = 91
        MANAGED_RESOURCES_OPAQUE_ORIGIN = 92
        API_INVALID_TEAM = 93
        API_INVALID_PERSPECTIVE = 94
        API_NOT_INITIALIZED = 95
        API_BAD_POSITION = 96
        API_NOT_IN_VISION = 97
        API_NOT_FOUND = 98
        API_INVALID_CAMP_GROUP = 99
        API_INVALID_CAMP_INFO = 100
        API_INVALID_CAMP = 101
        API_INVALID_CHAMPION = 102
        API_INVALID_WARD = 103
        API_INVALID_ABILITY = 104
        API_NOT_TEAMMATE = 105
        API_CAMP_GROUP_NOT_WAITING_TO_RESPAWN = 106
        API_CHAMPION_ALIVE = 107
        API_INVALID_CHAMPION_TYPE = 108
        API_INVALID_CAMP_TYPE = 109
        API_VALUE_OUT_OF_RANGE = 110
        API_BUFFER_TOO_SMALL = 111
        API_NOT_OWNER = 112
        INVALID_ACTION_RECALL_ALREADY_USED = 113
        INVALID_ACTION_SUBMIT_GEMS_ALREADY_USED = 114
        INVALID_ACTION_CANCEL_CHANNEL_NOT_CHANNELING = 115
        INVALID_ACTION_CANCEL_CHANNEL_ALREADY_USED = 116
        INVALID_ACTION_PLACE_WARD_ALLY_WARD_PRESENT = 117
        INVALID_ACTION_ALTAR_LOCK_BAD_ALTAR = 118
        INVALID_ACTION_ALTAR_LOCK_OUT_OF_RANGE = 119
        INVALID_ACTION_ALTAR_LOCK_ALREADY_LOCKED = 120
        INVALID_ACTION_IS_TELEPORTING = 123
        INVALID_ACTION_CANCEL_CHANNEL_NOT_CANCELLABLE = 124
        INVALID_ACTION_SUBMIT_GEMS_ALTAR_LOCKED = 125
        INVALID_ACTION_SPELL_CAST_ALREADY_MOVED = 126
        INVALID_ACTION_MOVE_EXCLUSIVE_SPELL_CAST = 127
        INVALID_ACTION_SELL_NOT_TAVI = 128
        INVALID_ACTION_SELL_INVALID_ITEM = 129
        INVALID_ACTION_SELL_NOT_OWNED = 130
        INVALID_ACTION_SELL_ALREADY_SOLD = 131
        INVALID_ACTION_SELL_GOLD_OVERFLOW = 132
        INVALID_ACTION_SELL_NOT_ON_SPAWN = 133

else:

    class ErrorCode:
        __slots__ = ()

        SUCCESS = 0
        INVALID_HANDLE = 1
        OUT_OF_MEMORY = 2
        FILE_NOT_FOUND = 3
        NULL_POINTER = 4
        INVALID_ARGUMENT = 5
        MAP_PARSE_ERROR = 6
        INVALID_MAP_FILE = 7
        INVALID_MAP_VERSION = 8
        REPLAY_PARSE_ERROR = 9
        INVALID_REPLAY_FILE = 10
        INVALID_REPLAY_VERSION = 11
        INVALID_DRAFT = 12
        DRAFT_NOT_DRAFTING = 13
        DRAFT_DUPLICATE_CHAMPION = 14
        DRAFT_INVALID_ROUND = 15
        DRAFT_UNAVAILABLE = 16
        ALREADY_INITIALIZED = 17
        INVALID_ACTION = 18
        INVALID_ACTION_MEMORY_BAD_RANGE = 19
        INVALID_ACTION_MEMORY_DRAFT_FROZEN = 20
        INVALID_ACTION_MOVE_CANT_MOVE = 21
        INVALID_ACTION_MOVE_BAD_DESTINATION = 22
        INVALID_ACTION_SPELL_CAST_INVALID_SOURCE = 23
        INVALID_ACTION_SPELL_CAST_INVALID_SLOT = 24
        INVALID_ACTION_SPELL_CAST_NO_ABILITY = 25
        INVALID_ACTION_SPELL_CAST_CANT_CAST = 26
        INVALID_ACTION_SPELL_CAST_ALREADY_CAST = 27
        INVALID_ACTION_SPELL_CAST_ON_COOLDOWN = 28
        INVALID_ACTION_SPELL_CAST_NO_CHARGES = 29
        INVALID_ACTION_SPELL_CAST_BAD_TARGET = 30
        INVALID_ACTION_SPELL_CAST_OUT_OF_RANGE = 31
        INVALID_ACTION_SPELL_CAST_BAD_DESTINATION = 32
        INVALID_ACTION_CANT_ACT = 33
        INVALID_ACTION_IS_RECALLING = 34
        INVALID_ACTION_IS_SUBMITTING_GEMS = 35
        INVALID_ACTION_MOVE_ALREADY_MOVED = 36
        INVALID_ACTION_MOVE_OUT_OF_RANGE = 37
        INVALID_ACTION_MOVE_UNREACHABLE = 38
        INVALID_ACTION_ATTACK_INVALID_SOURCE = 39
        INVALID_ACTION_ATTACK_CANT_ATTACK = 40
        INVALID_ACTION_ATTACK_ALREADY_ATTACKED = 41
        INVALID_ACTION_ATTACK_BAD_TARGET = 42
        INVALID_ACTION_ATTACK_OUT_OF_RANGE = 43
        INVALID_ACTION_RECALL_CANT_CHANNEL = 44
        INVALID_ACTION_SUBMIT_GEMS_CANT_CHANNEL = 45
        INVALID_ACTION_SUBMIT_GEMS_NO_GEMS = 46
        INVALID_ACTION_SUBMIT_GEMS_BAD_ALTAR = 47
        INVALID_ACTION_SUBMIT_GEMS_OUT_OF_RANGE = 48
        INVALID_ACTION_BUY_ITEM_NOT_ON_SPAWN = 49
        INVALID_ACTION_BUY_ITEM_INVALID_ITEM = 50
        INVALID_ACTION_BUY_ITEM_INVALID_COUNT = 51
        INVALID_ACTION_BUY_ITEM_NOT_ENOUGH_GOLD = 52
        INVALID_ACTION_BUY_ITEM_INVENTORY_FULL = 53
        INVALID_ACTION_BUY_ITEM_STATS_OVERFLOW = 54
        INVALID_ACTION_BUY_CONSUMABLE_NOT_ON_SPAWN = 55
        INVALID_ACTION_BUY_CONSUMABLE_INVALID_TYPE = 56
        INVALID_ACTION_BUY_CONSUMABLE_INVALID_COUNT = 57
        INVALID_ACTION_BUY_CONSUMABLE_INVENTORY_FULL = 58
        INVALID_ACTION_BUY_CONSUMABLE_NOT_ENOUGH_GOLD = 59
        INVALID_ACTION_USE_CONSUMABLE_INVALID_TYPE = 60
        INVALID_ACTION_USE_CONSUMABLE_NOT_OWNED = 61
        INVALID_ACTION_PLACE_WARD_NO_CHARGES = 62
        INVALID_ACTION_PLACE_WARD_CANT_PLACE = 63
        INVALID_ACTION_PLACE_WARD_BAD_DESTINATION = 64
        INVALID_ACTION_PLACE_WARD_OUT_OF_RANGE = 65
        MAP_SET_TEAM_ASPAWN_AREA_BAD_POSITION = 66
        MAP_ADD_BUSH_BAD_POSITION = 67
        MAP_REMOVE_BUSH_BAD_POSITION = 68
        MAP_ADD_WALL_BAD_POSITION = 69
        MAP_REMOVE_WALL_BAD_POSITION = 70
        MAP_ADD_HOLE_BAD_POSITION = 71
        MAP_REMOVE_HOLE_BAD_POSITION = 72
        MAP_ADD_ALTAR_BAD_POSITION = 73
        MAP_REMOVE_ALTAR_BAD_POSITION = 74
        MAP_GET_BUSH_BAD_POSITION = 75
        MAP_GET_WALL_BAD_POSITION = 76
        MAP_GET_HOLE_BAD_POSITION = 77
        MAP_GET_ALTAR_BAD_POSITION = 78
        MAP_GET_CAMP_HANDLE_AT_BAD_POSITION = 79
        MAP_ADD_CAMP_BAD_POSITION = 80
        MAP_SET_CAMP_BAD_POSITION = 81
        INVALID_ACTION_CHAMPION_TELEPORT_BAD_DESTINATION = 82
        INVALID_ACTION_RESPAWN_GROUP_BLOCKED_SPAWN = 83
        INVALID_ACTION_CAMP_TELEPORT_BAD_DESTINATION = 84
        TIMELINE_BAD_SEEK_POSITION = 85
        TIMELINE_BAD_SEEK_SUBSTEP = 86
        API_NO_MAP = 87
        MANAGED_RESOURCES_BAD_POSITION = 88
        MANAGED_RESOURCES_BAD_RADIUS = 89
        MANAGED_RESOURCES_ALIASING = 90
        MANAGED_RESOURCES_METADATA_MISMATCH = 91
        MANAGED_RESOURCES_OPAQUE_ORIGIN = 92
        API_INVALID_TEAM = 93
        API_INVALID_PERSPECTIVE = 94
        API_NOT_INITIALIZED = 95
        API_BAD_POSITION = 96
        API_NOT_IN_VISION = 97
        API_NOT_FOUND = 98
        API_INVALID_CAMP_GROUP = 99
        API_INVALID_CAMP_INFO = 100
        API_INVALID_CAMP = 101
        API_INVALID_CHAMPION = 102
        API_INVALID_WARD = 103
        API_INVALID_ABILITY = 104
        API_NOT_TEAMMATE = 105
        API_CAMP_GROUP_NOT_WAITING_TO_RESPAWN = 106
        API_CHAMPION_ALIVE = 107
        API_INVALID_CHAMPION_TYPE = 108
        API_INVALID_CAMP_TYPE = 109
        API_VALUE_OUT_OF_RANGE = 110
        API_BUFFER_TOO_SMALL = 111
        API_NOT_OWNER = 112
        INVALID_ACTION_RECALL_ALREADY_USED = 113
        INVALID_ACTION_SUBMIT_GEMS_ALREADY_USED = 114
        INVALID_ACTION_CANCEL_CHANNEL_NOT_CHANNELING = 115
        INVALID_ACTION_CANCEL_CHANNEL_ALREADY_USED = 116
        INVALID_ACTION_PLACE_WARD_ALLY_WARD_PRESENT = 117
        INVALID_ACTION_ALTAR_LOCK_BAD_ALTAR = 118
        INVALID_ACTION_ALTAR_LOCK_OUT_OF_RANGE = 119
        INVALID_ACTION_ALTAR_LOCK_ALREADY_LOCKED = 120
        INVALID_ACTION_IS_TELEPORTING = 123
        INVALID_ACTION_CANCEL_CHANNEL_NOT_CANCELLABLE = 124
        INVALID_ACTION_SUBMIT_GEMS_ALTAR_LOCKED = 125
        INVALID_ACTION_SPELL_CAST_ALREADY_MOVED = 126
        INVALID_ACTION_MOVE_EXCLUSIVE_SPELL_CAST = 127
        INVALID_ACTION_SELL_NOT_TAVI = 128
        INVALID_ACTION_SELL_INVALID_ITEM = 129
        INVALID_ACTION_SELL_NOT_OWNED = 130
        INVALID_ACTION_SELL_ALREADY_SOLD = 131
        INVALID_ACTION_SELL_GOLD_OVERFLOW = 132
        INVALID_ACTION_SELL_NOT_ON_SPAWN = 133



if not TYPE_CHECKING:

    def _add_to_string(cls: type) -> None:
        names = {v: k for k, v in vars(cls).items() if k.isupper() and type(v) is int}
        prefix = cls.__name__
        cls.to_string = staticmethod(lambda value: f"{prefix}.{names.get(value, value)}")

    for _cls in (Team, Symmetry, CampType, PassiveType, ChampionType, ChampionDraft, AbilityKind, BuffTypes, ItemPassive, ErrorCode):
        _add_to_string(_cls)
    del _cls


class Error(Exception):
    """Engine error raised by API calls; code is an ErrorCode."""

    code: int
    message: str

    def __init__(self, code: int, message: str = "") -> None:
        super().__init__(code, message)
        self.code = code
        self.message = message

    def __str__(self) -> str:
        return self.message or f"engine error {self.code}"


_bc._register_error(Error)

# ItemType members (engine item IDs 0..41)
_ITEM_NAMES = (
    "MONSTER_SHARD_OF_STRENGTH",
    "MONSTER_EMBLEM_OF_STRENGTH",
    "MONSTER_SHARD_OF_FOCUS",
    "MONSTER_EMBLEM_OF_FOCUS",
    "MONSTER_SHARD_OF_HEALTH",
    "MONSTER_EMBLEM_OF_HEALTH",
    "POWER_SHARD_OF_STRENGTH",
    "POWER_EMBLEM_OF_STRENGTH",
    "POWER_SHARD_OF_FOCUS",
    "POWER_EMBLEM_OF_FOCUS",
    "POWER_SHARD_OF_HEALTH",
    "POWER_EMBLEM_OF_HEALTH",
    "DURABILITY_SHARD_OF_STRENGTH",
    "DURABILITY_EMBLEM_OF_STRENGTH",
    "DURABILITY_SHARD_OF_FOCUS",
    "DURABILITY_EMBLEM_OF_FOCUS",
    "DURABILITY_SHARD_OF_HEALTH",
    "DURABILITY_EMBLEM_OF_HEALTH",
    "TREASURE_SHARD_OF_STRENGTH",
    "TREASURE_EMBLEM_OF_STRENGTH",
    "TREASURE_SHARD_OF_FOCUS",
    "TREASURE_EMBLEM_OF_FOCUS",
    "TREASURE_SHARD_OF_HEALTH",
    "TREASURE_EMBLEM_OF_HEALTH",
    "JUNGLE_SHARD_OF_STRENGTH",
    "JUNGLE_EMBLEM_OF_STRENGTH",
    "JUNGLE_SHARD_OF_FOCUS",
    "JUNGLE_EMBLEM_OF_FOCUS",
    "JUNGLE_SHARD_OF_HEALTH",
    "JUNGLE_EMBLEM_OF_HEALTH",
    "HOME_SHARD_OF_STRENGTH",
    "HOME_EMBLEM_OF_STRENGTH",
    "HOME_SHARD_OF_FOCUS",
    "HOME_EMBLEM_OF_FOCUS",
    "HOME_SHARD_OF_HEALTH",
    "HOME_EMBLEM_OF_HEALTH",
    "BRUISER_SHARD_OF_STRENGTH",
    "BRUISER_EMBLEM_OF_STRENGTH",
    "BRUISER_SHARD_OF_FOCUS",
    "BRUISER_EMBLEM_OF_FOCUS",
    "BRUISER_SHARD_OF_HEALTH",
    "BRUISER_EMBLEM_OF_HEALTH",
)



class ItemType(int):
    """Item type: an engine item ID with getters that forward to info()."""

    __slots__ = ()
    _members: ClassVar[tuple[ItemType, ...]]
    MONSTER_SHARD_OF_STRENGTH: ClassVar[ItemType]
    MONSTER_EMBLEM_OF_STRENGTH: ClassVar[ItemType]
    MONSTER_SHARD_OF_FOCUS: ClassVar[ItemType]
    MONSTER_EMBLEM_OF_FOCUS: ClassVar[ItemType]
    MONSTER_SHARD_OF_HEALTH: ClassVar[ItemType]
    MONSTER_EMBLEM_OF_HEALTH: ClassVar[ItemType]
    POWER_SHARD_OF_STRENGTH: ClassVar[ItemType]
    POWER_EMBLEM_OF_STRENGTH: ClassVar[ItemType]
    POWER_SHARD_OF_FOCUS: ClassVar[ItemType]
    POWER_EMBLEM_OF_FOCUS: ClassVar[ItemType]
    POWER_SHARD_OF_HEALTH: ClassVar[ItemType]
    POWER_EMBLEM_OF_HEALTH: ClassVar[ItemType]
    DURABILITY_SHARD_OF_STRENGTH: ClassVar[ItemType]
    DURABILITY_EMBLEM_OF_STRENGTH: ClassVar[ItemType]
    DURABILITY_SHARD_OF_FOCUS: ClassVar[ItemType]
    DURABILITY_EMBLEM_OF_FOCUS: ClassVar[ItemType]
    DURABILITY_SHARD_OF_HEALTH: ClassVar[ItemType]
    DURABILITY_EMBLEM_OF_HEALTH: ClassVar[ItemType]
    TREASURE_SHARD_OF_STRENGTH: ClassVar[ItemType]
    TREASURE_EMBLEM_OF_STRENGTH: ClassVar[ItemType]
    TREASURE_SHARD_OF_FOCUS: ClassVar[ItemType]
    TREASURE_EMBLEM_OF_FOCUS: ClassVar[ItemType]
    TREASURE_SHARD_OF_HEALTH: ClassVar[ItemType]
    TREASURE_EMBLEM_OF_HEALTH: ClassVar[ItemType]
    JUNGLE_SHARD_OF_STRENGTH: ClassVar[ItemType]
    JUNGLE_EMBLEM_OF_STRENGTH: ClassVar[ItemType]
    JUNGLE_SHARD_OF_FOCUS: ClassVar[ItemType]
    JUNGLE_EMBLEM_OF_FOCUS: ClassVar[ItemType]
    JUNGLE_SHARD_OF_HEALTH: ClassVar[ItemType]
    JUNGLE_EMBLEM_OF_HEALTH: ClassVar[ItemType]
    HOME_SHARD_OF_STRENGTH: ClassVar[ItemType]
    HOME_EMBLEM_OF_STRENGTH: ClassVar[ItemType]
    HOME_SHARD_OF_FOCUS: ClassVar[ItemType]
    HOME_EMBLEM_OF_FOCUS: ClassVar[ItemType]
    HOME_SHARD_OF_HEALTH: ClassVar[ItemType]
    HOME_EMBLEM_OF_HEALTH: ClassVar[ItemType]
    BRUISER_SHARD_OF_STRENGTH: ClassVar[ItemType]
    BRUISER_EMBLEM_OF_STRENGTH: ClassVar[ItemType]
    BRUISER_SHARD_OF_FOCUS: ClassVar[ItemType]
    BRUISER_EMBLEM_OF_FOCUS: ClassVar[ItemType]
    BRUISER_SHARD_OF_HEALTH: ClassVar[ItemType]
    BRUISER_EMBLEM_OF_HEALTH: ClassVar[ItemType]
    COUNT: ClassVar[int] = len(_ITEM_NAMES)

    def __new__(cls, value: int) -> ItemType:
        if not 0 <= value < len(_ITEM_NAMES):
            raise ValueError(f"invalid item type {value}")
        return cls._members[value]

    def __repr__(self) -> str:
        return f"ItemType.{_ITEM_NAMES[self]}"

    @property
    def name(self) -> str:
        """Return the item's display name."""
        return _ITEM_NAMES[self]

    def info(self) -> ItemInfo:
        """Return the item's static design data."""
        info = _item_infos[self]
        if info is None:
            info = _item_infos[self] = _bc.item_info(self)
        return info

    def cost(self) -> int:
        """Return the gold cost to buy the item."""
        return self.info().cost

    def combine_count(self) -> int:
        """Return how many copies combine into the result item."""
        return self.info().combine_count

    def result(self) -> Optional[ItemType]:
        """Return the item these copies combine into, or None if it does not combine."""
        return self.info().result

    def health_points(self) -> int:
        """Return the maximum health the item grants."""
        return self.info().health_points

    def strength_points(self) -> int:
        """Return the strength the item grants."""
        return self.info().strength_points

    def focus_points(self) -> int:
        """Return the focus the item grants."""
        return self.info().focus_points

    def passive(self) -> Optional[ItemPassive]:
        """Return the passive effect the item grants."""
        return self.info().passive

    def passive_magnitude(self) -> float:
        """Return the strength of the item's passive effect."""
        return self.info().passive_magnitude

    def is_purchasable(self) -> bool:
        """Return whether the item can be bought directly from the shop."""
        return self.info().purchasable


_item_infos: list[Optional[ItemInfo]] = [None] * len(_ITEM_NAMES)
ItemType._members = tuple(int.__new__(ItemType, i) for i in range(len(_ITEM_NAMES)))
for _i, _name in enumerate(_ITEM_NAMES):
    setattr(ItemType, _name, ItemType._members[_i])
_bc._register_item_type(ItemType)

_CONSUMABLE_NAMES = ("TP_CRYSTAL", "ALTAR_LOCK")
_RAW_CONSUMABLE_IDS = (0, 2)


class ConsumableType(int):
    """Consumable type with getters that forward to info()."""

    __slots__ = ()
    _members: ClassVar[tuple[ConsumableType, ...]]
    TP_CRYSTAL: ClassVar[ConsumableType]
    ALTAR_LOCK: ClassVar[ConsumableType]
    COUNT: ClassVar[int] = len(_CONSUMABLE_NAMES)

    def __new__(cls, value: int) -> ConsumableType:
        if not 0 <= value < len(_CONSUMABLE_NAMES):
            raise ValueError(f"invalid consumable type {value}")
        return cls._members[value]

    def __repr__(self) -> str:
        return f"ConsumableType.{_CONSUMABLE_NAMES[self]}"

    @property
    def name(self) -> str:
        """Return the consumable's display name."""
        return _CONSUMABLE_NAMES[self]

    def info(self) -> ConsumableInfo:
        """Return the consumable's static design data."""
        info = _consumable_infos[self]
        if info is None:
            info = _consumable_infos[self] = _bc.consumable_info(_RAW_CONSUMABLE_IDS[self])
        return info

    def cost(self) -> int:
        """Return the gold cost to buy one."""
        return self.info().cost

    def carry_limit(self) -> int:
        """Return how many a champion can carry at once."""
        return self.info().carry_limit


_consumable_infos: list[Optional[ConsumableInfo]] = [None] * len(_CONSUMABLE_NAMES)
ConsumableType._members = tuple(int.__new__(ConsumableType, i) for i in range(len(_CONSUMABLE_NAMES)))
ConsumableType.TP_CRYSTAL, ConsumableType.ALTAR_LOCK = ConsumableType._members


Buff = Union[BuffShield, BuffStun, BuffEnhancedAttack]


class BuffCollection:
    """Snapshot of a unit's active buffs; iterates present buffs in BuffTypes order."""

    __slots__ = ("_buffs", "_present")

    def __init__(self, buffs: list[Optional[Buff]], present: tuple[Buff, ...]) -> None:
        self._buffs = buffs
        self._present = present

    def get(self, kind: Union[int, type[Buff]]) -> Buff:
        """Return the captured buff of a kind; raises KeyError if it was absent.

        Args:
            kind: Buff kind to look up.
        """
        buff = self._buffs[kind if isinstance(kind, int) else kind.kind]
        if buff is None:
            raise IndexError("unit does not have this buff")
        return buff

    def has(self, kind: Union[int, type[Buff]]) -> bool:
        """Return whether a buff of a kind was active when the collection was captured.

        Args:
            kind: Buff kind to test for.
        """
        return self._buffs[kind if isinstance(kind, int) else kind.kind] is not None

    def has_shield(self) -> bool:
        """Check whether the captured collection has a shield buff."""
        return self.has(BuffTypes.SHIELD)

    def has_stun(self) -> bool:
        """Check whether the captured collection has a stun buff."""
        return self.has(BuffTypes.STUN)

    def has_enhanced_attack(self) -> bool:
        """Check whether the captured collection has an enhanced-attack buff."""
        return self.has(BuffTypes.ENHANCED_ATTACK)

    def is_empty(self) -> bool:
        """Return whether no buffs were active when the collection was captured."""
        return not self._present

    def __len__(self) -> int:
        return len(self._present)

    def __iter__(self) -> Iterator[Buff]:
        return iter(self._present)


class Unit:
    """Common interface of champions and camps."""

    __slots__ = ("_entity", "_design")
    _entity: _bc.Entity
    _design: bool

    def __init__(self) -> None:
        raise TypeError(f"{type(self).__name__} cannot be constructed directly")

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Unit):
            return NotImplemented
        return self._entity == other._entity and self._design == other._design

    def __hash__(self) -> int:
        return hash((self._entity, self._design))

    def buffs(self) -> BuffCollection:
        """Capture the unit's current buffs; raises Error if the unit is not visible."""
        if self._design:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no live buffs")
        return BuffCollection(*_bc.buffs(self._entity.kind, self._entity.handle))


    def health(self) -> float:
        """Return current health; raises Error if the unit is not visible."""
        raise NotImplementedError

    def max_health(self) -> float:
        """Return current maximum health; raises Error if the unit is not visible."""
        raise NotImplementedError

    def strength(self) -> float:
        """Return current strength; raises Error if the unit is not visible."""
        raise NotImplementedError

    def focus(self) -> float:
        """Return current focus; raises Error if the unit is not visible."""
        raise NotImplementedError

    def attack_range(self) -> int:
        """Return the attack range in Chebyshev tiles; melee units have range 1."""
        raise NotImplementedError

    def position(self) -> Vec2:
        """Return the current tile; raises Error if the unit is not visible."""
        raise NotImplementedError

    def is_visible(self) -> bool:
        """Return whether the unit is currently visible."""
        raise NotImplementedError

    def attack_tiles(self) -> Bitboard:
        """Return the tiles within attack range of the unit's current position."""
        result = Bitboard()
        self.attack_tiles_into(result)
        return result

    def attack_tiles_into(self, out: Bitboard) -> None:
        """Like attack_tiles, but writes into out.

        Args:
            out: Destination Bitboard; its previous contents are overwritten.
        """
        out.clear()
        out.draw_chebyshev(self.position(), self.attack_range())


class Camp(Unit):
    """A neutral camp; int(camp) is its engine handle."""

    __slots__ = ("_handle", "_group", "_type", "_spawn_position")
    _handle: _CampHandle
    _group: _CampGroupHandle
    _type: int
    _spawn_position: Vec2

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Camp):
            return NotImplemented
        return self._handle == other._handle

    def __hash__(self) -> int:
        return hash(self._handle)

    def __repr__(self) -> str:
        return f"Camp({self._handle}, {CampType.to_string(self._type)}, spawn={self._spawn_position!r})"

    @classmethod
    def _new(cls, handle: _CampHandle, group: _CampGroupHandle, kind: int, spawn_position: Vec2) -> Camp:
        self = cls.__new__(cls)
        self._entity = _bc.Entity(_EntityKind.CAMP, handle)
        self._design = False
        self._handle = handle
        self._group = group
        self._type = kind
        self._spawn_position = spawn_position
        return self

    def __index__(self) -> int:
        return self._handle

    def kind(self) -> CampType:
        """Return the camp's type."""
        return self._type  # type: ignore[return-value]

    def is_wolf(self) -> bool:
        """Check whether this camp is a wolf."""
        return self.kind() == CampType.WOLF

    def is_elk(self) -> bool:
        """Check whether this camp is an elk."""
        return self.kind() == CampType.ELK

    def is_gem_cluster(self) -> bool:
        """Check whether this camp is a gem cluster."""
        return self.kind() == CampType.GEM_CLUSTER

    def is_gold_cluster(self) -> bool:
        """Check whether this camp is a gold cluster."""
        return self.kind() == CampType.GOLD_CLUSTER

    def is_monolith(self) -> bool:
        """Check whether this camp is a monolith."""
        return self.kind() == CampType.MONOLITH

    def spawn_position(self) -> Vec2:
        """Return the camp's spawn tile, not its live position."""
        return self._spawn_position

    def camp_group(self) -> game_map.CampGroup:
        """Return the group this camp belongs to."""
        return game_map._camp_group(self._group)

    def health(self) -> float:
        """Return current health; raises Error if the unit is not visible."""
        return _bc.camp_get(self._handle, 1)

    def max_health(self) -> float:
        """Return current maximum health; raises Error if the unit is not visible."""
        return _bc.camp_get(self._handle, 2)

    def position(self) -> Vec2:
        """Return the current tile; raises Error if the unit is not visible."""
        return _bc.camp_get(self._handle, 0)

    def strength(self) -> float:
        """Return current strength; raises Error if the unit is not visible."""
        return _bc.camp_get(self._handle, 3)

    def focus(self) -> float:
        """Return current focus; raises Error if the unit is not visible."""
        return _bc.camp_get(self._handle, 4)

    def is_visible(self) -> bool:
        """Return whether the camp is currently visible."""
        return _bc.camp_visible(self._handle)

    def base_stats(self) -> CampStats:
        """Return the camp type's design stats."""
        return _bc.camp_design(self._type)

    def attack_range(self) -> int:
        """Return the attack range in Chebyshev tiles; melee camps have range 1."""
        return self.base_stats().base_attack_range


class AltarLock(_bc.AltarLock):
    """Snapshot of an altar lock."""

    __slots__ = ()

    def _entity(self) -> _bc.Entity:
        return _bc.Entity(_EntityKind.ALTAR_LOCK, self._handle)

    def position(self) -> Vec2:
        """Return the altar lock's tile."""
        return self._position

    def team(self) -> Team:
        """Return the team that owns the altar lock."""
        return self._team

    def is_ally(self) -> bool:
        """Check whether this altar lock belongs to the observing team."""
        return self.team() == Team.ALLY

    def is_enemy(self) -> bool:
        """Check whether this altar lock belongs to the opposing team."""
        return self.team() == Team.ENEMY

    def hits_remaining(self) -> int:
        """Return how many hits the altar lock can still take."""
        return self._hits_remaining


_bc._register("AltarLock", AltarLock)


class Ward(_bc.Ward):
    """Immutable snapshot of a visible ward."""

    __slots__ = ()

    def position(self) -> Vec2:
        """Return the ward's tile."""
        return self._position

    def team(self) -> Team:
        """Return the team that owns the ward."""
        return self._team

    def is_ally(self) -> bool:
        """Check whether this ward belongs to the observing team."""
        return self.team() == Team.ALLY

    def is_enemy(self) -> bool:
        """Check whether this ward belongs to the opposing team."""
        return self.team() == Team.ENEMY

    def remaining_time(self) -> Optional[int]:
        """Return the rounds before the ward expires, or None if it does not expire."""
        return self._remaining_time

    @staticmethod
    def visible_tiles_from(position: Vec2) -> Bitboard:
        """Return the vision a ward would have at a tile.

        Args:
            position: Hypothetical ward tile.
        """
        return _bc.ward_vision(position.x, position.y)

    def visible_tiles(self) -> Bitboard:
        """Return the tiles this ward sees."""
        position = self._position
        return _bc.ward_vision(position.x, position.y)

    @staticmethod
    def visible_tiles_from_into(position: Vec2, out: Bitboard) -> None:
        """Like visible_tiles_from, but writes into out.

        Args:
            position: Hypothetical ward tile.
            out: Destination Bitboard; its previous contents are overwritten.
        """
        _bc.ward_vision_into(position.x, position.y, out)

    def visible_tiles_into(self, out: Bitboard) -> None:
        """Like visible_tiles, but writes into out.

        Args:
            out: Destination Bitboard; its previous contents are overwritten.
        """
        position = self._position
        _bc.ward_vision_into(position.x, position.y, out)

_bc._register("Ward", Ward)


class Teleport(_bc.Teleport):
    """Snapshot of a channelling teleport and its reserved destination."""

    __slots__ = ()

    def champion(self) -> Champion:
        """Return the champion channelling the teleport."""
        return Champion._new(self._champion)

    def team(self) -> Team:
        """Return the team of the teleporting champion."""
        return self._team

    def is_ally(self) -> bool:
        """Check whether this teleport belongs to the observing team."""
        return self.team() == Team.ALLY

    def is_enemy(self) -> bool:
        """Check whether this teleport belongs to the opposing team."""
        return self.team() == Team.ENEMY

    def position(self) -> Vec2:
        """Return the reserved destination tile."""
        return self._position

    def remaining_time(self) -> int:
        """Return the rounds left before the teleport completes."""
        return self._remaining_time

    def visible_tiles(self) -> Bitboard:
        """Return the tiles visible from the reservation."""
        position = self._position
        return _bc.hypothetical_vision(self._type, position.x, position.y)

    def visible_tiles_into(self, out: Bitboard) -> None:
        """Like visible_tiles, but writes into out.

        Args:
            out: Destination Bitboard; its previous contents are overwritten.
        """
        position = self._position
        _bc.hypothetical_vision_into(self._type, position.x, position.y, out)


_bc._register("Teleport", Teleport)


AbilityInfo = Union[
    AbilityAttackReset, AbilityDamage, AbilityAoeDamage, AbilityCc, AbilityHeal, AbilityDash, AbilityShield
]
_ABILITY_TYPES: tuple[type, ...] = (
    type(None), AbilityAttackReset, AbilityDamage, AbilityAoeDamage, AbilityCc, AbilityHeal, AbilityDash, AbilityShield
)


def _require_self(handle: int) -> None:
    own = _cache.my_champion
    if own < 0 or handle != own:
        raise ValueError("action is only available for self")


def _target_entity(target: Union[Unit, AltarLock]) -> _bc.Entity:
    if isinstance(target, Unit):
        return target._entity
    return target._entity()


class Ability:
    """One of a champion's two ability slots."""

    __slots__ = ("_owner", "_slot", "_design", "_info", "_info_error")
    _owner: _ChampionHandle
    _slot: int
    _design: bool
    _info: Optional[AbilityInfo]
    _info_error: Optional[Error]

    def __init__(self) -> None:
        raise TypeError("Ability cannot be constructed directly")

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Ability):
            return NotImplemented
        return self._key() == other._key()

    def __hash__(self) -> int:
        return hash(self._key())

    def _key(self) -> tuple[object, ...]:
        return (self._owner, self._slot, self._design, self._info if self._design else None)

    def __repr__(self) -> str:
        return f"Ability(owner={self._owner}, slot={self._slot})"

    @classmethod
    def _new(cls, owner: _ChampionHandle, slot: int) -> Ability:
        self = cls.__new__(cls)
        self._owner = owner
        self._slot = slot
        self._design = False
        self._info = None
        self._info_error = None
        try:
            self._info = _bc.ability_props(owner, slot)[0]
        except Error as failure:
            if failure.code != ErrorCode.API_NOT_IN_VISION:
                raise
            self._info_error = failure
        return self

    @classmethod
    def _from_design(cls, info: Optional[AbilityInfo], slot: int) -> Ability:
        self = cls.__new__(cls)
        self._owner = 0xFFFFFFFF  # type: ignore[assignment]
        self._slot = slot
        self._design = True
        self._info = info
        self._info_error = None
        return self

    def slot(self) -> int:
        """Return the ability's slot index on its champion."""
        return self._slot

    def cooldown(self) -> int:
        """Return the remaining cooldown; 0 means ready."""
        if self._design:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no live cooldown")
        return _bc.champion_cooldown(self._owner, self._slot)

    def is_on_cooldown(self) -> bool:
        """Return whether the cooldown is still running."""
        return self.cooldown() > 0

    def is_ready(self) -> bool:
        """Check whether this ability is ready to cast."""
        return (
            self.info() is not None
            and not self.is_on_cooldown()
            and Champion._new(self._owner).can_cast()
        )

    def valid_targets(self) -> AbilityTargetMask:
        """Return the categories of target the ability accepts."""
        if self._design:
            return AbilityTargetMask(0 if self._info is None else self._info.valid_targets)
        return AbilityTargetMask(_bc.ability_props(self._owner, self._slot)[1])

    def valid_tiles(self) -> Bitboard:
        """Return the tiles within casting range; does not check visibility or occupancy."""
        result = Bitboard()
        self.mask_into(result)
        return result

    def mask_into(self, out: Bitboard) -> None:
        """Like valid_tiles, but writes into out.

        Args:
            out: Destination Bitboard; its previous contents are overwritten.
        """
        info = self.info()
        if info is None:
            raise ValueError("ability has no properties")
        cast_range = getattr(info, "range", None)
        if cast_range is None:
            raise ValueError("ability has no casting range")
        if self._design:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no position")
        out.clear()
        out.draw_chebyshev(_bc.champion_get(self._owner, 2), cast_range)

    def can_target_camps(self) -> bool:
        """Return whether the ability can target camps."""
        return AbilityTargetMask.CAMPS in self.valid_targets()

    def can_target_enemies(self) -> bool:
        """Return whether the ability can target enemy champions."""
        return AbilityTargetMask.ENEMIES in self.valid_targets()

    def can_target_allies(self) -> bool:
        """Return whether the ability can target allied champions."""
        return AbilityTargetMask.ALLIES in self.valid_targets()

    def can_target_self(self) -> bool:
        """Return whether the ability can target its caster."""
        return AbilityTargetMask.SELF in self.valid_targets()

    def can_target_ground(self) -> bool:
        """Return whether the ability can target a tile."""
        return AbilityTargetMask.GROUND in self.valid_targets()

    def info(self, kind: Optional[AbilityKind] = None) -> Optional[AbilityInfo]:
        """Ability properties, or None for an empty slot; with kind, require that kind.

        Args:
            kind: Optional ability kind the slot must have; raises if it differs.
        """
        if self._info_error is not None:
            raise self._info_error
        if kind is None:
            return self._info
        if self._info is None:
            raise ValueError("ability has no properties")
        if not self.has(kind):
            raise TypeError("ability is not of the requested kind")
        return self._info

    def get(self, kind: type[_A]) -> _A:
        """Return the ability's payload of the requested kind; raises if the slot holds a different kind.

        Args:
            kind: Ability kind to extract.
        """
        info = self.info()
        if info is None:
            raise ValueError("ability has no properties")
        if not isinstance(info, kind):
            raise TypeError("ability is not of the requested kind")
        return info

    def has(self, kind: Union[AbilityKind, type[AbilityInfo]]) -> bool:
        """Return whether the slot holds an ability of a kind.

        Args:
            kind: Ability kind to test for.
        """
        info = self._info
        if info is None:
            return False
        if isinstance(kind, int):
            return 0 < kind < len(_ABILITY_TYPES) and type(info) is _ABILITY_TYPES[kind]
        return isinstance(info, kind)

    def is_attack_reset(self) -> bool:
        """Check whether this is an attack-reset ability."""
        return self.has(AbilityKind.ATTACK_RESET)

    def is_damage(self) -> bool:
        """Check whether this is a damage ability."""
        return self.has(AbilityKind.DAMAGE)

    def is_aoe_damage(self) -> bool:
        """Check whether this is an area-damage ability."""
        return self.has(AbilityKind.AOE_DAMAGE)

    def is_cc(self) -> bool:
        """Check whether this is a crowd-control ability."""
        return self.has(AbilityKind.CC)

    def is_heal(self) -> bool:
        """Check whether this is a healing ability."""
        return self.has(AbilityKind.HEAL)

    def is_dash(self) -> bool:
        """Check whether this is a dash ability."""
        return self.has(AbilityKind.DASH)

    def is_shield(self) -> bool:
        """Check whether this is a shield ability."""
        return self.has(AbilityKind.SHIELD)

    def _require_runtime(self) -> None:
        if self._design:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design ability has no live owner")

    def damage_amount(self, target: Optional[Champion] = None) -> float:
        """Return the damage the ability would deal, after mitigation and shields if a target is given.

        Args:
            target: Optional visible champion that would be hit.
        """
        self._require_runtime()
        if target is None:
            return _bc.ability_damage(self._owner, self._slot)
        return _bc.ability_damage_against(self._owner, self._slot, target._handle)

    def heal_amount(self, target: Optional[Champion] = None) -> float:
        """Return the healing the ability would do, capped at the target's missing health if a target is given.

        Args:
            target: Optional visible champion that would be healed.
        """
        self._require_runtime()
        if target is None:
            return _bc.ability_heal(self._owner, self._slot)
        return _bc.ability_heal_against(self._owner, self._slot, target._handle)

    def shield_amount(self, target: Optional[Champion] = None) -> float:
        """Return the shield the ability would grant.

        Args:
            target: Optional visible champion that would be shielded.
        """
        self._require_runtime()
        if target is None:
            return _bc.ability_shield(self._owner, self._slot)
        return _bc.ability_shield_against(self._owner, self._slot, target._handle)

    def use(self, target: Optional[Union[Unit, Vec2]] = None) -> bool:
        """Cast on a unit, at a ground tile, or without a target.

        Unit casts land on the target's tile. Returns False for a newly revealed obstruction;
        known invalid targets raise Error.

        Args:
            target: Champion or camp to cast on, a tile to cast at, or None for an untargeted ability.
        """
        _require_self(self._owner)
        if target is None:
            entity = None
            destination = None
        elif isinstance(target, Vec2):
            entity = None
            destination = target
        elif isinstance(target, Unit):
            entity = _target_entity(target)
            destination = None
        else:
            raise TypeError("Ability.use expects a Unit, Vec2, or None")
        x, y = (-1, -1) if destination is None else (destination.x, destination.y)
        return _bc.cast(self._slot, 0 if entity is None else entity.kind, 0 if entity is None else entity.handle, x, y)


class Consumable:
    """A champion's consumable slot of one type."""

    __slots__ = ("_owner", "_type")
    _owner: _ChampionHandle
    _type: ConsumableType

    def __init__(self) -> None:
        raise TypeError("Consumable cannot be constructed directly")

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Consumable):
            return NotImplemented
        return self._owner == other._owner and self._type == other._type

    def __hash__(self) -> int:
        return hash((self._owner, self._type))

    def __repr__(self) -> str:
        return f"Consumable(owner={self._owner}, kind={self._type!r})"

    @classmethod
    def _new(cls, owner: _ChampionHandle, kind: ConsumableType) -> Consumable:
        self = cls.__new__(cls)
        self._owner = owner
        self._type = kind
        return self

    def kind(self) -> ConsumableType:
        """Return the consumable's type."""
        return self._type

    def count(self) -> int:
        """Return how many the champion is carrying."""
        return _bc.champion_consumables(self._owner)[_RAW_CONSUMABLE_IDS[self._type]]

    def use(self, destination: Optional[Vec2] = None) -> bool:
        """Return False for a newly revealed obstruction; known invalid destinations raise Error.

        Args:
            destination: Tile to use the consumable at.
        """
        _require_self(self._owner)
        x, y = (-1, -1) if destination is None else (destination.x, destination.y)
        return _bc.use_consumable(_RAW_CONSUMABLE_IDS[self._type], x, y)


_DESIGN_HANDLE = 0xFFFFFFFF


class Champion(Unit):
    """A champion; int(champion) is its engine handle. Champion.design(kind) builds a draft design."""

    __slots__ = ("_handle", "_design_type", "_abilities", "_consumables")
    _handle: _ChampionHandle
    _design_type: Optional[ChampionType]
    _abilities: Optional[tuple[Ability, Ability]]
    _consumables: Optional[tuple[Consumable, ...]]

    @classmethod
    def _new(cls, handle: _ChampionHandle) -> Champion:
        self = cls.__new__(cls)
        self._entity = _bc.Entity(_EntityKind.CHAMPION, handle)
        self._design = False
        self._handle = handle
        self._design_type = None
        self._abilities = None
        self._consumables = None
        return self

    @classmethod
    def design(cls, kind: ChampionType) -> Champion:
        """Static design of a champion type, as used during the draft.

        Args:
            kind: Champion type to describe.
        """
        _, _, first, second = _cache.champion_designs[kind]
        self = cls.__new__(cls)
        self._entity = _bc.Entity(_EntityKind.CHAMPION, _DESIGN_HANDLE)
        self._design = True
        self._handle = _DESIGN_HANDLE  # type: ignore[assignment]
        self._design_type = kind
        self._abilities = (Ability._from_design(first, 0), Ability._from_design(second, 1))
        self._consumables = None
        return self

    def __index__(self) -> int:
        return self._handle

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Champion):
            return NotImplemented
        return self._handle == other._handle and self._design_type == other._design_type

    def __hash__(self) -> int:
        return hash((self._handle, self._design_type))

    def __repr__(self) -> str:
        if self._design_type is not None:
            return f"Champion.design({ChampionType.to_string(self._design_type)})"
        return f"Champion({self._handle})"

    def _require_runtime(self) -> None:
        if self._design_type is not None:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no live state")

    def consumables(self) -> tuple[Consumable, ...]:
        """Consumable slots, indexed by ConsumableType."""
        slots = self._consumables
        if slots is None:
            handle = self._handle
            slots = self._consumables = tuple(
                _builtins.map(Consumable._new, (handle,) * ConsumableType.COUNT, ConsumableType._members)
            )
        return slots

    def health(self) -> float:
        """Return current health; raises Error if the unit is not visible."""
        if self._design_type is not None:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no live state")
        return _bc.champion_get(self._handle, 3)

    def max_health(self) -> float:
        """Return current maximum health; raises Error if the unit is not visible."""
        if self._design_type is not None:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no live state")
        return _bc.champion_get(self._handle, 4)

    def champion_name(self) -> ChampionType:
        """Return the champion's type."""
        if self._design_type is not None:
            return self._design_type
        return _bc.champion_get(self._handle, 1)

    def passive(self) -> PassiveType:
        """Return the champion's passive."""
        return _cache.champion_designs[self.champion_name()][1]

    def team(self) -> Team:
        """Return the champion's team."""
        if self._design_type is not None:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no live state")
        return _bc.champion_get(self._handle, 0)

    def is_ally(self) -> bool:
        """Check whether this champion belongs to the observing team."""
        return self.team() == Team.ALLY

    def is_enemy(self) -> bool:
        """Check whether this champion belongs to the opposing team."""
        return self.team() == Team.ENEMY

    def strength(self) -> float:
        """Return current strength; raises Error if the unit is not visible."""
        if self._design_type is not None:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no live state")
        return _bc.champion_get(self._handle, 5)

    def focus(self) -> float:
        """Return current focus; raises Error if the unit is not visible."""
        if self._design_type is not None:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no live state")
        return _bc.champion_get(self._handle, 6)

    def attack_range(self) -> int:
        """Return the attack range in Chebyshev tiles; melee champions have range 1."""
        if self._design_type is not None:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no live state")
        return _bc.champion_get(self._handle, 7)

    def position(self) -> Vec2:
        """Return the current tile; raises Error if the unit is not visible."""
        if self._design_type is not None:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no live state")
        return _bc.champion_get(self._handle, 2)

    def is_alive(self) -> bool:
        """Return whether the champion is alive."""
        self._require_runtime()
        return _bc.champion_is_alive(self._handle)

    def is_visible(self) -> bool:
        """Return whether the champion is currently visible."""
        if self._design_type is not None:
            return False
        return _bc.champion_visible(self._handle)

    def state(self) -> ChampionState:
        """Return the champion's life, channel and action flags."""
        if self._design_type is not None:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no live state")
        return ChampionState(_bc.champion_get(self._handle, 8))

    def can_move(self) -> bool:
        """Check whether this champion can move."""
        return ChampionState.CAN_MOVE in self.state()

    def can_attack(self) -> bool:
        """Check whether this champion can attack."""
        return ChampionState.CAN_ATTACK in self.state()

    def can_cast(self) -> bool:
        """Check whether this champion can cast."""
        return ChampionState.CAN_CAST in self.state()

    def is_recalling(self) -> bool:
        """Check whether this champion is recalling."""
        return ChampionState.RECALLING in self.state()

    def is_submitting_gems(self) -> bool:
        """Check whether this champion is submitting gems."""
        return ChampionState.SUBMITTING_GEMS in self.state()

    def is_teleporting(self) -> bool:
        """Check whether this champion is teleporting."""
        return ChampionState.TELEPORTING in self.state()

    def has_completed_turn(self) -> bool:
        """Check whether this champion has completed its turn."""
        return ChampionState.COMPLETED_TURN in self.state()

    def restrictions(self) -> BuffRestrictions:
        """Return the actions the champion's buffs currently prohibit."""
        self._require_runtime()
        return BuffRestrictions(_bc.champion_restrictions(self._handle))

    def visible_tiles(self) -> Bitboard:
        """Current vision for this champion."""
        if self._design_type is not None:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no position")
        return _bc.champion_vision(self._handle)

    def visible_tiles_from(self, position: Vec2) -> Bitboard:
        """Vision this champion would have at position.

        Args:
            position: Hypothetical tile to see from.
        """
        return _bc.hypothetical_vision(self.champion_name(), position.x, position.y)

    def visible_tiles_into(self, out: Bitboard) -> None:
        """Like visible_tiles, but writes into out.

        Args:
            out: Destination Bitboard; its previous contents are overwritten.
        """
        if self._design_type is not None:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no position")
        _bc.champion_vision_into(self._handle, out)

    def visible_tiles_from_into(self, position: Vec2, out: Bitboard) -> None:
        """Like visible_tiles_from, but writes into out.

        Args:
            position: Hypothetical tile to see from.
            out: Destination Bitboard; its previous contents are overwritten.
        """
        _bc.hypothetical_vision_into(self.champion_name(), position.x, position.y, out)

    def remaining_channel_time(self) -> int:
        """Return the rounds left on the current recall, tribute or teleport channel."""
        self._require_runtime()
        return _bc.champion_channel_time(self._handle)

    def ward_positions(self) -> list[Vec2]:
        """Return the tiles of this champion's placed wards."""
        _require_self(self._handle)
        return _bc.champion_ward_positions(self._handle)

    def ward_cooldown(self) -> int:
        """Return the rounds until another ward can be placed."""
        _require_self(self._handle)
        return _bc.champion_ward_cooldown(self._handle)

    def ward_placement_tiles(self) -> Bitboard:
        """Return the tiles where this champion could place a ward now."""
        result = Bitboard()
        self.ward_placement_tiles_into(result)
        return result

    def ward_placement_tiles_into(self, out: Bitboard) -> None:
        """Like ward_placement_tiles, but writes into out.

        Args:
            out: Destination Bitboard; its previous contents are overwritten.
        """
        _require_self(self._handle)
        out.clear()
        out.draw_chebyshev(self.position(), constants.WARD_PLACEMENT_RADIUS)

    def can_buy(self, item: Union[ItemType, ConsumableType], count: object = _COUNT_OMITTED) -> bool:
        """Return whether the champion can buy an item or consumable in its current state.

        Args:
            item: Item or consumable to buy.
            count: Number of copies to buy.
        """
        _require_self(self._handle)
        if isinstance(item, ConsumableType):
            if count is not _COUNT_OMITTED:
                raise TypeError("count is only valid for item purchases")
            if not _bc.champion_can_shop(self._handle, True):
                return False
            if not 0 <= int(item) < ConsumableType.COUNT:
                return False
            consumable_info = item.info()
            return self.gold_count() >= consumable_info.cost and self.consumables()[int(item)].count() < consumable_info.carry_limit
        if not isinstance(item, ItemType):
            raise TypeError("item must be an ItemType or ConsumableType")
        if count is _COUNT_OMITTED:
            count = 1
        if not isinstance(count, int):
            raise TypeError("count must be an integer")
        if not _bc.champion_can_shop(self._handle, False) or count <= 0:
            return False
        if not 0 <= int(item) < ItemType.COUNT:
            return False
        item_info = item.info()
        return item_info.purchasable and self.gold_count() >= item_info.cost * count

    def can_sell(self) -> bool:
        """Return whether the champion can sell items in its current state."""
        _require_self(self._handle)
        return _bc.champion_can_sell(self._handle)

    def attack(self, target: Union[Unit, AltarLock]) -> None:
        """Basic-attack a visible champion, camp or altar lock in range; requires self.

        Args:
            target: Unit or altar lock to attack.
        """
        _require_self(self._handle)
        entity = _target_entity(target)
        _bc.attack(entity.kind, entity.handle)

    def base_stats(self) -> ChampionStats:
        """Return the champion type's design stats."""
        return _cache.champion_designs[self.champion_name()][0]

    def abilities(self) -> tuple[Ability, Ability]:
        """Return the champion's ability slots."""
        slots = self._abilities
        if slots is None:
            handle = self._handle
            slots = self._abilities = (Ability._new(handle, 0), Ability._new(handle, 1))
        return slots

    def item_count(self) -> list[int]:
        """Carried item counts, indexed by ItemType."""
        self._require_runtime()
        return _bc.champion_items(self._handle)

    def gem_count(self) -> int:
        """Return the gems the champion is carrying."""
        self._require_runtime()
        return _bc.champion_gems(self._handle)

    def gold_count(self) -> int:
        """Return the gold the champion is carrying."""
        _require_self(self._handle)
        return _bc.champion_gold(self._handle)

    def buy(self, kind: Union[ItemType, ConsumableType], count: int = 1) -> None:
        """Buy count items, or one consumable.

        Args:
            kind: Item or consumable to buy.
            count: Number of items to buy; consumables are bought one at a time.
        """
        _require_self(self._handle)
        if isinstance(kind, ConsumableType):
            if count != 1:
                raise TypeError("consumables are bought one at a time")
            _bc.buy_consumable(_RAW_CONSUMABLE_IDS[kind])
        else:
            _bc.buy_item(kind, count)

    def sell(self, kind: ItemType, count: int = 1) -> None:
        """Sell items; requires self and the engine validates location and inventory.

        Args:
            kind: Item to sell.
            count: Number of copies to sell.
        """
        _require_self(self._handle)
        _bc.sell(kind, count)

    def recall(self) -> None:
        """Start channelling a recall to the team spawn; requires self."""
        _require_self(self._handle)
        _bc.recall()

    def cancel_recall(self) -> None:
        """Cancel the current recall; requires self."""
        _require_self(self._handle)
        if not self.state() & ChampionState.RECALLING:
            raise ValueError("champion is not recalling")
        _bc.cancel_channel()

    def place_ward(self, destination: Vec2) -> bool:
        """Return False for a newly revealed obstruction; known invalid destinations raise Error.

        Args:
            destination: Tile to place the ward on.
        """
        _require_self(self._handle)
        return _bc.place_ward(destination.x, destination.y)

    def submit_gems(self, altar: Vec2) -> None:
        """Start tributing carried gems at an altar; requires self.

        Args:
            altar: Altar tile to tribute at.
        """
        _require_self(self._handle)
        _bc.submit_gems(altar.x, altar.y)

    def cancel_submit_gems(self) -> None:
        """Cancel the current tribute; requires self."""
        _require_self(self._handle)
        if not self.state() & ChampionState.SUBMITTING_GEMS:
            raise ValueError("champion is not submitting gems")
        _bc.cancel_channel()

    def move(self, delta: Vec2) -> bool:
        """Return False for a newly revealed obstruction; known invalid destinations raise Error.

        Args:
            delta: Displacement from the current position.
        """
        _require_self(self._handle)
        destination = self.position() + delta
        return _bc.move_to(destination.x, destination.y)

    def move_absolute(self, destination: Vec2) -> bool:
        """Return False for a newly revealed obstruction; known invalid destinations raise Error.

        Args:
            destination: Map tile to move to.
        """
        _require_self(self._handle)
        return _bc.move_to(destination.x, destination.y)

    def move_towards(self, target: Vec2) -> bool:
        """Move to the reachable tile on a shortest path to target, avoiding walls, holes, altars, the enemy
        spawn and visible units. Return False without moving when already there, unable to move, or target is
        blocked or unreachable, and for a newly revealed obstruction.

        Args:
            target: Map tile to approach.
        """
        _require_self(self._handle)
        origin = self.position()
        if origin == target or ChampionState.CAN_MOVE not in self.state():
            return False
        blocked = (game_map.walls() | game_map.holes() | game_map.altars() | game_map.spawn_region(Team.ENEMY)
                   | vision.champion_tiles() | vision.camp_tiles() | vision.teleport_tiles())
        blocked.set(origin, False)
        if blocked.is_set(target):
            return False
        roots = Bitboard()
        roots.set(target)
        destinations = Bitboard()
        destinations.set(origin)
        distances = DistanceField()
        distances.compute_to(blocked, roots, destinations, Bitboard())
        reachable = blocked.reachable_tiles(origin, self.base_stats().base_move_speed).positions()
        best = min(reachable, key=distances.get, default=origin)
        if distances.get(best) >= distances.get(origin):
            return False
        return self.move_absolute(best)

    def set_priority(self, priority: int) -> None:
        """Set the champion's scheduling priority; requires self.

        Args:
            priority: New priority; takes effect from the next round and persists.
        """
        _require_self(self._handle)
        _bc.set_priority(priority)

    def prev_priority(self) -> int:
        """Return the priority used for the current round."""
        if self._design_type is not None:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no scheduling state")
        return _bc.champion_priority(self._handle, 0)

    def priority(self) -> int:
        """Return the priority that will apply from the next round."""
        if self._design_type is not None:
            raise Error(ErrorCode.API_NOT_IN_VISION, "design champion has no scheduling state")
        return _bc.champion_priority(self._handle, 1)

    def end_turn(self) -> None:
        """End the turn now; requires self."""
        _require_self(self._handle)
        end_turn()


def _absolute_team(team: Team) -> int:
    if team == Team.ALLY:
        return _bc.my_team()
    if team == Team.ENEMY:
        return 1 - _bc.my_team()
    raise ValueError("invalid team")


class DraftMemoryBuffer:
    """Draft memory as read at startup; submit() replaces it during the draft."""

    __slots__ = ("_bytes",)

    def __init__(self) -> None:
        self._bytes: bytes = _bc.draft_memory_read()

    def is_empty(self) -> bool:
        """Return whether no draft memory was submitted."""
        return not self._bytes

    def __len__(self) -> int:
        return len(self._bytes)

    def __getitem__(self, offset: int) -> int:
        if offset < 0:
            raise IndexError("draft memory access outside buffer")
        return self._bytes[offset]

    def __iter__(self) -> Iterator[int]:
        return iter(self._bytes)

    def __bytes__(self) -> bytes:
        return self._bytes

    def submit(self, data: Union[bytes, bytearray, memoryview, DraftMemoryBuffer]) -> None:
        """Submit bytes-like data as the next draft memory.

        Args:
            data: Bytes-like object to pass to this team's gameplay bots.
        """
        if _own_champion() is not None:
            raise Error(ErrorCode.INVALID_ACTION, "draft memory can only be submitted during the draft")
        _bc.draft_memory_submit(data._bytes if isinstance(data, DraftMemoryBuffer) else data)


class SharedMemoryBuffer:
    """Fixed-size team shared memory; index for single bytes, read/write for buffers."""

    __slots__ = ()

    @staticmethod
    def size() -> int:
        """Return the shared memory size in bytes."""
        return _cache.shared_memory_size

    def __len__(self) -> int:
        return _cache.shared_memory_size

    def __getitem__(self, offset: int) -> int:
        if offset < 0 or offset >= _cache.shared_memory_size:
            raise IndexError("shared memory access outside buffer")
        return _bc.shared_memory_get(offset)

    def __setitem__(self, offset: int, value: int) -> None:
        if offset < 0 or offset >= _cache.shared_memory_size:
            raise IndexError("shared memory access outside buffer")
        _bc.shared_memory_set(offset, value)

    def write(self, offset: int, data: Union[bytes, bytearray, memoryview]) -> None:
        """Write a bytes-like object starting at offset.

        Args:
            offset: Byte offset into shared memory.
            data: Bytes-like object to write.
        """
        _check_shared_range(offset, memoryview(data).nbytes)
        _bc.shared_memory_write(offset, data)

    def read(self, offset: int, out: Union[bytearray, memoryview]) -> None:
        """Fill a writable bytes-like object from offset.

        Args:
            offset: Byte offset into shared memory.
            out: Writable bytes-like object; its length sets how many bytes are read.
        """
        _check_shared_range(offset, memoryview(out).nbytes)
        _bc.shared_memory_read(offset, out)


def _check_shared_range(offset: int, length: int) -> None:
    size = _cache.shared_memory_size
    if offset < 0 or offset > size or length > size - offset:
        raise IndexError("shared memory access outside buffer")


shared_memory = SharedMemoryBuffer()

_cached_self: Optional[Champion] = None
_self_loaded = False


def _own_champion() -> Optional[Champion]:
    global _cached_self, _self_loaded
    if not _self_loaded:
        handle: _ChampionHandle = _cache.my_champion
        _cached_self = None if handle == -1 else Champion._new(handle)
        _self_loaded = True
    return _cached_self


def me() -> Champion:
    """This bot's champion; raises ValueError during the draft."""
    own = _cached_self if _cached_self is not None else _own_champion()
    if own is None:
        raise ValueError("there is no champion during the draft")
    return own


def is_draft() -> bool:
    """Return whether this bot instance is running the draft rather than controlling a champion."""
    return _own_champion() is None


def champions(team: Optional[Team] = None) -> list[Champion]:
    """Visible champions of a team, or of both teams (allies first).

    Args:
        team: Team to include, or None for both teams.
    """
    if team is None:
        return [*_builtins.map(Champion._new, _bc.allies()), *_builtins.map(Champion._new, _bc.enemies())]
    _absolute_team(team)
    return list(_builtins.map(Champion._new, _bc.allies() if team == Team.ALLY else _bc.enemies()))


def fuel_remaining() -> int:
    """Return the fuel left in the current turn."""
    remaining = _bc.fuel_remaining()
    if remaining < 0:
        raise RuntimeError("fuel_remaining ABI mismatch: negative fuel")
    return remaining


def end_turn() -> None:
    """End the current turn."""
    _sys.stdout.flush()
    _sys.stderr.flush()
    _bc.end_turn()


def resign() -> None:
    # A successful resign never returns, so flush buffered output first.
    """Resign the game for the bot's team."""
    _sys.stdout.flush()
    _sys.stderr.flush()
    _bc.resign()


def global_submitted_gems(team: Team) -> int:
    """Return the total gems a team has tributed.

    Args:
        team: Team to count.
    """
    return _bc.submitted_gems(_absolute_team(team))


def round_number() -> int:
    """Return the current round number."""
    return _bc.round_number()

if TYPE_CHECKING:
    draft_memory: DraftMemoryBuffer
    champion_designs: tuple[Champion, ...]


def __getattr__(name: str) -> object:
    if name == "draft_memory":
        value: object = DraftMemoryBuffer()
    elif name == "champion_designs":
        value = tuple(_builtins.map(Champion.design, range(12)))  # type: ignore[arg-type]
    else:
        raise AttributeError(f"module 'cambc' has no attribute {name!r}")
    globals()[name] = value
    return value


from . import constants, debug, draft, game_map, vision  # noqa: E402
