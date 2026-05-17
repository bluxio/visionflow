"use client";

import type { ExerciseType } from "@/lib/types";

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
}

export function UploadForm({
  exerciseType,
  onExerciseChange,
  onFileChange,
  onAnalyze,
  loading,
  fileName,
}: UploadFormProps) {
  return (
    <section className="rounded-xl border border-[#1E2635] bg-[#111826] p-6">
      <h2 className="text-sm font-medium uppercase tracking-wider text-[#A0AEC0]">
        Analyze form
      </h2>
      <p className="mt-1 text-sm text-[#A0AEC0]">
        Side-angle clip, under 30 seconds and 25MB. Squats use MediaPipe on the server.
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
