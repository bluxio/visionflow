import type {
  ExtractionResult,
  GeneratedDrafts,
  Inspection,
  InspectionIntake,
  InspectionStatus,
} from "@/types/inspection";

export type ExtractApiResponse = {
  inspection: Inspection;
  source: "gemini" | "mock";
  persisted: boolean;
};

export type BoardApiResponse = {
  inspections: Inspection[];
  source: "mongodb" | "empty";
};

export async function submitInspectionExtract(
  intake: InspectionIntake,
): Promise<ExtractApiResponse> {
  const res = await fetch("/api/extract", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(intake),
  });
  const data = (await res.json()) as ExtractApiResponse & { error?: string };
  if (!res.ok) {
    throw new Error(data.error || "Extraction failed");
  }
  return data;
}

export async function fetchInspectionsBoard(): Promise<BoardApiResponse> {
  const res = await fetch("/api/inspections", { cache: "no-store" });
  if (!res.ok) {
    return { inspections: [], source: "empty" };
  }
  return (await res.json()) as BoardApiResponse;
}

export async function fetchInspection(
  id: string,
): Promise<Inspection | null> {
  const res = await fetch(`/api/inspections/${id}`, { cache: "no-store" });
  if (res.status === 404) return null;
  if (!res.ok) throw new Error("Failed to load inspection");
  const data = (await res.json()) as { inspection: Inspection };
  return data.inspection;
}

export async function saveInspectionExtraction(
  id: string,
  extraction: ExtractionResult,
): Promise<Inspection> {
  const res = await fetch(`/api/inspections/${id}`, {
    method: "PATCH",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ extraction }),
  });
  const data = (await res.json()) as {
    inspection?: Inspection;
    error?: string;
  };
  if (!res.ok || !data.inspection) {
    throw new Error(data.error || "Failed to save edits");
  }
  return data.inspection;
}

export async function approveInspection(id: string): Promise<Inspection> {
  const res = await fetch(`/api/inspections/${id}/approve`, {
    method: "POST",
  });
  const data = (await res.json()) as {
    inspection?: Inspection;
    error?: string;
  };
  if (!res.ok || !data.inspection) {
    throw new Error(data.error || "Approval failed");
  }
  return data.inspection;
}

export async function generateInspectionDrafts(
  id: string,
): Promise<{ inspection: Inspection; drafts: GeneratedDrafts }> {
  const res = await fetch(`/api/inspections/${id}/drafts`, {
    method: "POST",
  });
  const data = (await res.json()) as {
    inspection?: Inspection;
    drafts?: GeneratedDrafts;
    error?: string;
  };
  if (!res.ok || !data.inspection || !data.drafts) {
    throw new Error(data.error || "Draft generation failed");
  }
  return { inspection: data.inspection, drafts: data.drafts };
}

export async function patchInspectionStatus(
  id: string,
  status: InspectionStatus,
): Promise<Inspection> {
  const res = await fetch(`/api/inspections/${id}`, {
    method: "PATCH",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ status }),
  });
  const data = (await res.json()) as {
    inspection?: Inspection;
    error?: string;
  };
  if (!res.ok || !data.inspection) {
    throw new Error(data.error || "Status update failed");
  }
  return data.inspection;
}
