# VisionFlow

VisionFlow is a vision-based web copilot demo with:

- `backend/` FastAPI planner (`/next_action`)
- `runner/` Playwright executor
- `demo_app/` stable local job-application UI for reproducible demos

## Local Run

Start backend + demo app:

```bash
cd /Users/boluakande/visionflow
docker compose up --build -d demo_app backend
```

Set up runner once:

```bash
python -m venv runner/.venv && source runner/.venv/bin/activate && pip install -r runner/requirements.txt && playwright install chromium
```

Run the executor:

```bash
source runner/.venv/bin/activate
python runner/run.py
```

Optional runner env controls:

```bash
export DEMO_APP_URL="http://localhost:8080"
export BACKEND_URL="http://localhost:8000/next_action"
export MAX_STEPS="30"
export SLOW_MS="300"
export SETTLE_MS="250"
```

Optional backend kill-switch for demo reliability:

```bash
export DEMO_SCRIPTED_MODE="1"
```

## Cloud Run Deploy (When Billing Is Active)

Use the one-command script:

```bash
cd /Users/boluakande/visionflow
PROJECT_ID="YOUR_PROJECT_ID" REGION="us-central1" GEMINI_API_KEY="YOUR_NEW_KEY" ./infra/deploy_cloud_run.sh
```

The script will:

- enable required APIs
- create/update Secret Manager secret `gemini-api-key`
- grant secret access to runtime service account
- deploy `visionflow-backend` and `visionflow-demo-app`
- print both deployed service URLs

Run navigator against deployed services:

```bash
export DEMO_APP_URL="https://VISIONFLOW-DEMO-APP-URL"
export BACKEND_URL="https://VISIONFLOW-BACKEND-URL/next_action"
source runner/.venv/bin/activate
python runner/run.py
```

## Hackathon Submission Checklist

- [ ] Demo video under 4 minutes
- [ ] Separate proof-of-cloud-deployment clip
- [ ] Architecture diagram
- [ ] Public repo with reproducible setup instructions
