"""
Squat form analysis using MediaPipe Pose Landmarker (Tasks API) + OpenCV.

Pipeline:
1. Sample video frames and extract 33 pose landmarks per frame.
2. Track hip/knee/ankle angles to detect squat reps (down-up cycles).
3. Score depth, knee tracking, and torso lean across reps.
4. Emit structured FormFeedback for the API response.
"""

from __future__ import annotations

import ssl
import urllib.request
from dataclasses import dataclass
from pathlib import Path

import certifi
import numpy as np

from app.schemas import AnalyzeResponse, ExerciseType, FormFeedback, Severity

_MODEL_URL = (
    "https://storage.googleapis.com/mediapipe-models/pose_landmarker/"
    "pose_landmarker_lite/float16/1/pose_landmarker_lite.task"
)
_MODEL_CACHE = Path("/tmp/workout-form-coach/pose_landmarker_lite.task")


def _ensure_model() -> str:
    _MODEL_CACHE.parent.mkdir(parents=True, exist_ok=True)
    if not _MODEL_CACHE.exists():
        ctx = ssl.create_default_context(cafile=certifi.where())
        with urllib.request.urlopen(_MODEL_URL, context=ctx) as resp:
            _MODEL_CACHE.write_bytes(resp.read())
    return str(_MODEL_CACHE)


@dataclass
class FrameMetrics:
    hip_y: float
    knee_angle: float
    hip_angle: float
    torso_lean: float
    knee_offset: float


def _angle(a: np.ndarray, b: np.ndarray, c: np.ndarray) -> float:
    ba = a - b
    bc = c - b
    cosine = np.dot(ba, bc) / (np.linalg.norm(ba) * np.linalg.norm(bc) + 1e-8)
    return float(np.degrees(np.arccos(np.clip(cosine, -1.0, 1.0))))


def _lm(landmarks, idx: int, w: int, h: int) -> np.ndarray:
    lm = landmarks[idx]
    return np.array([lm.x * w, lm.y * h, lm.z * w])


def _extract_metrics(landmarks, w: int, h: int) -> FrameMetrics | None:
    if len(landmarks) < 29:
        return None

    try:
        l_hip, r_hip = _lm(landmarks, 23, w, h), _lm(landmarks, 24, w, h)
        l_knee, r_knee = _lm(landmarks, 25, w, h), _lm(landmarks, 26, w, h)
        l_ankle, r_ankle = _lm(landmarks, 27, w, h), _lm(landmarks, 28, w, h)
        l_shoulder, r_shoulder = _lm(landmarks, 11, w, h), _lm(landmarks, 12, w, h)
    except (IndexError, AttributeError):
        return None

    vis = [landmarks[i].visibility for i in (23, 24, 25, 26, 27, 28, 11, 12)]
    if min(vis) < 0.5:
        return None

    hip = (l_hip + r_hip) / 2
    knee = (l_knee + r_knee) / 2
    ankle = (l_ankle + r_ankle) / 2
    shoulder = (l_shoulder + r_shoulder) / 2

    knee_angle = (_angle(hip, knee, ankle) + _angle(r_hip, r_knee, r_ankle)) / 2
    hip_angle = (_angle(l_shoulder, l_hip, l_knee) + _angle(r_shoulder, r_hip, r_knee)) / 2

    vertical = np.array([0.0, -1.0])
    torso_vec = shoulder[:2] - hip[:2]
    torso_vec = torso_vec / (np.linalg.norm(torso_vec) + 1e-8)
    torso_lean = float(np.degrees(np.arccos(np.clip(np.dot(vertical, torso_vec), -1.0, 1.0))))

    knee_offset = float(abs(knee[0] - ankle[0]) / (w + 1e-8))

    return FrameMetrics(
        hip_y=hip[1] / h,
        knee_angle=knee_angle,
        hip_angle=hip_angle,
        torso_lean=torso_lean,
        knee_offset=knee_offset,
    )


