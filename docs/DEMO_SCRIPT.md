# Demo video script (60–90 seconds)

Record against the **live app**: https://visionflow.vercel.app  
Use a real side-view squat clip (5 slow reps, full body in frame).

## 0:00–0:10 — Hook

- Show landing: **Workout Form Coach** / Performance Lab UI
- One line: “Upload a squat video, get rep counts and coaching cues.”

## 0:10–0:30 — Upload

- Select **Squat (pose analysis)**
- Pick video (phone camera roll is fine)
- Tap **Analyze form**
- Brief loading: “Uploading …MB to cloud storage…” then “Analyzing on server…”

## 0:30–0:55 — Results

- Scroll results: overall score, **rep count** (e.g. 5 reps), depth / knee tracking / torso lean
- Read one feedback card aloud
- Show recommendations list

## 0:55–1:15 — History

- Point at **Recent analyses**
- Tap an item → modal with full detail
- Optional: “History is scoped per browser via client ID; five free analyses per day.”

## 1:15–1:25 — Tech (end card)

- On screen: `Next.js → Supabase Storage → FastAPI → MediaPipe → Postgres`
- GitHub: https://github.com/bluxio/visionflow

## Recording tips

- **Production:** https://visionflow.vercel.app (Wi‑Fi, keep tab open during upload)
- **Local:** `npm run dev` in `frontend/`, `uvicorn` in `backend/` on :8000
- Film squat: side angle, hip height, feet visible, socks/shoes for contrast
- Capture screenshots for README: upload screen, results (with rep count), history panel → save as `docs/assets/results.png`

## LinkedIn / applications (one-liner)

Built and deployed a computer vision fitness coach: upload squat video → MediaPipe pose analysis → rep count + form scores + coaching cues, with cloud storage and persistent history.
