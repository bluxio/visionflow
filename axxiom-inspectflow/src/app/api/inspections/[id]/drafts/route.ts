import { generateDrafts } from "@/lib/drafts";
import {
  getInspectionFromDb,
  updateInspectionInDb,
} from "@/lib/inspection-repository";
import { isMongoConfigured } from "@/lib/mongodb";

type RouteContext = { params: Promise<{ id: string }> };

export async function POST(_request: Request, context: RouteContext) {
  try {
    const { id } = await context.params;

    if (!isMongoConfigured()) {
      return Response.json(
        { error: "MongoDB not configured — generate drafts locally" },
        { status: 503 },
      );
    }

    const existing = await getInspectionFromDb(id);
    if (!existing) {
      return Response.json({ error: "Not found" }, { status: 404 });
    }

    if (existing.status !== "Approved" && existing.status !== "Follow-up prepared") {
      return Response.json(
        {
          error:
            "Manager approval required before preparing follow-up drafts.",
        },
        { status: 403 },
      );
    }

    if (!existing.extraction) {
      return Response.json(
        { error: "No structured findings to draft from" },
        { status: 400 },
      );
    }

    const drafts = generateDrafts({
      ...existing,
      extraction: existing.extraction,
      clarifications: existing.clarifications,
    });
    const updated = await updateInspectionInDb(id, {
      drafts,
      status: "Follow-up prepared",
    });

    return Response.json({ inspection: updated, drafts });
  } catch (err) {
    console.error("[/api/inspections/[id]/drafts]", err);
    return Response.json(
      { error: "Draft generation failed" },
      { status: 500 },
    );
  }
}
