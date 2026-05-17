"use client";

import type { HistoryItem } from "@/lib/types";

function formatDate(iso: string) {
  return new Date(iso).toLocaleString(undefined, {
    month: "short",
    day: "numeric",
    hour: "2-digit",
    minute: "2-digit",
  });
}

function severityDot(severity: string) {
  if (severity === "critical") return "bg-red-400";
  if (severity === "warning") return "bg-amber-400";
  return "bg-emerald-400";
}

interface HistoryPanelProps {
  items: HistoryItem[];
  loading: boolean;
  onSelect: (id: string) => void;
}

export function HistoryPanel({ items, loading, onSelect }: HistoryPanelProps) {
  return (
    <section className="rounded-xl border border-[#1E2635] bg-[#111826] p-6">
      <h2 className="text-sm font-medium uppercase tracking-wider text-[#A0AEC0]">
        Recent analyses
      </h2>

      {loading && (
        <p className="mt-4 text-sm text-[#A0AEC0]">Loading history…</p>
      )}

      {!loading && items.length === 0 && (
        <p className="mt-4 text-sm text-[#A0AEC0]">
          No analyses yet. Upload a video to get started.
        </p>
      )}

      <ul className="mt-4 space-y-2">
        {items.map((item) => (
          <li key={item.id}>
            <button
              type="button"
              onClick={() => onSelect(item.id)}
              className="flex w-full items-center gap-3 rounded-xl border border-[#1E2635] bg-[#0A0F1A] px-4 py-3 text-left transition-colors hover:border-[#33AFFF]/40"
            >
              <span
                className={`h-2 w-2 shrink-0 rounded-full ${severityDot(item.severity_max)}`}
              />
              <div className="min-w-0 flex-1">
                <p className="truncate text-sm font-medium capitalize text-[#E2E8F0]">
                  {item.exercise_type.replace(/_/g, " ")}
                </p>
                <p className="text-xs text-[#A0AEC0]">{formatDate(item.created_at)}</p>
              </div>
              <div className="text-right">
                <p className="text-sm font-semibold tabular-nums text-[#33AFFF]">
                  {item.overall_score}
                </p>
                <p className="text-xs text-[#A0AEC0]">{item.rep_count} reps</p>
              </div>
            </button>
          </li>
        ))}
      </ul>
    </section>
  );
}
