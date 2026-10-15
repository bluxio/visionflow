"use client";

import Link from "next/link";
import { useRouter } from "next/navigation";
import { useEffect, useMemo, useState } from "react";

import { AppShell } from "@/components/layout/app-shell";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { Textarea } from "@/components/ui/textarea";
import { buildCloseoutExtraction } from "@/lib/seed-inspections";
import {
  ensureSeedInspections,
  getLocalInspection,
  upsertLocalInspection,
} from "@/lib/inspections-storage";
import { PAGE_GUTTER } from "@/lib/layout";
import { cn } from "@/lib/utils";
import type {
  ClarificationAnswer,
  CloseoutGap,
  Inspection,
} from "@/types/inspection";

type AnswerState = {
  optionId: string | null;
  note: string;
};

function isAnswerComplete(gap: CloseoutGap, state: AnswerState | undefined): boolean {
  if (!state?.optionId) return false;
  const option = gap.options.find((o) => o.id === state.optionId);
  if (!option) return false;
  if (option.id === "note") return state.note.trim().length > 0;
  if (option.id === "part") return state.note.trim().length > 0;
  return true;
}

function GapCard({
  gap,
  indexLabel,
  state,
  onSelect,
  onNote,
}: {
  gap: CloseoutGap;
  indexLabel: string;
  state: AnswerState;
  onSelect: (optionId: string) => void;
  onNote: (note: string) => void;
}) {
  const selected = gap.options.find((o) => o.id === state.optionId);
  const needsNote = selected?.id === "note" || selected?.id === "part";
  const noteMissing = needsNote && !state.note.trim();

  return (
    <section
      className={cn(
        "rounded-lg border bg-card p-4 shadow-[0_1px_0_rgba(16,42,67,0.04)]",
        state.optionId && !noteMissing
          ? "border-primary/35"
          : noteMissing
            ? "border-warning/40"
            : "border-border",
      )}
    >
      <p className="text-[11px] font-semibold tracking-[0.08em] text-muted-foreground uppercase">
        {indexLabel}
      </p>
      <h2 className="mt-1.5 text-[15px] font-semibold leading-snug text-navy">
        {gap.question}
      </h2>
      <p className="mt-1.5 text-xs leading-relaxed text-muted-foreground">
        {gap.why}
      </p>

      <div className="mt-3.5 grid gap-2">
        {gap.options.map((opt) => (
          <button
            key={opt.id}
            type="button"
            onClick={() => onSelect(opt.id)}
            className={cn(
              "min-h-11 rounded-md border px-3 py-2.5 text-left text-sm font-medium transition-colors",
              state.optionId === opt.id
                ? "border-primary bg-primary/[0.07] text-navy"
                : "border-border bg-background hover:border-border hover:bg-muted/50",
            )}
          >
            {opt.label}
          </button>
        ))}
      </div>

      {needsNote ? (
        <div className="mt-3 space-y-1.5">
          <Label className="text-xs font-medium text-foreground">
            {selected?.id === "part" ? "Part number (required)" : "Note (required)"}
          </Label>
          {selected?.id === "part" ? (
            <Input
              value={state.note}
              onChange={(e) => onNote(e.target.value)}
              placeholder="Enter part number"
              className="rounded-md font-mono"
              aria-invalid={noteMissing}
            />
          ) : (
            <Textarea
              value={state.note}
              onChange={(e) => onNote(e.target.value)}
              placeholder="Describe what you observed or still need confirmed"
              className="min-h-[84px] rounded-md text-sm"
              aria-invalid={noteMissing}
            />
          )}
          {noteMissing ? (
            <p className="text-xs text-warning">
              Enter text before continuing — “Add note” is not an answer by itself.
            </p>
          ) : null}
        </div>
      ) : null}

      {selected?.id === "not-tested" ? (
        <p className="mt-3 rounded-md border border-warning/30 bg-warning/5 px-2.5 py-2 text-xs leading-relaxed text-foreground">
          Information gap closed. Phase II still requires a completed test or
          manager follow-up — it is not marked as passed.
        </p>
      ) : null}
    </section>
  );
}

