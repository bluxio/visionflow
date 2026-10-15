"use client";

import Link from "next/link";
import { useEffect, useMemo, useState } from "react";
import { ArrowUpRight, Search } from "lucide-react";

import { AppShell } from "@/components/layout/app-shell";
import { Badge } from "@/components/ui/badge";
import { Button, buttonVariants } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select";
import {
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from "@/components/ui/table";
import {
  nextActionLabel,
  parseSiteParts,
  primaryFindingLabel,
  statusRank,
  urgencyRank,
} from "@/lib/finding-ui";
import { fetchInspectionsBoard } from "@/lib/inspections-api-client";
import {
  ensureSeedInspections,
  mergeServerInspections,
  resetDemoData,
} from "@/lib/inspections-storage";
import { PAGE_GUTTER } from "@/lib/layout";
import { cn } from "@/lib/utils";
import type { Inspection } from "@/types/inspection";
import { INSPECTION_STATUSES } from "@/types/inspection";

const dateFmt = new Intl.DateTimeFormat(undefined, {
  month: "short",
  day: "numeric",
  hour: "numeric",
  minute: "2-digit",
});

function statusTone(status: Inspection["status"]): string {
  if (status === "Needs clarification")
    return "border-warning/30 bg-warning/10 text-warning";
  if (status === "Ready for review")
    return "border-primary/25 bg-primary/5 text-primary";
  if (status === "Approved") return "border-border bg-muted text-foreground";
  if (status === "Follow-up prepared")
    return "border-border bg-card text-muted-foreground";
  return "border-border bg-card text-foreground";
}

function jobHref(inspection: Inspection): string {
  if (inspection.status === "On site" || inspection.status === "Needs clarification") {
    return `/jobs/${inspection.id}`;
  }
  return `/inspections/${inspection.id}`;
}

function sortOperational(a: Inspection, b: Inspection): number {
  const st = statusRank(a.status) - statusRank(b.status);
  if (st !== 0) return st;
  const au = a.extraction?.overallUrgency ?? "low";
  const bu = b.extraction?.overallUrgency ?? "low";
  const urg = urgencyRank(au) - urgencyRank(bu);
  if (urg !== 0) return urg;
  return b.createdAt.localeCompare(a.createdAt);
}

export function DashboardClient() {
  const [items, setItems] = useState<Inspection[]>([]);
  const [loading, setLoading] = useState(true);
  const [query, setQuery] = useState("");
  const [status, setStatus] = useState("all");

  useEffect(() => {
    let cancelled = false;
    async function load() {
      setLoading(true);
      const seeded = ensureSeedInspections();
      try {
        const board = await fetchInspectionsBoard();
        if (cancelled) return;
        if (board.inspections.length > 0) {
          setItems(mergeServerInspections(board.inspections));
        } else {
          setItems(seeded);
        }
      } catch {
        if (!cancelled) setItems(seeded);
      } finally {
        if (!cancelled) setLoading(false);
      }
    }
    void load();
    return () => {
      cancelled = true;
    };
  }, []);

  const metrics = useMemo(() => {
    return {
      onSite: items.filter((i) => i.status === "On site").length,
      incompleteCaught: items.filter((i) => i.status === "Needs clarification")
        .length,
      ready: items.filter((i) => i.status === "Ready for review").length,
      followUps: items.filter((i) => i.status === "Follow-up prepared").length,
    };
  }, [items]);

  const filtered = useMemo(() => {
    const q = query.trim().toLowerCase();
    return items
      .filter((i) => {
        if (status !== "all" && i.status !== status) return false;
        if (!q) return true;
        const hay = [
          i.siteName,
          i.building,
          i.elevatorId,
          i.technicianName,
          primaryFindingLabel(i.extraction?.findings),
        ]
          .join(" ")
          .toLowerCase();
        return hay.includes(q);
      })
      .sort(sortOperational);
  }, [items, query, status]);

  return (
    <AppShell>
      <div className={cn(PAGE_GUTTER, "space-y-6 pt-8")}>
        <div className="flex flex-wrap items-end justify-between gap-4">
          <div className="max-w-2xl space-y-1.5">
            <h1 className="text-2xl font-semibold tracking-tight text-navy">
              Inspection operations
            </h1>
            <p className="text-sm text-muted-foreground">
              Manager view of field closeouts, verification, and follow-up.
            </p>
          </div>
          <div className="flex flex-wrap gap-2">
            <Button
              type="button"
              variant="ghost"
              size="sm"
              className="text-muted-foreground"
              onClick={() => setItems(resetDemoData())}
            >
              Reset demo data
            </Button>
            <Link
              href="/jobs/seed-riverside-b03"
              className={cn(buttonVariants())}
            >
              Open demo job
            </Link>
          </div>
        </div>

        <div className="grid grid-cols-2 gap-px overflow-hidden rounded-md border border-border bg-border md:grid-cols-4">
          {[
            ["On-site closeouts", metrics.onSite],
            ["Incomplete handoffs caught", metrics.incompleteCaught],
            ["Ready for review", metrics.ready],
            ["Follow-ups prepared", metrics.followUps],
          ].map(([label, value]) => (
            <div key={label} className="bg-card px-4 py-3">
              <p className="text-[11px] font-medium tracking-wide text-muted-foreground uppercase">
                {label}
              </p>
              <p className="mt-1 text-2xl font-semibold tabular text-foreground">
                {loading ? "—" : value}
              </p>
            </div>
          ))}
        </div>

        <div className="flex flex-col gap-3 rounded-md border border-border bg-card p-3 sm:flex-row sm:items-center">
          <div className="relative min-w-0 flex-1">
            <Search className="pointer-events-none absolute top-1/2 left-2.5 size-3.5 -translate-y-1/2 text-muted-foreground" />
            <Input
              value={query}
              onChange={(e) => setQuery(e.target.value)}
              placeholder="Search site, elevator, technician…"
              className="h-8 rounded-md border-border bg-background pl-8"
            />
          </div>
          <Select value={status} onValueChange={(v) => v && setStatus(v)}>
            <SelectTrigger className="h-8 w-[180px] rounded-md">
              <SelectValue placeholder="Status" />
            </SelectTrigger>
            <SelectContent>
              <SelectItem value="all">All statuses</SelectItem>
              {INSPECTION_STATUSES.map((s) => (
                <SelectItem key={s} value={s}>
                  {s}
                </SelectItem>
              ))}
            </SelectContent>
          </Select>
        </div>

        <div className="overflow-hidden rounded-md border border-border bg-card">
          <Table>
            <TableHeader>
              <TableRow className="hover:bg-transparent">
                <TableHead className="pl-4">Site</TableHead>
                <TableHead>Elevator</TableHead>
                <TableHead className="hidden md:table-cell">Technician</TableHead>
                <TableHead>Closeout status</TableHead>
                <TableHead className="hidden sm:table-cell">Unresolved</TableHead>
                <TableHead className="hidden lg:table-cell">Submitted</TableHead>
                <TableHead className="pr-4">Next action</TableHead>
              </TableRow>
            </TableHeader>
            <TableBody>
              {loading ? (
                <TableRow>
                  <TableCell
                    colSpan={7}
                    className="h-24 text-center text-muted-foreground"
                  >
                    Loading…
                  </TableCell>
                </TableRow>
              ) : (
                filtered.map((inspection) => {
                  const { site, building } = parseSiteParts(
                    inspection.siteName,
                    inspection.building,
                  );
                  const unresolved =
                    inspection.unresolvedForManager ??
                    inspection.extraction?.missingInformation.length ??
                    0;
                  return (
                    <TableRow key={inspection.id}>
                      <TableCell className="max-w-[200px] pl-4">
                        <div className="font-medium">{site}</div>
                        {building ? (
                          <div className="text-xs text-muted-foreground">
                            {building}
                          </div>
                        ) : null}
                      </TableCell>
                      <TableCell className="font-mono text-[13px] tabular">
                        {inspection.elevatorId}
                      </TableCell>
                      <TableCell className="hidden text-sm md:table-cell">
                        {inspection.technicianName}
                      </TableCell>
                      <TableCell>
                        <Badge
                          variant="outline"
                          className={cn(
                            "rounded-md font-medium",
                            statusTone(inspection.status),
                          )}
                        >
                          {inspection.status}
                        </Badge>
                      </TableCell>
                      <TableCell className="hidden tabular sm:table-cell">
                        {unresolved}
                      </TableCell>
                      <TableCell className="hidden text-sm tabular text-muted-foreground lg:table-cell">
                        {dateFmt.format(new Date(inspection.createdAt))}
                      </TableCell>
                      <TableCell className="pr-4">
                        <Link
                          href={jobHref(inspection)}
                          className="inline-flex items-center gap-1 text-sm font-medium text-primary hover:underline"
                        >
                          {nextActionLabel(inspection.status, unresolved)}
                          <ArrowUpRight className="size-3.5 opacity-70" />
                        </Link>
                      </TableCell>
                    </TableRow>
                  );
                })
              )}
            </TableBody>
          </Table>
        </div>
      </div>
    </AppShell>
  );
}
