"""Mock analyzers for exercises without pose pipelines yet."""

import random

from app.schemas import AnalyzeResponse, ExerciseType, FormFeedback, Severity


def _severity_for_score(score: float) -> Severity:
    if score >= 75:
        return Severity.info
    if score >= 55:
        return Severity.warning
    return Severity.critical


def mock_analyze(exercise_type: ExerciseType) -> AnalyzeResponse:
    """Deterministic-ish mock feedback for non-squat exercises."""
    seed = hash(exercise_type.value) % 1000
    rng = random.Random(seed)

    aspects = {
        ExerciseType.deadlift: ["hip hinge", "bar path", "back neutrality"],
        ExerciseType.bench_press: ["elbow angle", "bar path", "shoulder stability"],
        ExerciseType.barbell_row: ["torso angle", "pull path", "core bracing"],
    }.get(exercise_type, ["form", "tempo", "range of motion"])

    feedback: list[FormFeedback] = []
    scores: list[float] = []
    for aspect in aspects:
        score = round(rng.uniform(58, 92), 1)
        scores.append(score)
        feedback.append(
            FormFeedback(
                aspect=aspect,
                score=score,
                feedback=f"Mock analysis: {aspect.replace('_', ' ')} looks {'solid' if score >= 75 else 'needs work'}.",
                severity=_severity_for_score(score),
            )
        )

    overall = round(sum(scores) / len(scores), 1)
    rep_count = rng.randint(4, 10)

    return AnalyzeResponse(
        exercise_type=exercise_type,
        overall_score=overall,
        rep_count=rep_count,
        feedback=feedback,
        recommendations=[
            f"Focus on improving your weakest {exercise_type.value.replace('_', ' ')} cue.",
            "Record from the side for better joint-angle visibility.",
            "Use a controlled 2-1-2 tempo on each rep.",
        ],
    )