export function GapResolution({ id }: { id: string }) {
  const router = useRouter();
  const [job, setJob] = useState<Inspection | null>(null);
  const [answers, setAnswers] = useState<Record<string, AnswerState>>({});

  useEffect(() => {
    ensureSeedInspections();
    const row = getLocalInspection(id);
    setJob(row);
    if (row?.closeoutGaps) {
      const init: Record<string, AnswerState> = {};
      for (const g of row.closeoutGaps) {
        init[g.id] = { optionId: null, note: "" };
      }
      setAnswers(init);
    }
  }, [id]);

  const gaps: CloseoutGap[] = job?.closeoutGaps ?? [];
  const canResolve = gaps.filter(
    (g) => (g.kind ?? "can_resolve") !== "requires_action",
  );
  const requiresAction = gaps.filter((g) => g.kind === "requires_action");

  const canContinue = useMemo(() => {
    if (gaps.length === 0) return false;
    return gaps.every((g) => isAnswerComplete(g, answers[g.id]));
  }, [gaps, answers]);

  const answeredCount = useMemo(() => {
    return gaps.filter((g) => isAnswerComplete(g, answers[g.id])).length;
  }, [gaps, answers]);

  function selectOption(gapId: string, optionId: string) {
    setAnswers((prev) => ({
      ...prev,
      [gapId]: {
        optionId,
        note: optionId === prev[gapId]?.optionId ? prev[gapId].note : "",
      },
    }));
  }

  function finish() {
    if (!job || !canContinue) return;

    const clarifications: ClarificationAnswer[] = gaps.map((gap) => {
      const state = answers[gap.id];
      const option = gap.options.find((o) => o.id === state.optionId)!;
      const note = state.note.trim();

      if (option.id === "note") {
        return {
          gapId: gap.id,
          question: gap.question,
          answerLabel: note,
          note,
          unresolved: true,
        };
      }

      if (option.id === "part") {
        return {
          gapId: gap.id,
          question: gap.question,
          answerLabel: `Part number: ${note}`,
          note,
          unresolved: false,
        };
      }

      return {
        gapId: gap.id,
        question: gap.question,
        answerLabel: option.label,
        note: note || undefined,
        unresolved:
          Boolean(option.unresolved) ||
          option.id === "unknown" ||
          option.id === "unable" ||
          option.id === "not-tested" ||
          option.id === "photo",
      };
    });

    const extraction = buildCloseoutExtraction(job, clarifications);
    const openFollowUps = extraction.recommendedNextActions.length;

    const next: Inspection = {
      ...job,
      status: "Ready for review",
      clarifications,
      extraction,
      extractionSource: "mock",
      unresolvedForManager: openFollowUps,
    };
    upsertLocalInspection(next);
    router.push(`/jobs/${job.id}/ready`);
  }

  if (!job) {
    return (
      <AppShell>
        <div
          className={cn(PAGE_GUTTER, "max-w-lg py-16 text-sm text-muted-foreground")}
        >
          Job not found.
        </div>
      </AppShell>
    );
  }

  if (gaps.length === 0) {
    return (
      <AppShell>
        <div className={cn(PAGE_GUTTER, "max-w-lg space-y-4 py-16")}>
          <p className="text-sm">No clarification items for this closeout.</p>
          <Link
            href={`/jobs/${job.id}/closeout`}
            className="text-sm text-primary underline"
          >
            Back to capture
          </Link>
        </div>
      </AppShell>
    );
  }

  return (
    <AppShell>
      <div className={cn(PAGE_GUTTER, "max-w-lg space-y-6 pt-7 pb-12")}>
        <div className="space-y-2">
          <p className="text-[11px] font-semibold tracking-[0.1em] text-warning uppercase">
            Before you leave
          </p>
          <h1 className="text-[1.65rem] font-semibold tracking-tight text-navy">
            Finish these before you leave
          </h1>
          <p className="text-sm leading-relaxed text-muted-foreground">
            {canResolve.length > 0
              ? `${canResolve.length} missing detail${canResolve.length === 1 ? "" : "s"} you can resolve on site`
              : "On-site questions"}
            {requiresAction.length > 0
              ? ` · ${requiresAction.length} item${requiresAction.length === 1 ? "" : "s"} that may require follow-up action`
              : ""}
            .
          </p>
          <p className="text-sm font-medium tabular text-foreground">
            {answeredCount} of {gaps.length} answered
            {canContinue ? " · Ready to continue" : ""}
          </p>
        </div>

        {canResolve.length > 0 ? (
          <div className="space-y-3">
            <h2 className="text-[11px] font-semibold tracking-[0.1em] text-muted-foreground uppercase">
              Can resolve now
            </h2>
            {canResolve.map((gap, i) => (
              <GapCard
                key={gap.id}
                gap={gap}
                indexLabel={`${i + 1}`}
                state={answers[gap.id] ?? { optionId: null, note: "" }}
                onSelect={(optionId) => selectOption(gap.id, optionId)}
                onNote={(note) =>
                  setAnswers((prev) => ({
                    ...prev,
                    [gap.id]: { ...prev[gap.id], note },
                  }))
                }
              />
            ))}
          </div>
        ) : null}

        {requiresAction.length > 0 ? (
          <div className="space-y-3">
            <h2 className="text-[11px] font-semibold tracking-[0.1em] text-warning uppercase">
              Requires action
            </h2>
            <p className="text-xs leading-relaxed text-muted-foreground">
              Confirming status closes the information gap. It does not invent a
              completed inspection step.
            </p>
            {requiresAction.map((gap, i) => (
              <GapCard
                key={gap.id}
                gap={gap}
                indexLabel={`${i + 1}`}
                state={answers[gap.id] ?? { optionId: null, note: "" }}
                onSelect={(optionId) => selectOption(gap.id, optionId)}
                onNote={(note) =>
                  setAnswers((prev) => ({
                    ...prev,
                    [gap.id]: { ...prev[gap.id], note },
                  }))
                }
              />
            ))}
          </div>
        ) : null}

        <Button
          type="button"
          size="lg"
          className="h-12 w-full rounded-md text-base"
          disabled={!canContinue}
          onClick={finish}
        >
          Continue to closeout summary
        </Button>
      </div>
    </AppShell>
  );
}
