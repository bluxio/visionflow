from typing import Any, Optional
from _cambc import Bitboard, ChampionStats, _AbilityInfo

settings: tuple[Any, ...]
my_champion: Any
champion_designs: tuple[tuple[ChampionStats, Any, Optional[_AbilityInfo], Optional[_AbilityInfo]], ...]
symmetry: Any
terrain: tuple[Bitboard, Bitboard, Bitboard, Bitboard]
spawns: tuple[Bitboard, Bitboard]
width: int
height: int
shared_memory_size: int
