"use client";

interface QuotaCardProps {
  message: string;
  limit: number;
  windowHours: number;
}

export function QuotaExceededCard({ message, limit, windowHours }: QuotaCardProps) {
  return (
    <div className="rounded-xl border border-amber-500/30 bg-amber-500/10 p-6">
      <h3 className="text-sm font-semibold text-amber-400">Daily limit reached</h3>
      <p className="mt-2 text-sm text-[#E2E8F0]">{message}</p>
      <p className="mt-2 text-xs text-[#A0AEC0]">
        {limit} analyses per {windowHours}h on the free plan.
      </p>
    </div>
  );
}

export function AnalysisErrorCard({ message }: { message: string }) {
  return (
    <div className="rounded-xl border border-red-500/30 bg-red-500/10 p-6">
      <h3 className="text-sm font-semibold text-red-400">Analysis failed</h3>
      <p className="mt-2 text-sm text-[#E2E8F0]">{message}</p>
    </div>
  );
}
