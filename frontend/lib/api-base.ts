/**
 * API base URL resolution.
 * Production calls Render directly (CORS allows *.vercel.app).
 * Avoids Vercel /wfc-api proxy — that hits ~60s serverless timeout during squat analysis.
 */
export const PRODUCTION_API = "https://workout-form-coach-api.onrender.com";

export function getApiBase(): string {
  const configured = process.env.NEXT_PUBLIC_API_URL?.replace(/\/$/, "");

  if (typeof window === "undefined") {
    return configured ?? PRODUCTION_API;
  }

  const host = window.location.hostname;
  const onLocal = host === "localhost" || host === "127.0.0.1";

  if (onLocal) {
    return configured ?? "http://localhost:8000";
  }

  if (configured && !configured.includes("localhost") && !configured.includes("127.0.0.1")) {
    return configured;
  }

  return PRODUCTION_API;
}
