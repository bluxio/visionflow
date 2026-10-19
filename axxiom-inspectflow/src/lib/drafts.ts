import type {
  ExtractionResult,
  GeneratedDrafts,
  InspectionIntake,
} from "@/types/inspection";

function phaseIiOutstanding(
  extraction: ExtractionResult,
  clarifications?: { gapId: string; answerLabel: string; unresolved: boolean }[],
): boolean {
  const fromClarification = clarifications?.some(
    (c) =>
      c.gapId === "gap-phase-ii" &&
      (c.unresolved || /not tested/i.test(c.answerLabel)),
  );
  const fromFindings = extraction.findings.some((f) =>
    /phase ii.*not tested/i.test(f.issue),
  );
  return Boolean(fromClarification || fromFindings);
}

function buildPartsBlock(extraction: ExtractionResult): string[] {
  const mentioned = extraction.partsMentioned.filter(
    (p) => p && !/^none confirmed$/i.test(p.trim()),
  );
  const hallRelated = mentioned.some((p) => /hall button/i.test(p));
  const partConfirmed = mentioned.some((p) => /part number:/i.test(p));

  if (hallRelated && !partConfirmed) {
    return [
      "Hall button assembly",
      "Exact part number not confirmed.",
      "Photo or part identification required before ordering.",
      "Order status: Blocked pending verification.",
    ];
  }

  if (mentioned.length > 0) {
    return [
      ...mentioned,
      "Order status: Blocked pending verification.",
    ];
  }

  return [
    "No parts ready to order.",
    "Order status: Blocked pending verification.",
  ];
}

export function generateDrafts(
  inspection: InspectionIntake & {
    extraction: ExtractionResult;
    workOrderNumber?: string;
    clarifications?: {
      gapId?: string;
      question: string;
      answerLabel: string;
      unresolved: boolean;
    }[];
  },
): GeneratedDrafts {
  const { siteName, elevatorId, inspectionType, technicianName, extraction } =
    inspection;

  const findingsBlock = extraction.findings
    .map(
      (f, i) =>
        `${i + 1}. [${f.severity.toUpperCase()}] ${f.issue}\n   Evidence: ${f.evidenceFromNotes || "(none)"}`,
    )
    .join("\n");

  const clarifications =
    inspection.clarifications
      ?.map(
        (c) =>
          `- ${c.question}\n  Answer: ${c.answerLabel}${c.unresolved ? " (unresolved)" : ""}`,
      )
      .join("\n") || "- None";

  const workOrder = [
    "WORK-ORDER DRAFT — PREPARED FOR REVIEW",
    "Do not dispatch until a manager confirms the handoff.",
    "",
    `Work order: ${inspection.workOrderNumber ? `#${inspection.workOrderNumber}` : "(new)"}`,
    `Site: ${siteName}`,
    `Elevator: ${elevatorId}`,
    `Inspection type: ${inspectionType}`,
    `Technician: ${technicianName}`,
    `Priority: ${extraction.overallUrgency}`,
    "",
    "Summary:",
    extraction.summary,
    "",
    "Findings:",
    findingsBlock || "- (none)",
    "",
    "Recommended next actions:",
    extraction.recommendedNextActions.map((a) => `- ${a}`).join("\n") ||
      "- (none)",
    "",
    "Open gaps:",
    extraction.missingInformation.map((m) => `- ${m}`).join("\n") || "- None",
  ].join("\n");

  const partsLines = buildPartsBlock(extraction);
  const partsRequest = [
    "PARTS REQUEST DRAFT — PREPARED FOR REVIEW",
    "No order has been placed. Do not purchase from this draft alone.",
    "",
    `Site: ${siteName}`,
    `Elevator: ${elevatorId}`,
    "",
    "Parts candidates:",
    ...partsLines.map((line) => `- ${line}`),
  ].join("\n");

  const phaseOpen = phaseIiOutstanding(
    extraction,
    inspection.clarifications as
      | { gapId: string; answerLabel: string; unresolved: boolean }[]
      | undefined,
  );

  const customerBody = phaseOpen
    ? `A technician completed an on-site inspection visit for elevator ${elevatorId} at ${siteName}. Phase I fire recall passed. Phase II verification remains outstanding and has been flagged for follow-up.`
    : `A technician completed an on-site inspection visit for elevator ${elevatorId} at ${siteName}. Findings from the visit are below; manager review is still required before final inspection closure.`;

  const customerUpdate = [
    "CUSTOMER UPDATE DRAFT — PREPARED FOR REVIEW",
    "Do not send until a manager confirms the wording.",
    "",
    "Dear Facilities team,",
    "",
    customerBody,
    "",
    "Items we are tracking:",
    extraction.findings.map((f) => `- ${f.issue}`).join("\n") ||
      "- (to be confirmed)",
    "",
    "Note: An on-site visit is not the same as completed inspection requirements.",
    "",
    "Thank you,",
    "Axxiom service coordination",
  ].join("\n");

  const inspectionSummary = [
    "INSPECTION SUMMARY DRAFT — PREPARED FOR REVIEW",
    "",
    `Site: ${siteName}`,
    `Elevator: ${elevatorId}`,
    `Type: ${inspectionType}`,
    `Technician: ${technicianName}`,
    "",
    extraction.summary,
    "",
    "Structured findings:",
    findingsBlock || "- (none)",
    "",
    "Technician clarifications:",
    clarifications,
    "",
    "Remaining uncertainties / open follow-up:",
    extraction.missingInformation.map((m) => `- ${m}`).join("\n") || "- None",
  ].join("\n");

  return {
    workOrder,
    partsRequest,
    customerUpdate,
    inspectionSummary,
    generatedAt: new Date().toISOString(),
  };
}
