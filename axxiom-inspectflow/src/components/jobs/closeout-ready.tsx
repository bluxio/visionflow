"use client";

import { useRouter } from "next/navigation";
import { useEffect, useState } from "react";

import { AppShell } from "@/components/layout/app-shell";
import { Button } from "@/components/ui/button";
import {
  ensureSeedInspections,
  getLocalInspection,
  upsertLocalInspection,
} from "@/lib/inspections-storage";
import { PAGE_GUTTER } from "@/lib/layout";
import { cn } from "@/lib/utils";
import type { Inspection } from "@/types/inspection";

function followUpLabel(count: number): string {
  if (count === 1) return "1 follow-up action remains open.";
  return `${count} follow-up actions remain open.`;
}

export function CloseoutReady({ id }: { id: string }) {
  const router = useRouter();
  const [job, setJob] = useState<Inspection | null>(null);

  useEffect(() => {
    ensureSeedInspections();
    setJob(getLocalInspection(id));
  }, [id]);

  function submit() {
    if (!job) return;
    const next: Inspection = {
      ...job,
      status: "Ready for review",
    };
    upsertLocalInspection(next);
    router.push(`/inspections/${job.id}`);
  }

  if (!job || !job.extraction) {
    return (
      <AppShell>
        <div
          className={cn(PAGE_GUTTER, "max-w-lg py-16 text-sm text-muted-foreground")}
        >
          Closeout not ready yet.
        </div>
      </AppShell>
    );
  }

  const followUps = job.extraction.recommendedNextActions;
  const followUpCount = followUps.length;
  const findings = job.extraction.findings;
  const parts = job.extraction.partsMentioned.filter(
    (p) => !/^none confirmed$/i.test(p),
  );

  return (
    <AppShell>
      <div className={cn(PAGE_GUTTER, "max-w-lg space-y-6 pt-7 pb-12")}>
        <div className="space-y-2">
          <p className="text-[11px] font-semibold tracking-[0.1em] text-primary uppercase">
            Field closeout
          </p>
          <h1 className="text-[1.65rem] font-semibold tracking-tight text-navy">
            Closeout captured
          </h1>
          <p className="text-sm leading-relaxed text-muted-foreground">
            All on-site questions were answered. {followUpLabel(followUpCount)}
          </p>
        </div>

        <section className="space-y-2">
          <h2 className="text-[11px] font-semibold tracking-[0.1em] text-muted-foreground uppercase">
            Findings
          </h2>
          <ul className="space-y-2 rounded-lg border border-border bg-card p-4 text-sm shadow-[0_1px_0_rgba(16,42,67,0.04)]">
            {findings.map((f) => (
              <li key={f.issue} className="leading-snug text-foreground">
                · {f.issue}
              </li>
            ))}
          </ul>
        </section>

        <section className="space-y-2">
          <h2 className="text-[11px] font-semibold tracking-[0.1em] text-muted-foreground uppercase">
            Follow-up
          </h2>
          <ul className="space-y-2 rounded-lg border border-warning/25 bg-warning/[0.04] p-4 text-sm">
            {followUps.map((a) => (
              <li key={a} className="leading-snug">
                · {a}
              </li>
            ))}
          </ul>
        </section>

        <section className="space-y-2">
          <h2 className="text-[11px] font-semibold tracking-[0.1em] text-muted-foreground uppercase">
            Parts
          </h2>
          <ul className="space-y-2 rounded-lg border border-border bg-card p-4 text-sm shadow-[0_1px_0_rgba(16,42,67,0.04)]">
            {parts.map((p) => (
              <li key={p} className="leading-snug">
                · {p}
              </li>
            ))}
          </ul>
        </section>

        <section className="space-y-2">
          <h2 className="text-[11px] font-semibold tracking-[0.1em] text-muted-foreground uppercase">
            Evidence
          </h2>
          <div className="space-y-4 rounded-lg border border-border bg-card p-4 text-sm shadow-[0_1px_0_rgba(16,42,67,0.04)]">
            <div>
              <p className="text-[11px] font-medium tracking-[0.08em] text-muted-foreground uppercase">
                Original narration
              </p>
              <p className="mt-1.5 whitespace-pre-wrap font-mono text-xs leading-relaxed text-foreground">
                {job.notes}
              </p>
            </div>
            {job.photoNames && job.photoNames.length > 0 ? (
              <div>
                <p className="text-[11px] font-medium tracking-[0.08em] text-muted-foreground uppercase">
                  Photos
                </p>
                <ul className="mt-1.5 text-xs text-muted-foreground">
                  {job.photoNames.map((n) => (
                    <li key={n}>· {n}</li>
                  ))}
                </ul>
              </div>
            ) : null}
            <div>
              <p className="text-[11px] font-medium tracking-[0.08em] text-muted-foreground uppercase">
                Clarification answers
              </p>
              <ul className="mt-1.5 space-y-2.5">
                {(job.clarifications ?? []).map((c) => (
                  <li key={c.gapId}>
                    <span className="text-muted-foreground">{c.question}</span>
                    <br />
                    <span className="font-medium text-foreground">
                      {c.answerLabel}
                      {c.unresolved ? " · unresolved" : ""}
                    </span>
                  </li>
                ))}
              </ul>
            </div>
          </div>
        </section>

        <Button
          type="button"
          size="lg"
          className="h-12 w-full rounded-md text-base"
          onClick={submit}
        >
          Submit closeout
        </Button>
      </div>
    </AppShell>
  );
}
