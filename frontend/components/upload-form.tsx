"use client";

import { canUseStorageUpload } from "@/lib/supabase-client";
import type { ExerciseType } from "@/lib/types";

const LARGE_FILE_BYTES = 8 * 1024 * 1024;

const EXERCISES: { value: ExerciseType; label: string }[] = [
  { value: "squat", label: "Squat (pose analysis)" },
  { value: "deadlift", label: "Deadlift" },
  { value: "bench_press", label: "Bench Press" },
  { value: "barbell_row", label: "Barbell Row" },
];

interface UploadFormProps {
  exerciseType: ExerciseType;
  onExerciseChange: (v: ExerciseType) => void;
  onFileChange: (file: File | null) => void;
  onAnalyze: () => void;
  loading: boolean;
  fileName: string | null;
  fileSizeBytes?: number;
}

export function UploadForm({
  exerciseType,
  onExerciseChange,
  onFileChange,
  onAnalyze,
  loading,
  fileName,
  fileSizeBytes,
}: UploadFormProps) {
  const needsSupabase =
    typeof fileSizeBytes === "number" &&
    fileSizeBytes > LARGE_FILE_BYTES &&
    !canUseStorageUpload();

  return (
    <section className="rounded-xl border border-[#1E2635] bg-[#111826] p-6">
      <h2 className="text-sm font-medium uppercase tracking-wider text-[#A0AEC0]">
        Analyze form
      </h2>
      <p className="mt-1 text-sm text-[#A0AEC0]">
        Phone videos are fine (up to 200MB). We analyze the first ~45 seconds only.
        Film from the side with your full body in frame.
      </p>

      <div className="mt-6 space-y-4">
        <label className="block">
          <span className="text-sm text-[#E2E8F0]">Exercise</span>
          <select
            value={exerciseType}
            onChange={(e) => onExerciseChange(e.target.value as ExerciseType)}
            className="mt-2 w-full rounded-xl border border-[#1E2635] bg-[#0A0F1A] px-4 py-3 text-[#E2E8F0] outline-none focus:border-[#33AFFF]"
            disabled={loading}
          >
            {EXERCISES.map((ex) => (
              <option key={ex.value} value={ex.value}>
                {ex.label}
              </option>
            ))}
          </select>
        </label>

        <label className="block">
          <span className="text-sm text-[#E2E8F0]">Video</span>
          <input
            type="file"
            accept="video/*"
            capture="environment"
            onChange={(e) => onFileChange(e.target.files?.[0] ?? null)}
            className="mt-2 w-full rounded-xl border border-dashed border-[#1E2635] bg-[#0A0F1A] px-4 py-6 text-sm text-[#A0AEC0] file:mr-4 file:rounded-lg file:border-0 file:bg-[#33AFFF] file:px-4 file:py-2 file:text-sm file:font-medium file:text-[#0A0F1A]"
            disabled={loading}
          />
          {fileName && (
            <p className="mt-2 truncate text-xs text-[#A0AEC0]">{fileName}</p>
          )}
          {needsSupabase && (
            <p className="mt-2 rounded-lg border border-amber-500/40 bg-amber-500/10 px-3 py-2 text-xs text-amber-200">
              This file is large. Production must have{" "}
              <code className="text-amber-100">NEXT_PUBLIC_SUPABASE_URL</code> and{" "}
              <code className="text-amber-100">NEXT_PUBLIC_SUPABASE_ANON_KEY</code> on Vercel
              (see docs/DEPLOY.md). Without them, upload to Render often fails on phone Wi‑Fi.
            </p>
          )}
        </label>

        <button
          type="button"
          onClick={onAnalyze}
          disabled={loading || !fileName}
          className="w-full rounded-xl bg-[#33AFFF] py-3 text-sm font-semibold text-[#0A0F1A] transition-opacity hover:opacity-90 disabled:cursor-not-allowed disabled:opacity-40"
        >
          {loading ? "Analyzing…" : "Analyze form"}
        </button>
      </div>
    </section>
  );
}
