"use client";

import type { AnalyzeResponse } from "@/lib/types";

function severityColor(severity: string) {
  if (severity === "critical") return "text-red-400";
  if (severity === "warning") return "text-amber-400";
  return "text-emerald-400";
}

interface AnalysisResultProps {
  result: AnalyzeResponse;
}

export function AnalysisResult({ result }: AnalysisResultProps) {
  return (
    <section className="rounded-xl border border-[#1E2635] bg-[#111826] p-6">
      <div className="flex items-start justify-between gap-4">
        <div>
          <h2 className="text-sm font-medium uppercase tracking-wider text-[#A0AEC0]">
            Results
          </h2>
          <p className="mt-1 capitalize text-[#E2E8F0]">
            {result.exercise_type.replace(/_/g, " ")}
          </p>
        </div>
        <div className="text-right">
          <p className="text-3xl font-semibold tabular-nums text-[#33AFFF]">
            {result.overall_score}
          </p>
          <p className="text-xs text-[#A0AEC0]">overall score</p>
        </div>
      </div>

      <p className="mt-4 text-sm text-[#A0AEC0]">
        <span className="text-[#E2E8F0]">{result.rep_count}</span> reps detected
      </p>

      <ul className="mt-6 space-y-3">
        {result.feedback.map((item) => (
          <li
            key={item.aspect}
            className="rounded-xl border border-[#1E2635] bg-[#0A0F1A] p-4"
          >
            <div className="flex items-center justify-between">
              <span className="text-sm font-medium capitalize text-[#E2E8F0]">
                {item.aspect.replace(/_/g, " ")}
              </span>
              <span
                className={`text-sm font-semibold tabular-nums ${severityColor(item.severity)}`}
              >
                {item.score}
              </span>
            </div>
            <p className="mt-2 text-sm text-[#A0AEC0]">{item.feedback}</p>
          </li>
        ))}
      </ul>

      {result.recommendations.length > 0 && (
        <div className="mt-6">
          <h3 className="text-xs font-medium uppercase tracking-wider text-[#A0AEC0]">
            Recommendations
          </h3>
          <ul className="mt-2 list-inside list-disc space-y-1 text-sm text-[#E2E8F0]">
            {result.recommendations.map((rec) => (
              <li key={rec}>{rec}</li>
            ))}
          </ul>
        </div>
      )}
    </section>
  );
}
