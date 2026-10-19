import type {
  ClarificationAnswer,
  ExtractionResult,
  Inspection,
} from "@/types/inspection";
import { createRiversideAssignedJob } from "@/lib/closeout";

function emptyExtraction(): ExtractionResult {
  return {
    summary: "",
    findings: [],
    partsMentioned: [],
    recommendedNextActions: [],
    missingInformation: [],
    overallUrgency: "medium",
    confidence: 0.5,
  };
}

function withDefaults(partial: Partial<Inspection> & Pick<Inspection, "id" | "siteName" | "elevatorId" | "inspectionType" | "technicianName" | "status" | "createdAt">): Inspection {
  return {
    notes: "",
    extraction: null,
    drafts: null,
    extractionSource: "none",
    clarifications: [],
    closeoutGaps: [],
    photoNames: [],
    unresolvedForManager: 0,
    ...partial,
  };
}

/** Operational board seeds spanning the Field Closeout pipeline. */
export const SEED_INSPECTIONS: Inspection[] = [
  createRiversideAssignedJob(),
  withDefaults({
    id: "seed-harbor-a01",
    siteName: "Harbor View Apartments",
    building: "Building A",
    elevatorId: "ELV-A-01",
    inspectionType: "Callback / complaint",
    technicianName: "Denise Alvarez",
    workOrderNumber: "24822",
    notes:
      "Resident complaint: car leveling short at lobby. Releveling noisy. No entrapment.",
    status: "Needs clarification",
    createdAt: "2026-08-10T11:05:00.000Z",
    extractionSource: "mock",
    unresolvedForManager: 2,
    closeoutGaps: [],
    clarifications: [],
    extraction: {
      summary: "Lobby leveling shortfall; clarifications still open on-site.",
      findings: [
        {
          issue: "Lobby leveling short (~3/4\")",
          severity: "medium",
          evidenceFromNotes: "Car leveling short at lobby.",
        },
      ],
      partsMentioned: [],
      recommendedNextActions: ["Confirm controller fault log"],
      missingInformation: ["Controller fault log", "Noise source confirmation"],
      overallUrgency: "medium",
      confidence: 0.7,
    },
  }),
  withDefaults({
    id: "seed-civic-frgt",
    siteName: "Civic Center Parking Garage",
    elevatorId: "FRGT-1",
    inspectionType: "Maintenance check",
    technicianName: "Denise Alvarez",
    workOrderNumber: "24791",
    notes:
      "Freight gate switch intermittent at landing 2. Temporary adjustment restored operation.",
    status: "Ready for review",
    createdAt: "2026-08-10T13:48:00.000Z",
    extractionSource: "mock",
    unresolvedForManager: 1,
    clarifications: [
      {
        gapId: "gap-gate",
        question: "Is the unit safe to leave in service until replacement?",
        answerLabel: "Yes — monitoring until replacement",
        unresolved: false,
      },
    ],
    extraction: {
      summary:
        "Freight gate switch intermittent; temporary fix in place. Replacement recommended.",
      findings: [
        {
          issue: "Intermittent gate switch at landing 2",
          severity: "high",
          evidenceFromNotes: "Gate switch intermittent at landing 2.",
        },
      ],
      partsMentioned: ["Landing gate switch assembly"],
      recommendedNextActions: ["Replace gate switch", "Keep monitored"],
      missingInformation: ["OEM part number confirmation"],
      overallUrgency: "high",
      confidence: 0.72,
    },
  }),
  withDefaults({
    id: "seed-metro-svc2",
    siteName: "Metro County Courthouse",
    elevatorId: "SVC-2",
    inspectionType: "Maintenance check",
    technicianName: "James Okonkwo",
    workOrderNumber: "24755",
    notes: "Guide shoes worn on car side, rails OK. Recommend shoe replacement next PM.",
    status: "Approved",
    createdAt: "2026-08-10T09:40:00.000Z",
    approvedAt: "2026-08-10T15:10:00.000Z",
    extractionSource: "mock",
    unresolvedForManager: 0,
    clarifications: [
      {
        gapId: "gap-shoes",
        question: "Are rails within wear tolerance?",
        answerLabel: "Yes — rails OK",
        unresolved: false,
      },
    ],
    extraction: {
      summary: "Worn guide shoes; rails acceptable. Shoe replacement at next PM.",
      findings: [
        {
          issue: "Worn car guide shoes",
          severity: "medium",
          evidenceFromNotes: "Guide shoes worn on car side, rails OK.",
        },
      ],
      partsMentioned: ["Car guide shoes (pair)"],
      recommendedNextActions: ["Add guide shoe replacement to next PM"],
      missingInformation: [],
      overallUrgency: "low",
      confidence: 0.88,
    },
  }),
  withDefaults({
    id: "seed-westfield-p4",
    siteName: "Westfield Office Park",
    building: "Tower 2",
    elevatorId: "ELV-P-04",
    inspectionType: "Periodic inspection",
    technicianName: "Priya Nair",
    workOrderNumber: "24710",
    notes:
      "Cat-1 complete. All tests passed. Machine room HVAC filter dirty — building responsibility.",
    status: "Follow-up prepared",
    createdAt: "2026-08-09T16:15:00.000Z",
    approvedAt: "2026-08-09T17:50:00.000Z",
    extractionSource: "mock",
    unresolvedForManager: 0,
    extraction: {
      summary:
        "Periodic inspection passed. Facilities notified about HVAC filter.",
      findings: [
        {
          issue: "Machine room HVAC filter dirty (building)",
          severity: "low",
          evidenceFromNotes: "HVAC filter dirty (building responsibility).",
        },
      ],
      partsMentioned: [],
      recommendedNextActions: ["Notify facilities of HVAC filter"],
      missingInformation: [],
      overallUrgency: "low",
      confidence: 0.9,
    },
    drafts: {
      workOrder:
        "WORK-ORDER DRAFT — PREPARED FOR REVIEW\n\nNotify facilities: machine room HVAC filter on ELV-P-04.",
      partsRequest:
        "PARTS REQUEST DRAFT — PREPARED FOR REVIEW\n\nNo Axxiom parts required.",
      customerUpdate:
        "CUSTOMER UPDATE DRAFT — PREPARED FOR REVIEW\n\nPeriodic inspection on ELV-P-04 completed. Please replace machine room HVAC filter.",
      inspectionSummary:
        "INSPECTION SUMMARY DRAFT — PREPARED FOR REVIEW\n\nELV-P-04 Cat-1 passed. Building HVAC filter follow-up only.",
      generatedAt: "2026-08-09T18:20:00.000Z",
    },
  }),
  withDefaults({
    id: "seed-lakeside-fr1",
    siteName: "Lakeside Hotel",
    building: "Front of house",
    elevatorId: "ELV-FR-1",
    inspectionType: "Acceptance / new install",
    technicianName: "Marcus Chen",
    workOrderNumber: "24688",
    notes:
      "Modernization punch. Hall lanterns corrected on floors 3 and 8. AHJ witness Thursday.",
    status: "Ready for review",
    createdAt: "2026-08-08T20:30:00.000Z",
    extractionSource: "mock",
    unresolvedForManager: 1,
    clarifications: [
      {
        gapId: "gap-ahj",
        question: "Is AHJ witness confirmed?",
        answerLabel: "Scheduled Thursday — result pending",
        unresolved: true,
      },
    ],
    extraction: {
      summary:
        "Modernization punch largely complete; AHJ witness still pending.",
      findings: [
        {
          issue: "AHJ witness test still pending",
          severity: "medium",
          evidenceFromNotes: "Final acceptance pending AHJ witness Thursday.",
        },
      ],
      partsMentioned: [],
      recommendedNextActions: ["Coordinate AHJ witness Thursday"],
      missingInformation: ["AHJ witness test result"],
      overallUrgency: "medium",
      confidence: 0.84,
    },
  }),
];

