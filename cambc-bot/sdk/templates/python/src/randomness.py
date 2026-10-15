import cambc
from cambc import game_map

_state = None


def below(limit):
    # WASI randomness is always zero in matches, so seed from our spawn and this champion's start.
    global _state
    if _state is None:
        spawn = game_map.spawn_region(cambc.Team.ALLY).positions()[0]
        _state = spawn.x * 73856093 ^ spawn.y * 19349663
        if not cambc.is_draft():
            start = cambc.me().position()
            _state ^= start.x * 83492791 ^ start.y * 2654435761
    _state = (_state * 6364136223846793005 + 1442695040888963407) % 2**64
    return (_state >> 33) % limit
