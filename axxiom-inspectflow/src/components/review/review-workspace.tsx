"use client";

import Link from "next/link";
import { useCallback, useEffect, useMemo, useState } from "react";

import { AppShell } from "@/components/layout/app-shell";
import { Button, buttonVariants } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Textarea } from "@/components/ui/textarea";
import { generateDrafts } from "@/lib/drafts";
import { parseSiteParts } from "@/lib/finding-ui";
import {
  approveInspection,
  fetchInspection,
  generateInspectionDrafts,
  saveInspectionExtraction,
} from "@/lib/inspections-api-client";
import {
  ensureSeedInspections,
  getLocalInspection,
  upsertLocalInspection,
} from "@/lib/inspections-storage";
import { PAGE_GUTTER } from "@/lib/layout";
import { cn } from "@/lib/utils";
import type {
  ExtractionResult,
  Finding,
  Inspection,
  SeverityLevel,
} from "@/types/inspection";
import { SEVERITY_LEVELS } from "@/types/inspection";
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select";

const dateFmt = new Intl.DateTimeFormat(undefined, {
  dateStyle: "medium",
  timeStyle: "short",
});

type FindingRow = Finding & { key: string };

function FindingCard({
  finding,
  canEdit,
  editing,
  onEdit,
  onConfirm,
  onDismiss,
  onChange,
}: {
  finding: FindingRow;
  canEdit: boolean;
  editing: boolean;
  onEdit: () => void;
  onConfirm: () => void;
  onDismiss: () => void;
  onChange: (patch: Partial<Finding>) => void;
}) {
  if (editing && canEdit) {
    return (
      <div className="space-y-2 rounded-md border border-primary/30 bg-card p-3">
        <Input
          value={finding.issue}
          onChange={(e) => onChange({ issue: e.target.value })}
          className="rounded-md"
        />
        <Select
          value={finding.severity}
          onValueChange={(v) => {
            if (v) onChange({ severity: v as SeverityLevel });
          }}
        >
          <SelectTrigger className="w-full rounded-md">
            <SelectValue />
          </SelectTrigger>
          <SelectContent>
            {SEVERITY_LEVELS.map((s) => (
              <SelectItem key={s} value={s}>
                {s}
              </SelectItem>
            ))}
          </SelectContent>
        </Select>
        <Textarea
          value={finding.evidenceFromNotes}
          onChange={(e) => onChange({ evidenceFromNotes: e.target.value })}
          className="min-h-[64px] rounded-md text-xs"
        />
        <Button type="button" size="sm" onClick={onConfirm}>
          Done
        </Button>
      </div>
    );
  }

  return (
    <div className="rounded-md border border-border bg-card p-3">
      <div className="flex flex-wrap items-start justify-between gap-2">
        <div>
          <p className="text-sm font-medium">{finding.issue}</p>
          <p className="mt-1 text-[11px] font-medium capitalize text-muted-foreground">
            {finding.severity}
          </p>
        </div>
        {canEdit ? (
          <div className="flex gap-1">
            <Button type="button" size="xs" variant="ghost" onClick={onEdit}>
              Edit
            </Button>
            <Button type="button" size="xs" variant="ghost" onClick={onConfirm}>
              Confirm
            </Button>
            <Button type="button" size="xs" variant="ghost" onClick={onDismiss}>
              Dismiss
            </Button>
          </div>
        ) : null}
      </div>
      {finding.evidenceFromNotes ? (
        <p className="mt-2 border-l-2 border-border pl-2 text-xs text-muted-foreground">
          “{finding.evidenceFromNotes}”
        </p>
      ) : null}
    </div>
  );
}

