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

async function apiFetch(path: string, init?: RequestInit, timeoutMs?: number): Promise<Response> {
  const controller = timeoutMs ? new AbortController() : null;
  const timeout = controller
    ? setTimeout(() => controller.abort(), timeoutMs)
    : undefined;

  try {
    return await fetch(apiUrl(path), {
      ...init,
      signal: controller?.signal,
    });
  } catch (err) {
    if (err instanceof Error && err.name === "AbortError") {
      throw new Error("Request timed out. Try again in a moment.");
    }
    const msg = err instanceof Error ? err.message : "Network error";
    if (msg === "Load failed" || msg === "Failed to fetch") {
      throw new Error(
        "Connection lost. Large iPhone videos can take several minutes to upload — keep this tab open.",
      );
    }
    throw err;
  } finally {
    if (timeout) clearTimeout(timeout);
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

export async function wakeBackend(): Promise<void> {
  try {
    await apiFetch("/health", { method: "GET", cache: "no-store" }, 30_000);
  } catch {
    /* ignore */
  }
}

/** No client timeout — large MOV uploads can take many minutes. */
export async function analyzeUpload(
  file: File,
  exerciseType: ExerciseType,
): Promise<AnalyzeResponse> {
  await wakeBackend();

  const form = new FormData();
  form.append("file", file);
  form.append("exercise_type", exerciseType);

  const res = await apiFetch("/analyze-upload", {
    method: "POST",
    headers: { "X-Client-Id": getClientId() },
    body: form,
  });

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
  const res = await apiFetch(
    "/history",
    {
      headers: { "X-Client-Id": getClientId() },
      cache: "no-store",
    },
    30_000,
  );
  if (!res.ok) throw new Error(await parseError(res));
  return res.json() as Promise<HistoryItem[]>;
}

export async function fetchHistoryDetail(id: string): Promise<HistoryDetail> {
  const res = await apiFetch(
    `/history/${id}`,
    {
      headers: { "X-Client-Id": getClientId() },
      cache: "no-store",
    },
    30_000,
  );
  if (!res.ok) throw new Error(await parseError(res));
  return res.json() as Promise<HistoryDetail>;
}