export function buildCloseoutExtraction(
  inspection: Inspection,
  clarifications: ClarificationAnswer[],
): ExtractionResult {
  const notes = inspection.notes.toLowerCase();
  const findings: ExtractionResult["findings"] = [];
  const parts: string[] = [];
  const actions: string[] = [];
  const missing: string[] = [];

  const phase = clarifications.find((c) => c.gapId === "gap-phase-ii");
  const oil = clarifications.find((c) => c.gapId === "gap-oil");
  const button = clarifications.find((c) => c.gapId === "gap-button");

  if (notes.includes("phase i") || notes.includes("fire recall")) {
    findings.push({
      issue: "Phase I fire recall passed from lobby switch",
      severity: "low",
      evidenceFromNotes:
        "Phase I fire recall worked from the lobby switch.",
    });
  }

  if (phase) {
    if (phase.answerLabel.startsWith("Yes — passed")) {
      findings.push({
        issue: "Phase II fire service tested — passed",
        severity: "low",
        evidenceFromNotes: phase.note || phase.answerLabel,
      });
    } else if (phase.answerLabel.startsWith("Yes — issue")) {
      findings.push({
        issue: "Phase II fire service — issue found",
        severity: "high",
        evidenceFromNotes: phase.note || phase.answerLabel,
      });
      actions.push("Investigate Phase II fire service issue");
    } else {
      findings.push({
        issue: "Phase II fire service not tested from in-car key switch",
        severity: "high",
        evidenceFromNotes: phase.note || phase.answerLabel,
      });
      missing.push("Phase II in-car fire service result");
      actions.push("Manager judgment on incomplete Phase II verification");
    }
  }

  if (oil) {
    if (oil.answerLabel === "Active seepage") {
      findings.push({
        issue: "Active hydraulic oil seepage under tank",
        severity: "high",
        evidenceFromNotes: oil.note || oil.answerLabel,
      });
      actions.push("Schedule hydraulic leak follow-up");
    } else if (oil.answerLabel === "Old residue") {
      findings.push({
        issue: "Hydraulic tank oil — old residue (not active seepage)",
        severity: "low",
        evidenceFromNotes: oil.note || oil.answerLabel,
      });
    } else {
      findings.push({
        issue: "Hydraulic tank condition: unable to confirm active seepage",
        severity: "medium",
        evidenceFromNotes: oil.note || oil.answerLabel,
      });
      missing.push("Whether hydraulic oil is active seepage vs residue");
      actions.push("Manager review of hydraulic tank observation");
    }
  }

  if (notes.includes("hall button") || notes.includes("sticking")) {
    findings.push({
      issue: "Hall button sticking on floor 7",
      severity: "medium",
      evidenceFromNotes: "The seventh-floor hall button is sticking.",
    });
    actions.push("Verify/replace hall button assembly");
  }

  if (button) {
    if (/^Part number:/i.test(button.answerLabel) && button.note) {
      parts.push(`Hall button assembly — ${button.note}`);
    } else {
      parts.push("Hall button assembly");
      parts.push("Exact part number not confirmed.");
      parts.push("Photo or part identification required before ordering.");
      parts.push("Order status: Blocked pending verification.");
      missing.push("Hall button assembly part number");
      if (!actions.some((a) => /hall button/i.test(a))) {
        actions.push("Identify hall button assembly before ordering");
      }
    }
  } else if (notes.includes("hall button") || notes.includes("sticking")) {
    parts.push("Hall button assembly");
    parts.push("Exact part number not confirmed.");
    parts.push("Photo or part identification required before ordering.");
    parts.push("Order status: Blocked pending verification.");
  }

  const openFollowUps = actions.length;

  return {
    summary:
      openFollowUps > 0
        ? `Closeout captured for ${inspection.elevatorId}. ${openFollowUps} follow-up action${openFollowUps === 1 ? "" : "s"} remain open.`
        : `Closeout captured for ${inspection.elevatorId}. Ready for manager verification.`,
    findings,
    partsMentioned: parts.length
      ? parts
      : [
          "No parts ready to order.",
          "Order status: Blocked pending verification.",
        ],
    recommendedNextActions: actions.length
      ? actions
      : ["Manager verification of closeout"],
    missingInformation: missing,
    overallUrgency: findings.some((f) => f.severity === "high")
      ? "high"
      : "medium",
    confidence: 0.75,
  };
}

// silence unused helper if tree-shaken oddly
void emptyExtraction;
