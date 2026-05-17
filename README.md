# Workout Form Coach

> Full-stack AI fitness app that analyzes squat videos using MediaPipe/OpenCV and returns form feedback, movement insights, and client-scoped analysis history.

Full-stack MVP for upload-based workout form analysis with structured coaching feedback.

**Resume bullet:** Built a full-stack computer vision fitness app using FastAPI, Next.js, TypeScript, OpenCV, and MediaPipe to analyze squat videos, estimate form quality, detect movement issues, and return coach-style feedback with analysis history and quota tracking.

## Stack

- **Frontend:** Next.js (App Router), TypeScript, Tailwind CSS
- **Backend:** FastAPI, Pydantic v2, MediaPipe + OpenCV (squat analysis)
- **Database:** Supabase (Postgres) — optional for local dev (in-memory fallback)

## Project structure

```
backend/app/          # FastAPI application
  routes/             # HTTP handlers
  services/           # analyzers, Supabase store, squat pose pipeline
  core/               # config, errors
frontend/             # Next.js UI (Performance Lab aesthetic)
supabase/schema.sql   # Postgres schema
```

## Quick start

### 1. Backend

```bash
cd backend
python3.11 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
cp .env.example .env
# Optional: set SUPABASE_URL and SUPABASE_SERVICE_ROLE_KEY
uvicorn app.main:app --reload --port 8000
```

API docs: http://localhost:8000/docs

### 2. Supabase (production / persistent history)

1. Create a Supabase project.
2. Run `supabase/schema.sql` in the SQL editor.
3. Add to `backend/.env`:
   - `SUPABASE_URL`
   - `SUPABASE_SERVICE_ROLE_KEY`

Without Supabase, the backend uses an in-memory store (fine for local testing).

### 3. Frontend

```bash
cd frontend
npm install
cp .env.example .env.local
npm run dev
```

Open http://localhost:3000

### 4. Docker (backend only)

```bash
docker compose up --build backend
```

Update root `docker-compose.yml` to point at the new backend if needed.

## API overview

| Method | Path | Notes |
|--------|------|--------|
| POST | `/analyze` | JSON body, mock analyzer |
| POST | `/analyze-upload` | Multipart video + `X-Client-Id` |
| GET | `/history` | Recent analyses, `X-Client-Id` |
| GET | `/history/{id}` | Full analysis detail |

**Quota:** 5 analyses per client per rolling 24h (HTTP 429 when exceeded).

## Squat analysis

`squat` uploads run the MediaPipe Pose pipeline (`squat_pose_analyzer.py`):

- Landmark extraction per frame
- Rep detection via knee-angle cycles
- Scores: depth, knee tracking, torso lean
- Structured `FormFeedback` + recommendations

Other exercises use the mock analyzer until dedicated pipelines are added.

## Environment variables

**Backend** (`backend/.env.example`):

- `SUPABASE_URL`, `SUPABASE_SERVICE_ROLE_KEY`
- `CORS_ORIGINS` (default `http://localhost:3000`)
- `DAILY_ANALYSIS_LIMIT`, `QUOTA_WINDOW_HOURS`
- `UPLOAD_DIR`

**Frontend** (`frontend/.env.example`):

- `NEXT_PUBLIC_API_URL` (default `http://localhost:8000`)

## Client ID

The frontend generates a persistent UUID in `localStorage` and sends it as `X-Client-Id` on all API calls for history scoping and quota tracking.
