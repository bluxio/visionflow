import type { ExtractionResult, InspectionIntake } from "@/types/inspection";

/**
 * Deterministic fallback when Gemini is unavailable.
 * Conservative: flags gaps rather than inventing details.
 */
export function mockExtractInspection(
  input: InspectionIntake,
): ExtractionResult {
  const notes = input.notes.toLowerCase();
  const findings: ExtractionResult["findings"] = [];
  const parts: string[] = [];
  const missing: string[] = [];
  const actions: string[] = [];

  if (notes.includes("leak") || notes.includes("oil") || notes.includes("seepage")) {
    findings.push({
      issue: "Possible hydraulic oil seepage in machine room",
      severity: "medium",
      evidenceFromNotes:
        "Notes mention oil seepage / leak indicators under the hydraulic tank.",
    });
    actions.push("Schedule follow-up to confirm leak rate and source");
    missing.push("Exact leak rate / volume over time not quantified");
  }

  if (notes.includes("button") || notes.includes("hall station")) {
    findings.push({
      issue: "Hall station / call button issue",
      severity: "medium",
      evidenceFromNotes:
        "Notes describe a sticky or faulty hall/call button needing service.",
    });
    parts.push("Hall button assembly (part number not confirmed)");
    missing.push("Confirmed part number for hall button assembly");
    actions.push("Create work order for hall button repair/replacement");
  }

  if (notes.includes("water") || notes.includes("pit") || notes.includes("debris")) {
    findings.push({
      issue: "Pit condition / water staining or debris noted",
      severity: "low",
      evidenceFromNotes:
        "Notes reference pit debris and/or water staining without standing water confirmed.",
    });
    missing.push("Drain status / active water intrusion status");
  }

  if (notes.includes("phase ii") || notes.includes("phase 2") || notes.includes("fire service")) {
    findings.push({
      issue: "Fire service verification incomplete or uncertain",
      severity: "high",
      evidenceFromNotes:
        "Notes indicate Phase I and/or Phase II fire service was not fully verified.",
    });
    missing.push("Full Phase II in-car fire service verification result");
    actions.push("Coordinate with building engineer to complete fire service tests");
  }

  if (findings.length === 0) {
    findings.push({
      issue: "Inspection notes received — detailed findings need manager review",
      severity: "low",
      evidenceFromNotes: "Automated fallback could not confidently isolate discrete issues.",
    });
    missing.push("Clearer issue list with locations and severity from technician");
  }

  if (notes.includes("written update") || notes.includes("customer")) {
    actions.push("Prepare customer-facing status update for facilities contact");
  }

  if (actions.length === 0) {
    actions.push("Manager review extracted findings before any work order");
  }

  const hasHigh = findings.some((f) => f.severity === "high" || f.severity === "critical");
  const overallUrgency = hasHigh ? "high" : findings.some((f) => f.severity === "medium") ? "medium" : "low";

  return {
    summary: `Inspection of ${input.elevatorId} at ${input.siteName} (${input.inspectionType}) by ${input.technicianName}. Structured findings are provisional — manager review required before any work order or customer communication.`,
    findings,
    partsMentioned: parts,
    recommendedNextActions: actions,
    missingInformation: missing.length
      ? missing
      : ["Confirm any omitted measurements or part numbers with technician"],
    overallUrgency,
    confidence: 0.55,
  };
}
