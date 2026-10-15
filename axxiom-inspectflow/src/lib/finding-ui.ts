import type { Finding, UrgencyLevel } from "@/types/inspection";
import type { InspectionStatus } from "@/types/inspection";

export type FindingGroup =
  | "Safety & compliance"
  | "Maintenance"
  | "Site condition";

const SAFETY =
  /fire|phase|safety|code|compliance|recall|shutdown|interlock|governor|brake|door.?restrict/i;
const SITE =
  /pit|water|debris|machine room|access|stain|hvac|stored|floor|wall|drain/i;

export function categorizeFinding(finding: Finding): FindingGroup {
  const text = `${finding.issue} ${finding.evidenceFromNotes}`;
  if (SAFETY.test(text)) return "Safety & compliance";
  if (SITE.test(text)) return "Site condition";
  return "Maintenance";
}

const UNCERTAIN =
  /could not|couldn't|unknown|tbd|not confirmed|not fully|unclear|may need|possible|approx|about |unable|blocked|verify|verification|uncertain|not inspected|not tested|pending/i;

export function findingNeedsVerification(finding: Finding): boolean {
  const text = `${finding.issue} ${finding.evidenceFromNotes}`;
  if (!finding.evidenceFromNotes.trim()) return true;
  return UNCERTAIN.test(text);
}

export function urgencyRank(u: UrgencyLevel): number {
  switch (u) {
    case "critical":
      return 0;
    case "high":
      return 1;
    case "medium":
      return 2;
    default:
      return 3;
  }
}

export function parseSiteParts(siteName: string, building?: string): {
  site: string;
  building: string | null;
} {
  if (building) return { site: siteName, building };
  const parts = siteName.split(/\s+[—–-]\s+/);
  if (parts.length >= 2) {
    return { site: parts[0].trim(), building: parts.slice(1).join(" — ").trim() };
  }
  return { site: siteName, building: null };
}

export function primaryFindingLabel(
  findings: Finding[] | undefined | null,
  fallback = "Closeout in progress",
): string {
  if (!findings || findings.length === 0) return fallback;
  const ranked = [...findings].sort((a, b) => {
    const order = { critical: 0, high: 1, medium: 2, low: 3 } as const;
    return order[a.severity] - order[b.severity];
  });
  return ranked[0].issue;
}

export function nextActionLabel(
  status: InspectionStatus,
  unresolved: number,
): string {
  switch (status) {
    case "On site":
      return "Record closeout";
    case "Needs clarification":
      return "Finish closeout";
    case "Ready for review":
      return unresolved > 0 ? "Manager judgment" : "Review handoff";
    case "Approved":
      return "Prepare follow-up";
    case "Follow-up prepared":
      return "View drafts";
    default:
      return "Open";
  }
}

export function statusRank(status: InspectionStatus): number {
  switch (status) {
    case "Needs clarification":
      return 0;
    case "On site":
      return 1;
    case "Ready for review":
      return 2;
    case "Approved":
      return 3;
    default:
      return 4;
  }
}
