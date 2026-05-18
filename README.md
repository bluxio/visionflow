# Workout Form Coach

I built this because I care about training smarter—not just lifting more. I wanted a simple way to upload a squat set, get rep counts, and see where form breaks down, without needing a coach in the room every session.

It’s a side project that grew into a full deployed stack: phone video in, pose analysis out, history saved.

| | |
|---|---|
| **App** | https://visionflow-dun.vercel.app |
| **Demo** | https://youtu.be/mvUUCDUzBVg |
| **API** | https://workout-form-coach-api.onrender.com/docs |

Upload a side-view squat (phone video is fine, up to 200MB). The backend analyzes the first ~45 seconds.

## Screenshots

| Upload | Results | History |
|--------|---------|---------|
| ![Upload](docs/assets/landing.png) | ![Results](docs/assets/results.png) | ![History](docs/assets/history.png) |

## What I learned shipping this

Most of the work wasn’t MediaPipe—it was getting production to behave:

- **Uploads:** Chunking large iPhone `.MOV` files straight to Render kept failing on mobile Wi‑Fi. Moving uploads to **Supabase Storage** fixed it.
- **Bucket mismatch:** The app expected `workout-videos`; my project had `uploads`. Classic “bucket not found” until names aligned.
- **Memory:** Render’s free tier (512MB) OOM’d on big videos. Trimming analyze length, downscaling with ffmpeg, and streaming downloads helped.
- **Rep counting:** Early runs returned **0 reps** on good footage. Loosening visibility gates, smoothing knee angles, and adding a hip-height fallback got **5 reps** on a real phone clip in prod.

That progression (0 reps → 5 reps) is in the history screenshot above.

## How it works

Browser uploads video to Supabase → FastAPI on Render downloads it → OpenCV + MediaPipe Pose Landmarker → heuristic scores for depth, knee tracking, and torso lean → results stored in Postgres.

More detail: [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) · Deploy notes: [`docs/DEPLOY.md`](docs/DEPLOY.md)

**Stack:** Next.js, FastAPI, MediaPipe, OpenCV, Supabase, Render, Vercel.

## Run locally

```bash
# Backend
cd backend && python3.11 -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt && cp .env.example .env
uvicorn app.main:app --reload --port 8000

# Frontend (separate terminal)
cd frontend && npm install && cp .env.example .env.local && npm run dev
```

Open http://localhost:3000. Supabase env vars are optional locally (in-memory history fallback).

## Repo layout

```
backend/app/     FastAPI — routes, squat_pose_analyzer, Supabase store
frontend/        Next.js UI
supabase/        schema + storage setup SQL
```

Squat analysis lives in `backend/app/services/squat_pose_analyzer.py` (rep detection + scoring heuristics, no custom ML model).

---

**Resume line (if useful):** Built and deployed a computer vision fitness app (Next.js, FastAPI, MediaPipe, Supabase, Render) that analyzes squat videos, counts reps, and returns form feedback with persistent history.
