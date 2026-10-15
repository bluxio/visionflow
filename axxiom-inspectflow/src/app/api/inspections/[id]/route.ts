import {
  getInspectionFromDb,
  updateInspectionInDb,
} from "@/lib/inspection-repository";
import { isMongoConfigured } from "@/lib/mongodb";
import type { ExtractionResult, InspectionStatus } from "@/types/inspection";
import { INSPECTION_STATUSES } from "@/types/inspection";

type RouteContext = { params: Promise<{ id: string }> };

function isStatus(value: unknown): value is InspectionStatus {
  return (
    typeof value === "string" &&
    (INSPECTION_STATUSES as readonly string[]).includes(value)
  );
}

export async function GET(_request: Request, context: RouteContext) {
  try {
    const { id } = await context.params;

    if (!isMongoConfigured()) {
      return Response.json(
        { error: "MongoDB not configured" },
        { status: 503 },
      );
    }

    const inspection = await getInspectionFromDb(id);
    if (!inspection) {
      return Response.json({ error: "Not found" }, { status: 404 });
    }

    return Response.json({ inspection });
  } catch (err) {
    console.error("[/api/inspections/[id] GET]", err);
    return Response.json({ error: "Failed to load" }, { status: 500 });
  }
}

export async function PATCH(request: Request, context: RouteContext) {
  try {
    const { id } = await context.params;

    if (!isMongoConfigured()) {
      return Response.json(
        {
          error:
            "MongoDB not configured — client should persist edits locally",
        },
        { status: 503 },
      );
    }

    const body: unknown = await request.json();
    if (!body || typeof body !== "object") {
      return Response.json({ error: "Invalid body" }, { status: 400 });
    }

    const b = body as Record<string, unknown>;
    const patch: {
      extraction?: ExtractionResult;
      status?: InspectionStatus;
      reviewedAt?: string;
    } = {};

    if (b.extraction) {
      patch.extraction = b.extraction as ExtractionResult;
      patch.reviewedAt = new Date().toISOString();
    }
    if (isStatus(b.status)) {
      patch.status = b.status;
    }

    if (!patch.extraction && !patch.status) {
      return Response.json({ error: "Nothing to update" }, { status: 400 });
    }

    const existing = await getInspectionFromDb(id);
    if (!existing) {
      return Response.json({ error: "Not found" }, { status: 404 });
    }

    const updated = await updateInspectionInDb(id, patch);
    return Response.json({ inspection: updated });
  } catch (err) {
    console.error("[/api/inspections/[id] PATCH]", err);
    return Response.json({ error: "Update failed" }, { status: 500 });
  }
}