def _detect_reps(metrics: list[FrameMetrics]) -> list[tuple[int, int]]:
    if len(metrics) < 10:
        return []

    angles = np.array([m.knee_angle for m in metrics])
    smoothed = np.convolve(angles, np.ones(5) / 5, mode="same")
    threshold = float(np.percentile(smoothed, 35))

    in_squat = False
    rep_bottoms: list[int] = []
    for i, angle in enumerate(smoothed):
        if not in_squat and angle < threshold:
            in_squat = True
            rep_bottoms.append(i)
        elif in_squat and angle > threshold + 15:
            in_squat = False

    reps: list[tuple[int, int]] = []
    for i, bottom in enumerate(rep_bottoms):
        start = rep_bottoms[i - 1] if i > 0 else max(0, bottom - 15)
        end = (
            rep_bottoms[i + 1]
            if i + 1 < len(rep_bottoms)
            else min(len(metrics) - 1, bottom + 15)
        )
        reps.append((start, end))
    return reps


def _score_depth(metrics: list[FrameMetrics], reps: list[tuple[int, int]]) -> tuple[float, str]:
    if not reps:
        return 55.0, "Could not detect clear squat reps. Film from the side with full body in frame."

    depths: list[float] = []
    for start, end in reps:
        segment = metrics[start:end]
        if not segment:
            continue
        min_angle = min(m.knee_angle for m in segment)
        depth_score = max(0, min(100, 100 - (min_angle - 85) * 1.2))
        depths.append(depth_score)

    avg = float(np.mean(depths)) if depths else 55.0
    if avg >= 80:
        msg = "Solid depth — hips reach roughly parallel or below on most reps."
    elif avg >= 65:
        msg = "Moderate depth. Aim to break parallel consistently."
    else:
        msg = "Depth is shallow. Control the descent and sit hips back/knees out."
    return round(avg, 1), msg


def _score_knee_tracking(metrics: list[FrameMetrics], reps: list[tuple[int, int]]) -> tuple[float, str]:
    if not reps:
        return 60.0, "Knee tracking unclear — ensure knees and ankles stay visible."

    offsets: list[float] = []
    for start, end in reps:
        segment = metrics[start:end]
        offsets.extend(m.knee_offset for m in segment)

    mean_offset = float(np.mean(offsets)) if offsets else 0.08
    score = max(0, min(100, 100 - mean_offset * 800))
    if score >= 78:
        msg = "Knees track well over mid-foot through the descent."
    elif score >= 62:
        msg = "Minor knee cave or forward drift. Push knees out in line with toes."
    else:
        msg = "Knees drift inward or far forward. Brace core and track knees over 2nd–3rd toe."
    return round(score, 1), msg


def _score_torso_lean(metrics: list[FrameMetrics], reps: list[tuple[int, int]]) -> tuple[float, str]:
    if not reps:
        return 65.0, "Torso angle could not be measured reliably."

    leans: list[float] = []
    for start, end in reps:
        segment = metrics[start:end]
        leans.extend(m.torso_lean for m in segment)

    mean_lean = float(np.mean(leans)) if leans else 25.0
    if mean_lean <= 42:
        score = max(60, min(100, 100 - abs(mean_lean - 32) * 1.5))
    else:
        score = max(40, 100 - (mean_lean - 42) * 2.5)

    if score >= 78:
        msg = "Torso stays braced with controlled forward lean."
    elif score >= 62:
        msg = "Some excessive lean or rounding. Keep chest up and lats engaged."
    else:
        msg = "Torso collapses or over-leans. Reduce load and focus on braced midline."
    return round(score, 1), msg


def _severity(score: float) -> Severity:
    if score >= 75:
        return Severity.info
    if score >= 55:
        return Severity.warning
    return Severity.critical


