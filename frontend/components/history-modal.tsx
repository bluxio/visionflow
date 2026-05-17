"use client";

import type { HistoryDetail } from "@/lib/types";

interface HistoryModalProps {
  detail: HistoryDetail | null;
  loading: boolean;
  onClose: () => void;
}

function severityColor(severity: string) {
  if (severity === "critical") return "text-red-400";
  if (severity === "warning") return "text-amber-400";
  return "text-emerald-400";
}

export function HistoryModal({ detail, loading, onClose }: HistoryModalProps) {
  if (!detail && !loading) return null;

  return (
    <div
      className="fixed inset-0 z-50 flex items-end justify-center bg-black/60 p-4 sm:items-center"
      onClick={onClose}
      role="presentation"
    >
      <div
        className="max-h-[85vh] w-full max-w-lg overflow-y-auto rounded-xl border border-[#1E2635] bg-[#111826] p-6 shadow-xl"
        onClick={(e) => e.stopPropagation()}
        role="dialog"
        aria-modal="true"
      >
        <div className="flex items-start justify-between gap-4">
          <h2 className="text-lg font-semibold text-[#E2E8F0]">Analysis details</h2>
          <button
            type="button"
            onClick={onClose}
            className="rounded-lg px-2 py-1 text-sm text-[#A0AEC0] hover:bg-[#1E2635] hover:text-[#E2E8F0]"
          >
            Close
          </button>
        </div>

        {loading && <p className="mt-6 text-sm text-[#A0AEC0]">Loading…</p>}

        {detail && !loading && (
          <>
            <p className="mt-2 capitalize text-[#A0AEC0]">
              {detail.exercise_type.replace(/_/g, " ")} · score{" "}
              <span className="text-[#33AFFF]">{detail.overall_score}</span> ·{" "}
              {detail.rep_count} reps
            </p>

            <ul className="mt-6 space-y-3">
              {detail.feedback.map((item) => (
                <li
                  key={item.aspect}
                  className="rounded-xl border border-[#1E2635] bg-[#0A0F1A] p-4"
                >
                  <div className="flex justify-between">
                    <span className="text-sm font-medium capitalize text-[#E2E8F0]">
                      {item.aspect.replace(/_/g, " ")}
                    </span>
                    <span
                      className={`text-sm font-semibold ${severityColor(item.severity)}`}
                    >
                      {item.score}
                    </span>
                  </div>
                  <p className="mt-2 text-sm text-[#A0AEC0]">{item.feedback}</p>
                </li>
              ))}
            </ul>

            <ul className="mt-6 list-inside list-disc space-y-1 text-sm text-[#E2E8F0]">
              {detail.recommendations.map((rec) => (
                <li key={rec}>{rec}</li>
              ))}
            </ul>
          </>
        )}
      </div>
    </div>
  );
}
