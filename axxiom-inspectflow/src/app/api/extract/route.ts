import { extractWithGemini } from "@/lib/gemini-extract";
import { insertInspection } from "@/lib/inspection-repository";
import { isMongoConfigured } from "@/lib/mongodb";
import { mockExtractInspection } from "@/lib/mock-extract";
import type { ExtractApiResponse } from "@/lib/inspections-api-client";
import type { Inspection, InspectionIntake } from "@/types/inspection";

function isValidBody(body: unknown): body is InspectionIntake {
  if (!body || typeof body !== "object") return false;
  const b = body as Record<string, unknown>;
  return (
    typeof b.siteName === "string" &&
    b.siteName.trim().length > 1 &&
    typeof b.elevatorId === "string" &&
    b.elevatorId.trim().length > 0 &&
    typeof b.inspectionType === "string" &&
    typeof b.technicianName === "string" &&
    b.technicianName.trim().length > 1 &&
    typeof b.notes === "string" &&
    b.notes.trim().length > 20
  );
}

export async function POST(request: Request) {
  try {
    const body: unknown = await request.json();
    if (!isValidBody(body)) {
      return Response.json(
        { error: "Invalid inspection intake payload" },
        { status: 400 },
      );
    }

    const intake: InspectionIntake = {
      siteName: body.siteName.trim(),
      elevatorId: body.elevatorId.trim(),
      inspectionType: body.inspectionType,
      technicianName: body.technicianName.trim(),
      notes: body.notes.trim(),
    };

    let source: "gemini" | "mock" = "mock";
    let extraction;

    if (process.env.GEMINI_API_KEY) {
      try {
        extraction = await extractWithGemini(intake);
        source = "gemini";
      } catch (err) {
        console.error("[/api/extract] Gemini failed, using mock:", err);
        extraction = mockExtractInspection(intake);
      }
    } else {
      extraction = mockExtractInspection(intake);
    }

    const inspection: Inspection = {
      id: crypto.randomUUID(),
      ...intake,
      extraction,
      status: "Ready for review",
      drafts: null,
      createdAt: new Date().toISOString(),
      extractionSource: source,
      clarifications: [],
      unresolvedForManager: extraction.missingInformation.length,
    };

    let persisted = false;
    if (isMongoConfigured()) {
      try {
        await insertInspection(inspection);
        persisted = true;
      } catch (err) {
        console.error("[/api/extract] MongoDB failed:", err);
      }
    }

    const response: ExtractApiResponse = {
      inspection,
      source,
      persisted,
    };

    return Response.json(response);
  } catch (err) {
    console.error("[/api/extract]", err);
    return Response.json({ error: "Extraction failed" }, { status: 500 });
  }
}
