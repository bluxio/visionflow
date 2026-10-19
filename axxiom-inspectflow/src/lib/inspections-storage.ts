import { SEED_INSPECTIONS } from "@/lib/seed-inspections";
import type { Inspection } from "@/types/inspection";

const STORAGE_KEY = "axxiom-inspectflow.inspections.v4";

function readAll(): Inspection[] {
  if (typeof window === "undefined") return [];
  try {
    const raw = window.localStorage.getItem(STORAGE_KEY);
    if (!raw) {
      writeAll(SEED_INSPECTIONS);
      return structuredClone(SEED_INSPECTIONS);
    }
    const parsed = JSON.parse(raw) as unknown;
    if (!Array.isArray(parsed) || parsed.length === 0) {
      writeAll(SEED_INSPECTIONS);
      return structuredClone(SEED_INSPECTIONS);
    }
    return parsed as Inspection[];
  } catch {
    return structuredClone(SEED_INSPECTIONS);
  }
}

function writeAll(items: Inspection[]) {
  window.localStorage.setItem(STORAGE_KEY, JSON.stringify(items));
}

export function listLocalInspections(): Inspection[] {
  return readAll().sort((a, b) => b.createdAt.localeCompare(a.createdAt));
}

export function ensureSeedInspections(): Inspection[] {
  return listLocalInspections();
}

export function resetDemoData(): Inspection[] {
  writeAll(structuredClone(SEED_INSPECTIONS));
  return listLocalInspections();
}

export function getLocalInspection(id: string): Inspection | null {
  return readAll().find((i) => i.id === id) ?? null;
}

export function upsertLocalInspection(inspection: Inspection): void {
  const all = readAll().filter((i) => i.id !== inspection.id);
  all.unshift(inspection);
  writeAll(all);
}

export function mergeServerInspections(server: Inspection[]): Inspection[] {
  const local = readAll();
  const byId = new Map<string, Inspection>();
  for (const item of local) byId.set(item.id, item);
  for (const item of server) byId.set(item.id, item);
  const merged = Array.from(byId.values()).sort((a, b) =>
    b.createdAt.localeCompare(a.createdAt),
  );
  writeAll(merged);
  return merged;
}
