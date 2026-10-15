"use client";

import Link from "next/link";
import { useRouter, useSearchParams } from "next/navigation";
import { useEffect, useState } from "react";
import { Camera, ClipboardList } from "lucide-react";

import { AppShell } from "@/components/layout/app-shell";
import { Button, buttonVariants } from "@/components/ui/button";
import { createRiversideAssignedJob } from "@/lib/closeout";
import {
  ensureSeedInspections,
  getLocalInspection,
  resetDemoData,
  upsertLocalInspection,
} from "@/lib/inspections-storage";
import { PAGE_GUTTER } from "@/lib/layout";
import { cn } from "@/lib/utils";
import type { Inspection } from "@/types/inspection";

export function AssignedJob({ id }: { id: string }) {
  const router = useRouter();
  const searchParams = useSearchParams();
  const [job, setJob] = useState<Inspection | null>(null);

  useEffect(() => {
    if (searchParams.get("reset") === "1") {
      resetDemoData();
      router.replace(`/jobs/${id}`);
      return;
    }
    ensureSeedInspections();
    let row = getLocalInspection(id);
    if (!row && id === "seed-riverside-b03") {
      row = createRiversideAssignedJob();
      upsertLocalInspection(row);
    }
    setJob(row);
  }, [id, router, searchParams]);

  function restartDemo() {
    resetDemoData();
    const fresh = getLocalInspection(id) ?? createRiversideAssignedJob();
    upsertLocalInspection(fresh);
    setJob(fresh);
    router.replace(`/jobs/${id}`);
  }

  if (!job) {
    return (
      <AppShell>
        <div className={cn(PAGE_GUTTER, "max-w-lg py-16")}>
          <p className="text-sm text-muted-foreground">Job not found.</p>
          <Link
            href="/dashboard"
            className={cn(buttonVariants(), "mt-4 inline-flex")}
          >
            Operations board
          </Link>
        </div>
      </AppShell>
    );
  }

  const history = job.equipmentHistory;

  return (
    <AppShell>
      <div className={cn(PAGE_GUTTER, "max-w-lg space-y-5 pt-6 pb-10")}>
        <div className="flex items-start justify-between gap-3">
          <div className="space-y-1">
            <p className="text-[11px] font-semibold tracking-wide text-muted-foreground uppercase">
              Assigned inspection
            </p>
            <h1 className="text-2xl font-semibold tracking-tight text-navy">
              {job.siteName}
            </h1>
            <p className="text-sm text-muted-foreground">
              {[job.building, job.elevatorId, job.inspectionType]
                .filter(Boolean)
                .join(" · ")}
            </p>
          </div>
          <Button
            type="button"
            variant="ghost"
            size="sm"
            className="shrink-0 text-muted-foreground"
            onClick={restartDemo}
          >
            Restart demo
          </Button>
        </div>

        <section className="rounded-md border border-border bg-card p-4 text-sm">
          <dl className="space-y-2.5">
            <div className="flex justify-between gap-3">
              <dt className="text-muted-foreground">Work order</dt>
              <dd className="font-mono tabular font-medium">
                #{job.workOrderNumber ?? "—"}
              </dd>
            </div>
            <div className="flex justify-between gap-3">
              <dt className="text-muted-foreground">Technician</dt>
              <dd>{job.technicianName}</dd>
            </div>
            <div className="flex justify-between gap-3">
              <dt className="text-muted-foreground">Scheduled</dt>
              <dd>{job.scheduledLabel ?? "—"}</dd>
            </div>
            <div className="flex justify-between gap-3">
              <dt className="text-muted-foreground">Status</dt>
              <dd>{job.status}</dd>
            </div>
          </dl>
        </section>

        {history ? (
          <section className="rounded-md border border-border bg-card p-4">
            <h2 className="text-sm font-semibold">Equipment history</h2>
            <dl className="mt-3 space-y-2 text-sm">
              <div>
                <dt className="text-[11px] font-medium tracking-wide text-muted-foreground uppercase">
                  Type
                </dt>
                <dd className="mt-0.5">{history.equipmentType}</dd>
              </div>
              <div>
                <dt className="text-[11px] font-medium tracking-wide text-muted-foreground uppercase">
                  Last inspection
                </dt>
                <dd className="mt-0.5">{history.lastInspection}</dd>
              </div>
              <div>
                <dt className="text-[11px] font-medium tracking-wide text-muted-foreground uppercase">
                  Last service
                </dt>
                <dd className="mt-0.5">{history.lastService}</dd>
              </div>
              <div>
                <dt className="text-[11px] font-medium tracking-wide text-muted-foreground uppercase">
                  Known open item
                </dt>
                <dd className="mt-0.5 text-warning">{history.knownOpenItem}</dd>
              </div>
            </dl>
          </section>
        ) : null}

        <div className="space-y-2 pt-2">
          <Link
            href={
              job.status === "Needs clarification"
                ? `/jobs/${job.id}/gaps`
                : `/jobs/${job.id}/closeout`
            }
            className={cn(
              buttonVariants({ size: "lg" }),
              "h-12 w-full rounded-md text-base",
            )}
          >
            <ClipboardList className="size-4" />
            {job.status === "Needs clarification"
              ? "Finish closeout"
              : "Record closeout"}
          </Link>
          <Button
            type="button"
            variant="outline"
            size="lg"
            className="h-11 w-full rounded-md"
            onClick={() => {
              const input = document.createElement("input");
              input.type = "file";
              input.accept = "image/*";
              input.multiple = true;
              input.onchange = () => {
                const names = input.files
                  ? Array.from(input.files).map((f) => f.name)
                  : [];
                if (!names.length) return;
                const next = {
                  ...job,
                  photoNames: [...(job.photoNames ?? []), ...names],
                };
                upsertLocalInspection(next);
                setJob(next);
              };
              input.click();
            }}
          >
            <Camera className="size-4" />
            Add photos
            {job.photoNames && job.photoNames.length > 0
              ? ` (${job.photoNames.length})`
              : ""}
          </Button>
        </div>

        <p className="text-center text-xs text-muted-foreground">
          From field inspection to complete follow-through.
        </p>
      </div>
    </AppShell>
  );
}
