# Architecture — Workout Form Coach

Recruiter-skimmable overview of how the system is designed, how squat analysis works today, and how new exercises plug in later.

## System overview

```mermaid
flowchart TB
  subgraph Client
    UI[Next.js App]
    LS[(localStorage client_id)]
  end

  subgraph API["FastAPI (port 8000)"]
    R1["/analyze-upload"]
    R2["/history"]
    Q[Quota check]
    R1 --> Q
    Q --> Router{exercise_type}
    Router -->|squat| Squat[squat_pose_analyzer]
    Router -->|other| Mock[analyzers.mock_analyze]
    Squat --> Resp[AnalyzeResponse]
    Mock --> Resp
    Resp --> Store[supabase_store]
  end

  subgraph Data
    SB[(Supabase Postgres)]
    Mem[(In-memory fallback)]
  end

  UI -->|multipart + X-Client-Id| R1
  UI -->|X-Client-Id| R2
  LS -.-> UI
  Store --> SB
  Store --> Mem
  R2 --> Store
```

**Design choices**

| Layer | Responsibility |
|--------|----------------|
| `routes/` | HTTP, headers, quota, temp file handling |
| `services/` | Analysis pipelines + persistence |
| `schemas.py` | Shared API contracts (Pydantic v2) |
| `core/` | Config, structured errors (e.g. 429 quota) |

Backend owns all DB access. Frontend never talks to Supabase directly.

---

## Upload → analyze → history flow

```mermaid
sequenceDiagram
  participant U as User
  participant F as Next.js
  participant A as FastAPI
  participant P as Pose pipeline
  participant D as Store

  U->>F: Select video + exercise
  F->>F: getClientId() from localStorage
  F->>A: POST /analyze-upload (file, X-Client-Id)
  A->>A: count_recent_analyses (quota)
  alt quota exceeded
    A-->>F: 429 quota_exceeded
  else ok
    A->>A: Save temp file
  alt exercise == squat
      A->>P: analyze_squat_video(path)
      P-->>A: AnalyzeResponse
    else
      A->>A: mock_analyze()
    end
    A->>D: save_analysis(client_id, result)
    A-->>F: AnalyzeResponse JSON
    F->>U: Render scores + feedback
    F->>A: GET /history
    A->>D: list_recent_analyses
    A-->>F: HistoryItem[]
  end
```

---

## Squat pose pipeline (implemented)

```mermaid
flowchart LR
  V[Video file] --> CV[OpenCV decode]
  CV --> Sample[~10 fps frame sample]
  Sample --> MP[MediaPipe Pose Landmarker]
  MP --> LM[33 landmarks / frame]
  LM --> FM[FrameMetrics per frame]
  FM --> Rep[Rep detection]
  Rep --> Score[Aspect scorers]
  Score --> Out[AnalyzeResponse]
```

### Frame metrics (per valid frame)

Landmarks use MediaPipe Pose indices (hips 23/24, knees 25/26, ankles 27/28, shoulders 11/12). Frames with low landmark visibility are dropped.

| Metric | Computation |
|--------|-------------|
| `knee_angle` | Angle at knee: hip–knee–ankle (both legs, averaged) |
| `torso_lean` | Deviation from vertical: hip → shoulder vector |
| `knee_offset` | Normalized horizontal distance knee vs ankle (tracking proxy) |

### Rep detection

1. Smooth knee angles (5-frame moving average).
2. Threshold = 35th percentile of smoothed angles.
3. A rep bottom = knee angle drops below threshold, then rises >15° above it.
4. `rep_count` = number of detected bottoms.

### Scoring (heuristic, documented)

This is **biomechanics-inspired rule scoring**, not a trained model. Appropriate for an applied SWE / CV portfolio project; not positioned as ML research.

| Aspect | Logic | Score range |
|--------|--------|-------------|
| **Depth** | Min knee angle per rep; target ~≤85° at bottom → higher score | 0–100 |
| **Knee tracking** | Mean horizontal knee–ankle offset across rep segments; lower drift → higher score | 0–100 |
| **Torso lean** | Mean forward lean; penalize far from ~32° or excessive lean | 0–100 |
| **Overall** | Mean of three aspect scores | 0–100 |
| **Severity** | info ≥75, warning ≥55, else critical | per aspect |

Coaching copy is templated from score bands (see `squat_pose_analyzer.py`).

### Model artifact

On first squat run, the backend downloads `pose_landmarker_lite.task` from Google’s MediaPipe model bucket (cached under `/tmp/workout-form-coach/`). SSL uses `certifi` for macOS compatibility.

---

## Exercise abstraction (extensibility)

```mermaid
flowchart TD
  ET[ExerciseType enum] --> R[analyze_upload router]
  R --> S{squat?}
  S -->|yes| SA[squat_pose_analyzer.analyze_squat_video]
  S -->|no| MA[analyzers.mock_analyze]
  SA --> AR[AnalyzeResponse]
  MA --> AR
```

**Adding a new exercise (e.g. deadlift):**

1. Implement `services/deadlift_pose_analyzer.py` → returns `AnalyzeResponse`.
2. Branch in `routes/analyze.py` on `ExerciseType.deadlift`.
3. No frontend contract change — same `FormFeedback[]` shape.

Shared contract (`schemas.py`):

- `FormFeedback`: aspect, score, feedback, severity  
- `AnalyzeResponse`: exercise_type, overall_score, rep_count, feedback[], recommendations[]

---

## Persistence

| Field | Purpose |
|-------|---------|
| `client_id` | Anonymous user scope (`X-Client-Id`) |
| `feedback` | JSON array of aspect scores |
| `severity_max` | Worst severity across aspects |
| `created_at` | Quota window + history sort |

Without Supabase env vars, `supabase_store.py` uses an in-process dict so local dev works without cloud setup.

---

## Roadmap (priority order)

Aligned with portfolio leverage — not all required for MVP credit.

| Priority | Item | Status |
|----------|------|--------|
| A | Architecture docs (this file + README) | Done |
| B | Richer outputs (angles, annotated frames, confidence) | Planned |
| C | Stored artifacts (thumbnails, per-rep timestamps) | Planned |
| D | Production deployment (Vercel + Cloud Run / Railway) | Planned |
| — | Real-time webcam | Out of scope unless hackathon |

---

## What this project is / is not

**Is:** Full-stack applied CV pipeline — upload, inference, structured feedback, history, quota, modular analyzers.

**Is not:** Custom neural training, clinical-grade biomechanics, or real-time streaming (yet).

That positioning matches **SWE / applied AI intern** roles, not ML research.