export function ReviewWorkspace({ id }: { id: string }) {
  const [inspection, setInspection] = useState<Inspection | null>(null);
  const [loading, setLoading] = useState(true);
  const [notFound, setNotFound] = useState(false);
  const [busy, setBusy] = useState<string | null>(null);
  const [message, setMessage] = useState<string | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [findings, setFindings] = useState<FindingRow[]>([]);
  const [editingKey, setEditingKey] = useState<string | null>(null);

  const hydrateFrom = useCallback((row: Inspection) => {
    setInspection(row);
    setFindings(
      (row.extraction?.findings ?? []).map((f, i) => ({
        ...f,
        key: `${row.id}-f-${i}`,
      })),
    );
    setEditingKey(null);
  }, []);

  useEffect(() => {
    let cancelled = false;
    async function load() {
      setLoading(true);
      ensureSeedInspections();
      try {
        const remote = await fetchInspection(id);
        if (cancelled) return;
        if (remote) {
          upsertLocalInspection(remote);
          hydrateFrom(remote);
          return;
        }
      } catch {
        // local
      }
      const local = getLocalInspection(id);
      if (cancelled) return;
      if (local) hydrateFrom(local);
      else setNotFound(true);
    }
    void load().finally(() => {
      if (!cancelled) setLoading(false);
    });
    return () => {
      cancelled = true;
    };
  }, [id, hydrateFrom]);

  const draftExtraction = useMemo((): ExtractionResult | null => {
    if (!inspection?.extraction) return null;
    return {
      ...inspection.extraction,
      findings: findings.map(({ issue, severity, evidenceFromNotes }) => ({
        issue,
        severity,
        evidenceFromNotes,
      })),
    };
  }, [inspection, findings]);

  const canEdit = inspection?.status === "Ready for review";
  const canApprove = inspection?.status === "Ready for review";
  const canDraft =
    inspection?.status === "Approved" ||
    inspection?.status === "Follow-up prepared";

  const unresolved =
    inspection?.clarifications?.filter((c) => c.unresolved) ?? [];
  const openFollowUps =
    inspection?.extraction?.recommendedNextActions.length ??
    inspection?.unresolvedForManager ??
    unresolved.length;

  async function persistExtraction(next: ExtractionResult) {
    if (!inspection) return null;
    const updated: Inspection = {
      ...inspection,
      extraction: next,
      reviewedAt: new Date().toISOString(),
    };
    try {
      const remote = await saveInspectionExtraction(inspection.id, next);
      upsertLocalInspection(remote);
      hydrateFrom(remote);
      return remote;
    } catch {
      upsertLocalInspection(updated);
      hydrateFrom(updated);
      return updated;
    }
  }

  async function handleApprove() {
    if (!inspection || !canApprove || !draftExtraction) return;
    setBusy("approve");
    setError(null);
    setMessage(null);
    try {
      const withEdits = await persistExtraction(draftExtraction);
      if (!withEdits) return;
      try {
        const remote = await approveInspection(withEdits.id);
        upsertLocalInspection(remote);
        hydrateFrom(remote);
      } catch {
        const local: Inspection = {
          ...withEdits,
          status: "Approved",
          approvedAt: new Date().toISOString(),
        };
        upsertLocalInspection(local);
        hydrateFrom(local);
      }
      setMessage(
        "Handoff confirmed for office follow-through. This does not mark safety tests complete or close open follow-up items.",
      );
    } catch (err) {
      setError(err instanceof Error ? err.message : "Approval failed");
    } finally {
      setBusy(null);
    }
  }

  async function handleDrafts() {
    if (!inspection || !canDraft || !inspection.extraction) return;
    setBusy("drafts");
    setError(null);
    setMessage(null);
    try {
      try {
        const { inspection: remote } = await generateInspectionDrafts(
          inspection.id,
        );
        upsertLocalInspection(remote);
        hydrateFrom(remote);
      } catch {
        const drafts = generateDrafts({
          ...inspection,
          extraction: inspection.extraction,
          clarifications: inspection.clarifications,
        });
        const local: Inspection = {
          ...inspection,
          drafts,
          status: "Follow-up prepared",
        };
        upsertLocalInspection(local);
        hydrateFrom(local);
      }
      setMessage("Follow-up drafts prepared for review.");
    } catch (err) {
      setError(err instanceof Error ? err.message : "Could not prepare drafts");
    } finally {
      setBusy(null);
    }
  }

  if (loading) {
    return (
      <AppShell>
        <div className={cn(PAGE_GUTTER, "py-16 text-sm text-muted-foreground")}>
          Loading handoff…
        </div>
      </AppShell>
    );
  }

  if (notFound || !inspection) {
    return (
      <AppShell>
        <div className={cn(PAGE_GUTTER, "py-16")}>
          <h1 className="text-xl font-semibold text-navy">Not found</h1>
          <Link
            href="/dashboard"
            className={cn(buttonVariants({ variant: "outline" }), "mt-4 inline-flex")}
          >
            Operations
          </Link>
        </div>
      </AppShell>
    );
  }

  const { site, building } = parseSiteParts(
    inspection.siteName,
    inspection.building,
  );

  return (
    <AppShell>
      <div className={cn(PAGE_GUTTER, "space-y-5 pt-6 pb-28")}>
        <div className="space-y-3 border-b border-border pb-4">
          <div className="flex flex-wrap items-start justify-between gap-3">
            <div className="space-y-1">
              <p className="text-[11px] font-semibold tracking-wide text-muted-foreground uppercase">
                Manager verification
              </p>
              <h1 className="text-xl font-semibold tracking-tight text-navy sm:text-2xl">
                Ready for manager review
              </h1>
              <h2 className="text-lg font-medium text-foreground">{site}</h2>
              <div className="flex flex-wrap items-center gap-x-2 gap-y-1 text-sm text-muted-foreground">
                {building ? <span>{building}</span> : null}
                {building ? <span className="text-border">·</span> : null}
                <span className="font-mono tabular text-foreground">
                  {inspection.elevatorId}
                </span>
                <span className="text-border">·</span>
                <span>{inspection.inspectionType}</span>
                <span className="text-border">·</span>
                <span>{inspection.technicianName}</span>
                {inspection.workOrderNumber ? (
                  <>
                    <span className="text-border">·</span>
                    <span className="font-mono tabular">
                      WO #{inspection.workOrderNumber}
                    </span>
                  </>
                ) : null}
                <span className="text-border">·</span>
                <span className="tabular">
                  {dateFmt.format(new Date(inspection.createdAt))}
                </span>
              </div>
            </div>
            <Link
              href="/dashboard"
              className={cn(buttonVariants({ variant: "outline", size: "sm" }))}
            >
              Operations
            </Link>
          </div>
          <div className="flex flex-wrap gap-2">
            <span className="inline-flex rounded-md border border-primary/25 bg-primary/5 px-2 py-0.5 text-[11px] font-medium text-primary">
              Information capture: Complete
            </span>
            {openFollowUps > 0 ? (
              <span className="inline-flex rounded-md border border-warning/30 bg-warning/10 px-2 py-0.5 text-[11px] font-medium tabular text-warning">
                {openFollowUps} follow-up action{openFollowUps === 1 ? "" : "s"}{" "}
                open
              </span>
            ) : null}
            {unresolved.length > 0 ? (
              <span className="inline-flex rounded-md border border-warning/30 bg-warning/10 px-2 py-0.5 text-[11px] font-medium tabular text-warning">
                {unresolved.length} item{unresolved.length === 1 ? "" : "s"} need
                {unresolved.length === 1 ? "s" : ""} manager judgment
              </span>
            ) : null}
            <span className="inline-flex rounded-md border border-border bg-card px-2 py-0.5 text-[11px] font-medium">
              Ready for manager review
            </span>
          </div>
        </div>

        {(message || error) && (
          <div
            className={cn(
              "rounded-md border px-3 py-2 text-sm",
              error
                ? "border-critical/30 bg-critical/5 text-critical"
                : "border-primary/25 bg-primary/5 text-navy",
            )}
          >
            {error || message}
          </div>
        )}

        <div className="grid gap-5 lg:grid-cols-2">
          <div className="space-y-4">
            <section className="rounded-md border border-border bg-card">
              <div className="border-b border-border px-4 py-2.5">
                <h2 className="text-sm font-semibold">Supporting evidence</h2>
              </div>
              <pre className="max-h-[280px] overflow-auto whitespace-pre-wrap p-4 font-mono text-[12.5px] leading-relaxed">
                {inspection.notes || "No narration captured."}
              </pre>
            </section>

            <section className="rounded-md border border-border bg-card p-4">
              <h2 className="text-sm font-semibold">Technician clarifications</h2>
              {(inspection.clarifications ?? []).length === 0 ? (
                <p className="mt-2 text-sm text-muted-foreground">
                  No on-site clarifications recorded.
                </p>
              ) : (
                <ul className="mt-3 space-y-3 text-sm">
                  {(inspection.clarifications ?? []).map((c) => (
                    <li
                      key={c.gapId}
                      className={cn(
                        "rounded-md border p-3",
                        c.unresolved
                          ? "border-warning/30 bg-warning/5"
                          : "border-border bg-background",
                      )}
                    >
                      <p className="text-muted-foreground">{c.question}</p>
                      <p className="mt-1 font-medium">
                        {c.answerLabel}
                        {c.unresolved ? (
                          <span className="ml-2 text-[11px] font-semibold text-warning uppercase">
                            Needs verification
                          </span>
                        ) : null}
                      </p>
                    </li>
                  ))}
                </ul>
              )}
            </section>

            {inspection.photoNames && inspection.photoNames.length > 0 ? (
              <section className="rounded-md border border-border bg-card p-4 text-sm">
                <h2 className="text-sm font-semibold">Photos</h2>
                <ul className="mt-2 text-xs text-muted-foreground">
                  {inspection.photoNames.map((n) => (
                    <li key={n}>· {n}</li>
                  ))}
                </ul>
              </section>
            ) : null}
          </div>

          <div className="space-y-4">
            <section className="space-y-2">
              <h2 className="text-sm font-semibold">Findings</h2>
              <div className="space-y-2">
                {findings.map((f) => (
                  <FindingCard
                    key={f.key}
                    finding={f}
                    canEdit={Boolean(canEdit)}
                    editing={editingKey === f.key}
                    onEdit={() => setEditingKey(f.key)}
                    onConfirm={() => setEditingKey(null)}
                    onDismiss={() =>
                      setFindings((prev) => prev.filter((x) => x.key !== f.key))
                    }
                    onChange={(patch) =>
                      setFindings((prev) =>
                        prev.map((x) =>
                          x.key === f.key ? { ...x, ...patch } : x,
                        ),
                      )
                    }
                  />
                ))}
              </div>
            </section>

            <section className="rounded-md border border-border bg-card p-3">
              <h3 className="text-[11px] font-semibold tracking-wide text-muted-foreground uppercase">
                Recommended follow-up
              </h3>
              <ul className="mt-2 space-y-1 text-sm">
                {(inspection.extraction?.recommendedNextActions ?? []).map(
                  (a) => (
                    <li key={a}>· {a}</li>
                  ),
                )}
              </ul>
            </section>

            <section className="rounded-md border border-border bg-card p-3">
              <h3 className="text-[11px] font-semibold tracking-wide text-muted-foreground uppercase">
                Parts candidates
              </h3>
              <ul className="mt-2 space-y-1 text-sm">
                {(inspection.extraction?.partsMentioned ?? []).map((p) => (
                  <li key={p}>· {p}</li>
                ))}
              </ul>
            </section>

            <section className="rounded-md border border-warning/30 bg-warning/5 p-3">
              <h3 className="text-[11px] font-semibold tracking-wide text-warning uppercase">
                Remaining uncertainties
              </h3>
              {(inspection.extraction?.missingInformation ?? []).length ===
              0 ? (
                <p className="mt-2 text-sm text-muted-foreground">None flagged.</p>
              ) : (
                <ul className="mt-2 space-y-1 text-sm">
                  {(inspection.extraction?.missingInformation ?? []).map(
                    (m) => (
                      <li key={m}>· {m}</li>
                    ),
                  )}
                </ul>
              )}
            </section>

            {inspection.drafts ? (
              <div className="space-y-3">
                <p className="text-[11px] font-semibold tracking-wide text-muted-foreground uppercase">
                  Prepared for review
                </p>
                {(
                  [
                    ["Work-order draft", inspection.drafts.workOrder],
                    ["Parts-request draft", inspection.drafts.partsRequest],
                    ["Customer update", inspection.drafts.customerUpdate],
                    [
                      "Inspection summary",
                      inspection.drafts.inspectionSummary,
                    ],
                  ] as const
                ).map(([title, body]) => (
                  <section
                    key={title}
                    className="rounded-md border border-border bg-card"
                  >
                    <div className="border-b border-border px-3 py-2">
                      <h3 className="text-sm font-medium">{title}</h3>
                      <p className="text-[11px] text-warning">
                        Prepared for review — not submitted
                      </p>
                    </div>
                    <pre className="max-h-48 overflow-auto whitespace-pre-wrap p-3 font-mono text-[11px] leading-relaxed">
                      {body}
                    </pre>
                  </section>
                ))}
              </div>
            ) : null}
          </div>
        </div>
      </div>

      <div className="fixed inset-x-0 bottom-0 z-30 border-t border-border bg-background/95 backdrop-blur-sm">
        <div
          className={cn(
            PAGE_GUTTER,
            "flex flex-wrap items-center justify-between gap-3 py-3",
          )}
        >
          <p className="text-xs text-muted-foreground">
            Confirming a handoff does not mean regulatory approval, completed
            safety testing, or closed follow-up items.
          </p>
          <div className="flex flex-wrap gap-2">
            <Button
              type="button"
              size="sm"
              onClick={handleApprove}
              disabled={!canApprove || Boolean(busy)}
            >
              {busy === "approve" ? "Confirming…" : "Confirm handoff"}
            </Button>
            <Button
              type="button"
              variant="secondary"
              size="sm"
              onClick={handleDrafts}
              disabled={!canDraft || Boolean(busy)}
            >
              {busy === "drafts" ? "Preparing…" : "Prepare follow-up"}
            </Button>
          </div>
        </div>
      </div>
    </AppShell>
  );
}
