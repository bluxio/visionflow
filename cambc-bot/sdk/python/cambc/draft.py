"""Draft-phase queries and picks."""

from __future__ import annotations

import _cambc as _bc

from . import ChampionDraft, Team, _absolute_team


def team_size() -> int:
    """Champions each team drafts."""
    return _bc.draft_num_rounds()


def current_draft(team: Team) -> list[ChampionDraft]:
    """Picks a team has made so far, in order.

    Args:
        team: Team whose picks to return.
    """
    return _bc.draft_picks(_absolute_team(team))


def pick(champion: ChampionDraft) -> None:
    """Pick a champion for the current draft round; only valid during the draft.

    Args:
        champion: Champion to pick.
    """
    _bc.draft_champion(champion)
