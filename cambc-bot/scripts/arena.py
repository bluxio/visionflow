#!/usr/bin/env python3
"""Play BOT against each OPPONENT on every map from both sides and summarise the results.

Usage: scripts/arena.py BOT OPPONENT [OPPONENT ...] [--maps a,b] [--champions]
"""
import argparse
import json
import subprocess
import tempfile
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CAMBC = ROOT / ".venv" / "bin" / "cambc"
CHAMPIONS = ["Ryth", "Vesper", "Vendetta", "Cassi", "Issac", "Stellar", "Prism", "Marcella", "Willow", "Ren", "Varg", "Tavi"]


def play(bot_a, bot_b, map_path, out_dir):
    stats = out_dir / f"{bot_a}-{bot_b}-{map_path.stem}.json"
    subprocess.run(
        [str(CAMBC), "run", bot_a, bot_b, "--map", str(map_path), "-o", str(out_dir / "replays"), "--statistics", str(stats)],
        cwd=ROOT, capture_output=True, text=True, check=False,
    )
    return json.loads(stats.read_text()) if stats.exists() else None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("bot")
    parser.add_argument("opponents", nargs="+")
    parser.add_argument("--maps", default="")
    parser.add_argument("--champions", action="store_true", help="per-champion breakdown for BOT")
    parser.add_argument("--seeds", default="", help="comma-separated seeds; BOT is copied once per seed")
    parser.add_argument("--quiet", action="store_true", help="only print totals")
    args = parser.parse_args()

    maps = sorted((ROOT / "maps").glob("*.map26a"))
    if args.maps:
        wanted = set(args.maps.split(","))
        maps = [m for m in maps if m.stem in wanted]

    variants = [args.bot]
    if args.seeds:
        variants = []
        for seed in args.seeds.split(","):
            name = f"{args.bot}-s{seed}"
            subprocess.run([str(ROOT / "scripts" / "variant.sh"), name, "--from", args.bot, f"seed = {seed}"],
                           cwd=ROOT, check=True)
            variants.append(name)

    with ThreadPoolExecutor(max_workers=8) as pool:
        list(pool.map(lambda bot: subprocess.run([str(CAMBC), "build", bot], cwd=ROOT, capture_output=True, check=True),
                      [*variants, *args.opponents]))

    out_dir = Path(tempfile.mkdtemp(prefix="arena-"))
    jobs = []
    for opponent in args.opponents:
        for variant in variants:
            for map_path in maps:
                jobs.append((opponent, map_path, "A", variant, opponent))
                jobs.append((opponent, map_path, "B", opponent, variant))

    with ThreadPoolExecutor(max_workers=8) as pool:
        results = list(pool.map(lambda job: play(job[3], job[4], job[1], out_dir), jobs))

    per_champion = defaultdict(lambda: defaultdict(float))
    totals = defaultdict(lambda: [0, 0, 0])
    for (opponent, map_path, side, _, _), stats in zip(jobs, results):
        if stats is None:
            print(f"{opponent:<12} {map_path.stem:<10} {side}  FAILED")
            continue
        ours = 0 if side == "A" else 1
        won = stats["outcome"] == ("teamA" if side == "A" else "teamB")
        totals[opponent][0] += won
        totals[opponent][1] += 1
        team_stats = []
        for team in (0, 1):
            members = [c for c in stats["champions"] if c["team"] == team]
            team_stats.append((
                stats["teams"][team]["submittedGems"],
                sum(c["kills"] for c in members),
                sum(c["deaths"] for c in members),
            ))
            if team == ours:
                for c in members:
                    row = per_champion[CHAMPIONS[c["type"]]]
                    row["games"] += 1
                    for key in ("deaths", "kills", "submittedGems", "collectedGems", "monsterDamageDone",
                                "championDamageDone", "championDamageTaken", "earnedGold"):
                        row[key] += c[key]
        us, them = team_stats[ours], team_stats[1 - ours]
        totals[opponent][2] += us[0] - them[0]
        faults = stats["botFaults"][ours]
        if not args.quiet:
            print(f"{opponent:<12} {map_path.stem:<10} {side}  {'WIN ' if won else 'loss'} r{stats['round']:<5}"
                  f" gems {us[0]:>4}-{them[0]:<4} kills {us[1]:>2}-{them[1]:<2} deaths {us[2]:>2}-{them[2]:<2}"
                  + (f" FAULTS={faults}" if faults else ""))

    for opponent, (wins, games, margin) in totals.items():
        print(f"== {args.bot} vs {opponent}: {wins}/{games} ({100 * wins / games:.0f}%, gem margin {margin / games:+.0f})")

    if args.champions:
        print(f"{'champion':<10} {'games':>5} {'deaths':>7} {'kills':>6} {'gems':>6} {'monDmg':>8} {'chDmg':>7} {'chTaken':>8} {'gold':>6}")
        for name, row in sorted(per_champion.items()):
            g = row["games"]
            print(f"{name:<10} {int(g):>5} {row['deaths'] / g:>7.2f} {row['kills'] / g:>6.2f} {row['submittedGems'] / g:>6.0f}"
                  f" {row['monsterDamageDone'] / g:>8.0f} {row['championDamageDone'] / g:>7.0f}"
                  f" {row['championDamageTaken'] / g:>8.0f} {row['earnedGold'] / g:>6.0f}")


if __name__ == "__main__":
    main()
