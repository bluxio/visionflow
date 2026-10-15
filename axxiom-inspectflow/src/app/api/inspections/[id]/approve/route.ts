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
        { error: "MongoDB not configured — approve locally" },
        { status: 503 },
      );
    }

    const existing = await getInspectionFromDb(id);
    if (!existing) {
      return Response.json({ error: "Not found" }, { status: 404 });
    }

    if (existing.status === "Follow-up prepared") {
      return Response.json({
        inspection: existing,
        message: "Already past approval",
      });
    }

    if (existing.status !== "Ready for review" && existing.status !== "Approved") {
      return Response.json(
        { error: "Closeout must be submitted before approval" },
        { status: 403 },
      );
    }

    const now = new Date().toISOString();
    const updated = await updateInspectionInDb(id, {
      status: "Approved",
      approvedAt: now,
      reviewedAt: existing.reviewedAt ?? now,
    });

    return Response.json({ inspection: updated });
  } catch (err) {
    console.error("[/api/inspections/[id]/approve]", err);
    return Response.json({ error: "Approval failed" }, { status: 500 });
  }
}
