/**
 * Resolve API base URL.
 * Production browser uses same-origin /wfc-api route handler (runtime proxy to Render).
 * Local dev talks to FastAPI on localhost:8000 directly.
 */
export function getApiBase(): string {
  const configured = process.env.NEXT_PUBLIC_API_URL?.replace(/\/$/, "");

  if (typeof window === "undefined") {
    return configured ?? "http://127.0.0.1:8000";
  }

  const host = window.location.hostname;
  const onLocal = host === "localhost" || host === "127.0.0.1";

  if (onLocal) {
    return configured ?? "http://localhost:8000";
  }

  // Prefer explicit production API URL when set (not localhost)
  if (configured && !configured.includes("localhost") && !configured.includes("127.0.0.1")) {
    return configured;
  }

  return "/wfc-api";
}
