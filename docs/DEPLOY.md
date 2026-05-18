# Deployment — Workout Form Coach

Monorepo layout: `frontend/` (Vercel) + `backend/` (Render) + Supabase.

## 1. Supabase

1. Create project at [supabase.com](https://supabase.com).
2. SQL Editor → run `supabase/schema.sql`.
3. SQL Editor → run `supabase/storage_setup.sql` if your bucket is not `uploads` yet, or to add anon policies on `uploads`.
4. Settings → API → copy **Project URL**, **anon public** key (frontend), and **service_role** key (backend only).

## 2. Backend (Render)

1. [Render Dashboard](https://dashboard.render.com) → **New** → **Blueprint** → connect `bluxio/visionflow` (or **Web Service** manually).
2. If manual (Docker):
   - **Root Directory:** leave empty (repo root) **or** set `backend` — must match Dockerfile path below
   - **Dockerfile Path:** `backend/Dockerfile` (from repo root) **not** `Dockerfile` alone
   - **Docker Context:** `backend` (or same as root directory)
   - If you see `open Dockerfile: no such file or directory`, the path points at repo root instead of `backend/`.
3. Environment variables:

| Key | Value |
|-----|--------|
| `SUPABASE_URL` | Your Supabase URL |
| `SUPABASE_SERVICE_ROLE_KEY` | Service role key |
| `CORS_ORIGINS` | `http://localhost:3000,https://YOUR-APP.vercel.app` |
| `DAILY_ANALYSIS_LIMIT` | `5` |
| `QUOTA_WINDOW_HOURS` | `24` |

4. Deploy → note public URL, e.g. `https://workout-form-coach-api.onrender.com`.
5. Verify: `curl https://YOUR-API.onrender.com/health`

**Free tier:** service sleeps after inactivity; first request may take 30–60s.

## 3. Frontend (Vercel)

1. [Vercel](https://vercel.com) → **Add New Project** → import GitHub repo.
2. **Root Directory:** `frontend`
3. Framework: Next.js (auto-detected)
4. Environment variables:

| Key | Value |
|-----|--------|
| `NEXT_PUBLIC_SUPABASE_URL` | Same as `SUPABASE_URL` |
| `NEXT_PUBLIC_SUPABASE_ANON_KEY` | Supabase **anon** key (required for large uploads) |
| `NEXT_PUBLIC_SUPABASE_STORAGE_BUCKET` | `uploads` (must match Supabase bucket id exactly) |
| `NEXT_PUBLIC_API_URL` | `https://workout-form-coach-api.onrender.com` (optional; hardcoded fallback exists) |

5. Deploy → note URL, e.g. `https://visionflow-dun.vercel.app`.

## 4. Wire CORS (after Vercel URL exists)

Update Render env `CORS_ORIGINS`:

```
http://localhost:3000,https://visionflow-dun.vercel.app
```

Redeploy backend (or wait for env reload).

## 5. README demo block

Add to README after live:

```markdown
## Live demo
- App: https://your-app.vercel.app
- API: https://your-api.onrender.com/docs
```

## 6. Record demo (60–90s)

See `docs/DEMO_SCRIPT.md` — use **production URLs** and a real side-angle squat video.
