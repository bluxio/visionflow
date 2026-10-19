"use client";

import { useState } from "react";
import type {
  AnalysisResult,
  CoverageSummary,
  FieldSeverity,
  RiskLevel,
} from "@/lib/types";

const COVERAGE_FIELDS: { key: keyof CoverageSummary; label: string }[] = [
  { key: "activeStatus", label: "Active status" },
  { key: "annualMax", label: "Annual max" },
  { key: "remainingBenefits", label: "Remaining benefits" },
  { key: "deductible", label: "Deductible" },
  { key: "preventivePercent", label: "Preventive %" },
  { key: "basicPercent", label: "Basic %" },
  { key: "majorPercent", label: "Major %" },
];

const SAMPLE_TEXT = `Member: Jane Doe DOB 04/12/1988
Plan: Delta Dental PPO — Group 44821
Eligibility: Active as of 01/01/2026
Benefit period: Calendar year
Annual maximum: $1,500 individual
Benefits used YTD: $420
Remaining: $1,080
Deductible: $50 individual — met
Preventive: 100% (no deductible)
Basic: 80%
Major: 50%
Waiting periods: not listed
Frequency: Prophy 2/year, BWX 1/year — last prophy unknown
Crowns: alternate benefit may apply
Missing tooth clause: not stated
Predetermination: recommended for services over $300
Network: in-network benefits shown`;

function riskStyles(level: RiskLevel) {
  switch (level) {
    case "low":
      return "bg-emerald-50 text-emerald-800 border-emerald-200";
    case "medium":
      return "bg-amber-50 text-amber-900 border-amber-200";
    case "high":
      return "bg-red-50 text-red-800 border-red-200";
  }
}

function severityStyles(severity: FieldSeverity) {
  switch (severity) {
    case "missing":
      return "bg-amber-50 border-amber-300 text-amber-950";
    case "ambiguous":
      return "bg-yellow-50 border-yellow-300 text-yellow-950";
    case "risky":
      return "bg-red-50 border-red-300 text-red-950";
  }
}

function displayValue(value: string | null) {
  return value?.trim() ? value : "—";
}

