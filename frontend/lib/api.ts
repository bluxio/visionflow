import { getApiBase } from "./api-base";
import { getClientId } from "./client-id";
import type {
  AnalyzeResponse,
  ExerciseType,
  HistoryDetail,
  HistoryItem,
  QuotaError,
} from "./types";

const CHUNK_SIZE = 4 * 1024 * 1024; // 4MB — avoids proxy/load-balancer timeouts
const DIRECT_UPLOAD_MAX = 8 * 1024 * 1024; // 8MB

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
        "Upload interrupted. Stay on Wi‑Fi, keep this tab open, and try again.",
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

async function parseAnalyzeResponse(res: Response): Promise<AnalyzeResponse> {
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

export async function wakeBackend(): Promise<void> {
  try {
    await apiFetch("/health", { method: "GET", cache: "no-store" }, 30_000);
  } catch {
    /* ignore */
  }
}

async function uploadChunked(
  file: File,
  exerciseType: ExerciseType,
  onProgress?: (message: string) => void,
): Promise<AnalyzeResponse> {
  const uploadId = crypto.randomUUID();
  const totalParts = Math.ceil(file.size / CHUNK_SIZE);
  const headers = { "X-Client-Id": getClientId() };

  for (let i = 0; i < totalParts; i++) {
    onProgress?.(`Uploading part ${i + 1} of ${totalParts}…`);
    const start = i * CHUNK_SIZE;
    const end = Math.min(start + CHUNK_SIZE, file.size);
    const blob = file.slice(start, end);

    const form = new FormData();
    form.append("upload_id", uploadId);
    form.append("part_index", String(i));
    form.append("total_parts", String(totalParts));
    form.append("chunk", blob, `part_${i}`);

    const res = await apiFetch(
      "/upload-chunk",
      { method: "POST", headers, body: form },
      120_000,
    );
    if (!res.ok) {
      throw new Error(await parseError(res));
    }
  }

  onProgress?.("Processing video on server…");

  const analyzeForm = new FormData();
  analyzeForm.append("upload_id", uploadId);
  analyzeForm.append("total_parts", String(totalParts));
  analyzeForm.append("filename", file.name);
  analyzeForm.append("exercise_type", exerciseType);

  const res = await apiFetch(
    "/analyze-assembled",
    { method: "POST", headers, body: analyzeForm },
    600_000,
  );
  return parseAnalyzeResponse(res);
}

async function uploadDirect(file: File, exerciseType: ExerciseType): Promise<AnalyzeResponse> {
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
    600_000,
  );
  return parseAnalyzeResponse(res);
}

export async function analyzeUpload(
  file: File,
  exerciseType: ExerciseType,
  onProgress?: (message: string) => void,
): Promise<AnalyzeResponse> {
  await wakeBackend();

  if (file.size <= DIRECT_UPLOAD_MAX) {
    onProgress?.("Uploading and analyzing…");
    return uploadDirect(file, exerciseType);
  }

  onProgress?.(
    `Large file (${Math.round(file.size / (1024 * 1024))}MB) — uploading in ${Math.ceil(file.size / CHUNK_SIZE)} parts…`,
  );
  return uploadChunked(file, exerciseType, onProgress);
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
