import type {
  ExtractionResult,
  GeneratedDrafts,
  Inspection,
  InspectionStatus,
} from "@/types/inspection";
import { getInspectFlowDb } from "@/lib/mongodb";

const STRIP = new Set(["_id", "insertedAt", "updatedAt"]);

export function documentToInspection(
  doc: Record<string, unknown>,
): Inspection | null {
  if (typeof doc.id !== "string" || typeof doc.siteName !== "string") {
    return null;
  }
  const row = { ...doc } as Record<string, unknown>;
  for (const key of STRIP) {
    delete row[key];
  }
  return row as Inspection;
}

export async function insertInspection(inspection: Inspection): Promise<void> {
  const db = await getInspectFlowDb();
  await db.collection("inspections").insertOne({
    ...inspection,
    insertedAt: new Date().toISOString(),
  });
}

export async function listInspectionsFromDb(): Promise<Inspection[]> {
  const db = await getInspectFlowDb();
  const docs = await db
    .collection("inspections")
    .find({})
    .sort({ createdAt: -1 })
    .toArray();

  return docs
    .map((d) => documentToInspection(d as Record<string, unknown>))
    .filter((i): i is Inspection => i !== null);
}

export async function getInspectionFromDb(
  id: string,
): Promise<Inspection | null> {
  const db = await getInspectFlowDb();
  const doc = await db.collection("inspections").findOne({ id });
  if (!doc) return null;
  return documentToInspection(doc as Record<string, unknown>);
}

export async function updateInspectionInDb(
  id: string,
  patch: {
    extraction?: ExtractionResult;
    status?: InspectionStatus;
    drafts?: GeneratedDrafts | null;
    reviewedAt?: string;
    approvedAt?: string;
  },
): Promise<Inspection | null> {
  const db = await getInspectFlowDb();
  const existing = await db.collection("inspections").findOne({ id });
  if (!existing) return null;

  const current = documentToInspection(existing as Record<string, unknown>);
  if (!current) return null;

  const next: Inspection = {
    ...current,
    ...patch,
    extraction: patch.extraction ?? current.extraction,
    status: patch.status ?? current.status,
    drafts: patch.drafts !== undefined ? patch.drafts : current.drafts,
  };

  await db.collection("inspections").updateOne(
    { id },
    {
      $set: {
        ...patch,
        updatedAt: new Date().toISOString(),
      },
    },
  );

  return next;
}