export function VerificationApp() {
  const [text, setText] = useState("");
  const [result, setResult] = useState<AnalysisResult | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [loading, setLoading] = useState(false);
  const [copied, setCopied] = useState(false);

  async function handleAnalyze() {
    setError(null);
    setCopied(false);
    setLoading(true);

    try {
      const response = await fetch("/api/analyze", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ text }),
      });

      const data = await response.json();
      if (!response.ok) {
        throw new Error(data.error || "Analysis failed");
      }

      setResult(data as AnalysisResult);
    } catch (err) {
      setResult(null);
      setError(err instanceof Error ? err.message : "Analysis failed");
    } finally {
      setLoading(false);
    }
  }

  async function handleCopy() {
    if (!result?.pmsPasteText) return;
    await navigator.clipboard.writeText(result.pmsPasteText);
    setCopied(true);
    window.setTimeout(() => setCopied(false), 2000);
  }

  return (
    <div className="mx-auto w-full max-w-3xl px-4 py-10 sm:px-6">
      <header className="mb-8">
        <p className="text-sm font-medium tracking-wide text-teal-700 uppercase">
          Dental ops demo
        </p>
        <h1 className="mt-1 text-3xl font-semibold tracking-tight text-zinc-900">
          Insurance Verification QA
        </h1>
        <p className="mt-2 max-w-2xl text-zinc-600">
          Paste messy portal text, eligibility responses, or phone notes. Get a
          structured benefits breakdown, missing-field flags, risk level, and a
          payer call checklist — ready to paste into your PMS.
        </p>
      </header>

      <section className="rounded-2xl border border-zinc-200 bg-white p-5 shadow-sm sm:p-6">
        <label
          htmlFor="insurance-text"
          className="block text-sm font-medium text-zinc-800"
        >
          Paste insurance info here (portal text, eligibility response, notes)
        </label>
        <textarea
          id="insurance-text"
          value={text}
          onChange={(e) => setText(e.target.value)}
          rows={12}
          placeholder="Paste eligibility text from the payer portal, phone notes, or benefits summary…"
          className="mt-2 w-full resize-y rounded-xl border border-zinc-300 bg-zinc-50 px-3 py-3 font-mono text-sm text-zinc-900 outline-none ring-teal-600/30 placeholder:text-zinc-400 focus:border-teal-600 focus:bg-white focus:ring-4"
        />

        <div className="mt-4 flex flex-wrap items-center gap-3">
          <button
            type="button"
            onClick={handleAnalyze}
            disabled={loading || !text.trim()}
            className="inline-flex items-center justify-center rounded-xl bg-teal-700 px-5 py-2.5 text-sm font-semibold text-white transition hover:bg-teal-800 disabled:cursor-not-allowed disabled:bg-zinc-300"
          >
            {loading ? "Analyzing…" : "Analyze Coverage"}
          </button>
          <button
            type="button"
            onClick={() => setText(SAMPLE_TEXT)}
            className="text-sm font-medium text-teal-700 hover:text-teal-900"
          >
            Load sample text
          </button>
        </div>

        {error && (
          <p className="mt-4 rounded-xl border border-red-200 bg-red-50 px-3 py-2 text-sm text-red-800">
            {error}
          </p>
        )}
      </section>

      {result && (
        <div className="mt-8 space-y-6">
          <div
            className={`rounded-2xl border px-4 py-3 ${riskStyles(result.riskLevel)}`}
          >
            <div className="flex flex-wrap items-baseline gap-x-3 gap-y-1">
              <span className="text-sm font-semibold tracking-wide uppercase">
                Risk level
              </span>
              <span className="text-lg font-bold capitalize">
                {result.riskLevel}
              </span>
            </div>
            {result.riskRationale && (
              <p className="mt-1 text-sm opacity-90">{result.riskRationale}</p>
            )}
          </div>

          <section className="rounded-2xl border border-zinc-200 bg-white p-5 shadow-sm sm:p-6">
            <h2 className="text-lg font-semibold text-zinc-900">
              1. Coverage Summary
            </h2>
            <dl className="mt-4 grid gap-3 sm:grid-cols-2">
              {COVERAGE_FIELDS.map(({ key, label }) => (
                <div
                  key={key}
                  className="rounded-xl border border-zinc-100 bg-zinc-50 px-3 py-3"
                >
                  <dt className="text-xs font-medium tracking-wide text-zinc-500 uppercase">
                    {label}
                  </dt>
                  <dd className="mt-1 text-sm font-medium text-zinc-900">
                    {displayValue(result.coverageSummary[key])}
                  </dd>
                </div>
              ))}
            </dl>
          </section>

          <section className="rounded-2xl border border-zinc-200 bg-white p-5 shadow-sm sm:p-6">
            <h2 className="text-lg font-semibold text-zinc-900">
              2. Missing &amp; Risky Fields
            </h2>
            {result.missingAndRiskyFields.length === 0 ? (
              <p className="mt-3 text-sm text-zinc-600">
                No material gaps flagged from this text.
              </p>
            ) : (
              <ul className="mt-4 space-y-3">
                {result.missingAndRiskyFields.map((item, index) => (
                  <li
                    key={`${item.field}-${index}`}
                    className={`rounded-xl border px-3 py-3 ${severityStyles(item.severity)}`}
                  >
                    <div className="flex flex-wrap items-center gap-2">
                      <span className="font-semibold">{item.field}</span>
                      <span className="rounded-full bg-white/70 px-2 py-0.5 text-xs font-medium tracking-wide uppercase">
                        {item.severity}
                      </span>
                    </div>
                    <p className="mt-1 text-sm">{item.detail}</p>
                  </li>
                ))}
              </ul>
            )}
          </section>

          <section className="rounded-2xl border border-zinc-200 bg-white p-5 shadow-sm sm:p-6">
            <h2 className="text-lg font-semibold text-zinc-900">
              3. Call Checklist
            </h2>
            {result.callChecklist.length === 0 ? (
              <p className="mt-3 text-sm text-zinc-600">
                No payer call needed based on this text.
              </p>
            ) : (
              <ol className="mt-4 list-decimal space-y-2 pl-5 text-sm text-zinc-800">
                {result.callChecklist.map((question, index) => (
                  <li key={index} className="pl-1 leading-relaxed">
                    {question}
                  </li>
                ))}
              </ol>
            )}
          </section>

          <section className="rounded-2xl border border-zinc-200 bg-white p-5 shadow-sm sm:p-6">
            <div className="flex flex-wrap items-center justify-between gap-3">
              <h2 className="text-lg font-semibold text-zinc-900">
                PMS paste
              </h2>
              <button
                type="button"
                onClick={handleCopy}
                className="inline-flex items-center justify-center rounded-xl border border-zinc-300 bg-white px-4 py-2 text-sm font-semibold text-zinc-800 transition hover:bg-zinc-50"
              >
                {copied ? "Copied" : "Copy to clipboard"}
              </button>
            </div>
            <pre className="mt-4 overflow-x-auto whitespace-pre-wrap rounded-xl border border-zinc-100 bg-zinc-50 p-4 font-mono text-xs leading-relaxed text-zinc-800">
              {result.pmsPasteText}
            </pre>
          </section>
        </div>
      )}
    </div>
  );
}