_MAX_WIDTH = 360
_MAX_SAMPLES = 60  # cap pose samples for Render free tier (~512MB RAM)
_MAX_ANALYZE_SECONDS = 45  # only process the first N seconds of long phone videos


def _resize_frame(frame, max_width: int = _MAX_WIDTH):
    import cv2

    h, w = frame.shape[:2]
    if w <= max_width:
        return frame
    scale = max_width / w
    return cv2.resize(frame, (max_width, int(h * scale)))


def analyze_squat_video(video_path: str) -> AnalyzeResponse:
    import gc

    # Lazy import: keeps FastAPI booting on /health if CV libs are misconfigured
    import cv2
    import mediapipe as mp
    from mediapipe.tasks import python as mp_python
    from mediapipe.tasks.python import vision

    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        raise ValueError("Could not open video file")

    fps = cap.get(cv2.CAP_PROP_FPS) or 30
    frame_skip = max(1, int(fps / 5))  # ~5 samples/sec (enough for rep detection)
    max_frame_idx = int(fps * _MAX_ANALYZE_SECONDS)

    options = vision.PoseLandmarkerOptions(
        base_options=mp_python.BaseOptions(model_asset_path=_ensure_model()),
        running_mode=vision.RunningMode.VIDEO,
        num_poses=1,
        min_pose_detection_confidence=0.5,
        min_pose_presence_confidence=0.5,
        min_tracking_confidence=0.5,
    )

    metrics: list[FrameMetrics] = []
    frame_idx = 0
    samples_taken = 0

    with vision.PoseLandmarker.create_from_options(options) as landmarker:
        while samples_taken < _MAX_SAMPLES and frame_idx < max_frame_idx:
            ok, frame = cap.read()
            if not ok:
                break
            if frame_idx % frame_skip != 0:
                frame_idx += 1
                continue

            frame = _resize_frame(frame)
            rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            h, w = rgb.shape[:2]
            mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb)
            timestamp_ms = int((frame_idx / fps) * 1000)
            result = landmarker.detect_for_video(mp_image, timestamp_ms)
            frame_idx += 1
            samples_taken += 1

            if not result.pose_landmarks:
                continue
            landmarks = result.pose_landmarks[0]
            m = _extract_metrics(landmarks, w, h)
            if m:
                metrics.append(m)

    cap.release()
    del cap
    gc.collect()

    reps = _detect_reps(metrics)
    rep_count = len(reps)

    depth_score, depth_msg = _score_depth(metrics, reps)
    knee_score, knee_msg = _score_knee_tracking(metrics, reps)
    torso_score, torso_msg = _score_torso_lean(metrics, reps)

    feedback = [
        FormFeedback(
            aspect="depth",
            score=depth_score,
            feedback=depth_msg,
            severity=_severity(depth_score),
        ),
        FormFeedback(
            aspect="knee_tracking",
            score=knee_score,
            feedback=knee_msg,
            severity=_severity(knee_score),
        ),
        FormFeedback(
            aspect="torso_lean",
            score=torso_score,
            feedback=torso_msg,
            severity=_severity(torso_score),
        ),
    ]

    overall = round((depth_score + knee_score + torso_score) / 3, 1)
    recommendations: list[str] = []
    if depth_score < 70:
        recommendations.append("Use a box or pause squat to groove consistent depth.")
    if knee_score < 70:
        recommendations.append("Practice tempo squats with a mini-band above knees for tracking.")
    if torso_score < 70:
        recommendations.append("Add front squats or goblet squats to reinforce upright torso.")
    if not recommendations:
        recommendations.append(
            "Form is solid — progress load gradually while filming every few sessions."
        )
    recommendations.append(
        "Film from the side at hip height with full body in frame for best tracking."
    )

    return AnalyzeResponse(
        exercise_type=ExerciseType.squat,
        overall_score=overall,
        rep_count=rep_count,
        feedback=feedback,
        recommendations=recommendations,
    )
