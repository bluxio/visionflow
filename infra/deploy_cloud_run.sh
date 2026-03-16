#!/usr/bin/env bash
set -euo pipefail

PROJECT_ID="${PROJECT_ID:-}"
REGION="${REGION:-us-central1}"
SECRET_NAME="gemini-api-key"

if [[ -z "$PROJECT_ID" ]]; then
  echo "ERROR: PROJECT_ID is required."
  echo "Example: PROJECT_ID=my-project REGION=us-central1 GEMINI_API_KEY=... ./infra/deploy_cloud_run.sh"
  exit 1
fi

if [[ -z "${GEMINI_API_KEY:-}" ]]; then
  echo "ERROR: GEMINI_API_KEY is required."
  exit 1
fi

gcloud config set project "$PROJECT_ID" >/dev/null
gcloud config set run/region "$REGION" >/dev/null

gcloud services enable \
  run.googleapis.com \
  cloudbuild.googleapis.com \
  artifactregistry.googleapis.com \
  secretmanager.googleapis.com

if gcloud secrets describe "$SECRET_NAME" >/dev/null 2>&1; then
  printf "%s" "$GEMINI_API_KEY" | gcloud secrets versions add "$SECRET_NAME" --data-file=-
else
  printf "%s" "$GEMINI_API_KEY" | gcloud secrets create "$SECRET_NAME" --data-file=-
fi

PROJECT_NUMBER="$(gcloud projects describe "$PROJECT_ID" --format="value(projectNumber)")"
SA="$PROJECT_NUMBER-compute@developer.gserviceaccount.com"
gcloud secrets add-iam-policy-binding "$SECRET_NAME" \
  --member="serviceAccount:$SA" \
  --role="roles/secretmanager.secretAccessor" >/dev/null

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$REPO_ROOT"

gcloud run deploy visionflow-backend \
  --source backend \
  --allow-unauthenticated \
  --set-secrets GEMINI_API_KEY="$SECRET_NAME:latest"

gcloud run deploy visionflow-demo-app \
  --source demo_app \
  --allow-unauthenticated \
  --port 8080

BACKEND_URL="$(gcloud run services describe visionflow-backend --region "$REGION" --format="value(status.url)")"
DEMO_APP_URL="$(gcloud run services describe visionflow-demo-app --region "$REGION" --format="value(status.url)")"

echo "visionflow-backend URL: $BACKEND_URL"
echo "visionflow-demo-app URL: $DEMO_APP_URL"
