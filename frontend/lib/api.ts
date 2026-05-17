import { getApiBase } from "./api-base";
import { getClientId } from "./client-id";
import type {
  AnalyzeResponse,
  ExerciseType,
  HistoryDetail,
  HistoryItem,
  QuotaError,
} from "./types";

function apiUrl(path: string): string {
  const base = getApiBase().replace(/\/$/, "");
  return `${base}${path.startsWith("/") ? path : `/${path}`}`;
}

const ANALYZE_TIMEOUT_MS = 300_000; // 5 min — Render free tier cold start + CV

async function apiFetch(path: string, init?: RequestInit, timeoutMs = 60_000): Promise<Response> {
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), timeoutMs);

  try {
    return await fetch(apiUrl(path), { ...init, signal: controller.signal });
  } catch (err) {
    if (err instanceof Error && err.name === "AbortError") {
      throw new Error(
        "Request timed out. Use a shorter video (under 30s) and try again after /health responds.",
      );
    }
    const msg = err instanceof Error ? err.message : "Network error";
    if (msg === "Load failed" || msg === "Failed to fetch") {
      throw new Error(
        "Connection lost during analysis. The server may have restarted (out of memory). " +
          "Try a shorter video (under 30s, under 25MB) and wait for /health to return OK first.",
      );
    }
    throw err;
  } finally {
    clearTimeout(timeout);
  }
}

async function parseError(res: Response): Promise<string> {
  try {
    const data = await res.json();
    if (typeof data.detail === "string") return data.detail;
    if (data.detail?.error?.message) return data.detail.error.message;
    if (data.detail?.error) return JSON.stringify(data.detail.error);
    return res.statusText;
  } catch {
    return res.statusText;
  }
}

/** Wake Render free tier before a long analyze-upload request. */
export async function wakeBackend(): Promise<void> {
  try {
    await fetch(apiUrl("/health"), { method: "GET", cache: "no-store" });
  } catch {
    /* ignore — analyze may still work if instance is already warm */
  }
}

export async function analyzeUpload(
  file: File,
  exerciseType: ExerciseType,
): Promise<AnalyzeResponse> {
  await wakeBackend();

  const form = new FormData();
  form.append("file", file);
  form.append("exercise_type", exerciseType);

  const res = await apiFetch(
    "/analyze-upload",
    {
      method: "POST",
      headers: { "X-Client-Id": getClientId() },
      body: form,
    },
    ANALYZE_TIMEOUT_MS,
  );

  if (res.status === 429) {
    const body = await res.json();
    const err = (body.detail?.error ?? body.error) as QuotaError["error"] | undefined;
    throw Object.assign(new Error(err?.message ?? "Quota exceeded"), {
      quota: err,
    });
  }

  if (!res.ok) {
    throw new Error(await parseError(res));
  }

  return res.json() as Promise<AnalyzeResponse>;
}

export async function fetchHistory(): Promise<HistoryItem[]> {
  const res = await apiFetch("/history", {
    headers: { "X-Client-Id": getClientId() },
    cache: "no-store",
  });
  if (!res.ok) throw new Error(await parseError(res));
  return res.json() as Promise<HistoryItem[]>;
}

export async function fetchHistoryDetail(id: string): Promise<HistoryDetail> {
  const res = await apiFetch(`/history/${id}`, {
    headers: { "X-Client-Id": getClientId() },
    cache: "no-store",
  });
  if (!res.ok) throw new Error(await parseError(res));
  return res.json() as Promise<HistoryDetail>;
}
