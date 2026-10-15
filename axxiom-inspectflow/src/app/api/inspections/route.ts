import { listInspectionsFromDb } from "@/lib/inspection-repository";
import { isMongoConfigured } from "@/lib/mongodb";
import type { BoardApiResponse } from "@/lib/inspections-api-client";

export async function GET() {
  try {
    if (!isMongoConfigured()) {
      const empty: BoardApiResponse = { inspections: [], source: "empty" };
      return Response.json(empty);
    }

    const inspections = await listInspectionsFromDb();
    return Response.json({
      inspections,
      source: "mongodb",
    } satisfies BoardApiResponse);
  } catch (err) {
    console.error("[/api/inspections GET]", err);
    return Response.json(
      { error: "Failed to list inspections" },
      { status: 500 },
    );
  }
}
