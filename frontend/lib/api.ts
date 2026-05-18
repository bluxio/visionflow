import { getApiBase } from "./api-base";
import { getClientId } from "./client-id";
import {
  canUseStorageUpload,
  getSupabaseBrowser,
  SUPABASE_STORAGE_BUCKET,
} from "./supabase-client";
import type {
  AnalyzeResponse,
  ExerciseType,
  HistoryDetail,
  HistoryItem,
  QuotaError,
} from "./types";

const CHUNK_SIZE = 2 * 1024 * 1024; // 2MB parts — reliable on mobile
const DIRECT_UPLOAD_MAX = 8 * 1024 * 1024;
const MAX_CHUNK_RETRIES = 3;

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
        "Upload interrupted. If this keeps happening, add Supabase keys on Vercel (see docs/DEPLOY.md) or use a smaller export from Photos.",
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

async function uploadChunkWithRetry(
  form: FormData,
  partLabel: string,
): Promise<void> {
  const headers = { "X-Client-Id": getClientId() };
  let lastError: Error | null = null;

  for (let attempt = 1; attempt <= MAX_CHUNK_RETRIES; attempt++) {
    try {
      const res = await apiFetch(
        "/upload-chunk",
        { method: "POST", headers, body: form },
        180_000,
      );
      if (!res.ok) {
        throw new Error(await parseError(res));
      }
      return;
    } catch (err) {
      lastError = err instanceof Error ? err : new Error(String(err));
      if (attempt < MAX_CHUNK_RETRIES) {
        await new Promise((r) => setTimeout(r, 1000 * attempt));
      }
    }
  }
  throw new Error(`${partLabel} failed after ${MAX_CHUNK_RETRIES} tries: ${lastError?.message}`);
}

async function uploadViaSupabase(
  file: File,
  exerciseType: ExerciseType,
  onProgress?: (message: string) => void,
): Promise<AnalyzeResponse> {
  const supabase = getSupabaseBrowser();
  if (!supabase) {
    throw new Error("Supabase storage is not configured in the frontend.");
  }

  const clientId = getClientId();
  const ext = file.name.includes(".") ? file.name.slice(file.name.lastIndexOf(".")) : ".mov";
  const storagePath = `${clientId}/${crypto.randomUUID()}${ext}`;

  onProgress?.(`Uploading ${Math.round(file.size / (1024 * 1024))}MB to cloud storage…`);

  const { error } = await supabase.storage.from(SUPABASE_STORAGE_BUCKET).upload(storagePath, file, {
    cacheControl: "3600",
    upsert: true,
  });

  if (error) {
    throw new Error(
      `Storage upload failed: ${error.message}. Run supabase/storage_setup.sql in your Supabase project.`,
    );
  }

  onProgress?.("Analyzing on server (this can take 1–2 minutes)…");

  const form = new FormData();
  form.append("storage_path", storagePath);
  form.append("exercise_type", exerciseType);

  const res = await apiFetch(
    "/analyze-storage",
    {
      method: "POST",
      headers: { "X-Client-Id": clientId },
      body: form,
    },
    600_000,
  );

  return parseAnalyzeResponse(res);
}

async function uploadChunked(
  file: File,
  exerciseType: ExerciseType,
  onProgress?: (message: string) => void,
): Promise<AnalyzeResponse> {
  const uploadId = crypto.randomUUID();
  const totalParts = Math.ceil(file.size / CHUNK_SIZE);

  for (let i = 0; i < totalParts; i++) {
    const label = `Part ${i + 1}/${totalParts}`;
    onProgress?.(`Uploading ${label}…`);

    const start = i * CHUNK_SIZE;
    const end = Math.min(start + CHUNK_SIZE, file.size);
    const blob = file.slice(start, end);

    const form = new FormData();
    form.append("upload_id", uploadId);
    form.append("part_index", String(i));
    form.append("total_parts", String(totalParts));
    form.append("chunk", blob, `part_${i}`);

    await uploadChunkWithRetry(form, label);
  }

  onProgress?.("Processing video on server…");

  const analyzeForm = new FormData();
  analyzeForm.append("upload_id", uploadId);
  analyzeForm.append("total_parts", String(totalParts));
  analyzeForm.append("filename", file.name);
  analyzeForm.append("exercise_type", exerciseType);

  const res = await apiFetch(
    "/analyze-assembled",
    {
      method: "POST",
      headers: { "X-Client-Id": getClientId() },
      body: analyzeForm,
    },
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

  if (canUseStorageUpload()) {
    return uploadViaSupabase(file, exerciseType, onProgress);
  }

  onProgress?.(
    `Uploading ${Math.round(file.size / (1024 * 1024))}MB in ${totalPartsLabel(file.size)} parts…`,
  );
  return uploadChunked(file, exerciseType, onProgress);
}

function totalPartsLabel(size: number) {
  return String(Math.ceil(size / CHUNK_SIZE));
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
