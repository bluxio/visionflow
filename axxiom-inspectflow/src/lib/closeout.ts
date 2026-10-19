import type { CloseoutGap, Inspection } from "@/types/inspection";

export const DEMO_CLOSEOUT_NARRATION = `Periodic inspection on ELV-B-03. Cab ride and leveling looked normal. There is slight oil underneath the hydraulic tank but I can't tell if it's active seepage or old residue. The seventh-floor hall button is sticking. Phase I fire recall worked from the lobby switch. I didn't verify Phase II from the in-car key switch. Hall button assembly may need replacement but I didn't confirm the part number.`;

export const RIVERSIDE_GAPS: CloseoutGap[] = [
  {
    id: "gap-oil",
    kind: "can_resolve",
    question:
      "The notes mention oil beneath the hydraulic tank. What did you observe?",
    why: "Missing information — confirm now while you can still see the tank.",
    options: [
      { id: "active", label: "Active seepage" },
      { id: "residue", label: "Old residue" },
      { id: "unable", label: "Unable to confirm", unresolved: true },
      { id: "photo", label: "Add photo" },
    ],
  },
  {
    id: "gap-button",
    kind: "can_resolve",
    question: "Can you confirm the hall button assembly?",
    why: "Missing information — photograph or identify the part before you leave.",
    options: [
      { id: "part", label: "Enter part number" },
      { id: "photo", label: "Take photo" },
      { id: "unknown", label: "Part unknown", unresolved: true },
    ],
    allowNote: true,
  },
  {
    id: "gap-phase-ii",
    kind: "requires_action",
    question:
      "Was Phase II fire service tested from the in-car key switch?",
    why: "Requires action — answering “Not tested” closes the information gap but does not complete the inspection step.",
    options: [
      { id: "passed", label: "Yes — passed" },
      { id: "issue", label: "Yes — issue found" },
      { id: "not-tested", label: "Not tested", unresolved: true },
      { id: "note", label: "Add note" },
    ],
    allowNote: true,
  },
];

/** Demo-assigned job the Loom opens first. */
export function createRiversideAssignedJob(): Inspection {
  return {
    id: "seed-riverside-b03",
    siteName: "Riverside Medical Center",
    building: "Tower B",
    elevatorId: "ELV-B-03",
    inspectionType: "Periodic inspection",
    technicianName: "Marcus Chen",
    workOrderNumber: "24817",
    scheduledLabel: "Today, 9:00 AM",
    notes: "",
    extraction: null,
    status: "On site",
    drafts: null,
    createdAt: new Date().toISOString(),
    extractionSource: "none",
    equipmentHistory: {
      lastInspection: "May 12, 2026 — Cat-1 passed",
      lastService: "Jun 3, 2026 — Door operator adjustment",
      knownOpenItem: "Floor 7 hall button intermittent (reported Jul 2026)",
      equipmentType: "Hydraulic passenger · 12 landings",
    },
    closeoutGaps: [],
    clarifications: [],
    photoNames: [],
    unresolvedForManager: 0,
  };
}

export function gapsForNotes(notes: string): CloseoutGap[] {
  const n = notes.toLowerCase();
  if (
    n.includes("phase ii") ||
    n.includes("hydraulic") ||
    n.includes("hall button") ||
    n.includes("elv-b-03")
  ) {
    return RIVERSIDE_GAPS;
  }

  const gaps: CloseoutGap[] = [];
  if (n.includes("oil") || n.includes("seepage") || n.includes("leak")) {
    gaps.push(RIVERSIDE_GAPS[0]);
  }
  if (n.includes("button") || n.includes("part")) {
    gaps.push(RIVERSIDE_GAPS[1]);
  }
  if (n.includes("phase") || n.includes("fire")) {
    gaps.push(RIVERSIDE_GAPS[2]);
  }
  return gaps.length > 0 ? gaps : RIVERSIDE_GAPS;
}
