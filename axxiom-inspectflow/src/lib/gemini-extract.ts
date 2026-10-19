import type {
  ExtractionResult,
  Finding,
  SeverityLevel,
  UrgencyLevel,
} from "@/types/inspection";
import type { InspectionIntake } from "@/types/inspection";

const GEMINI_URL =
  "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent";

export const EXTRACTION_SYSTEM_PROMPT = `You are InspectFlow, an assistant for elevator field-to-office workflows (Axxiom-style inspection / maintenance operations).

Your job: turn unstructured technician inspection notes into structured JSON for a human manager to review.

Hard rules:
1. Extract ONLY facts explicitly stated or clearly supported by the notes. Do NOT invent equipment IDs, part numbers, measurements, code citations, or safety conclusions.
2. If something is unclear or absent, put it in missingInformation (or use null where noted). Never fabricate.
3. findings[].evidenceFromNotes must quote or closely paraphrase the technician's words.
4. severity and overallUrgency must be conservative: prefer lower urgency when uncertain.
5. confidence is 0–1 reflecting how complete and unambiguous the notes are.
6. You do NOT make safety shutdown decisions, order parts, or submit work. You only structure information for human approval.

Return ONLY valid JSON matching this schema (no markdown):
{
  "summary": "string — 2-4 sentence operational summary",
  "findings": [
    {
      "issue": "string",
      "severity": "low" | "medium" | "high" | "critical",
      "evidenceFromNotes": "string"
    }
  ],
  "partsMentioned": ["string"],
  "recommendedNextActions": ["string"],
  "missingInformation": ["string"],
  "overallUrgency": "low" | "medium" | "high" | "critical",
  "confidence": number
}`;

function buildUserPrompt(input: InspectionIntake): string {
  return [
    `Site / customer: ${input.siteName}`,
    `Elevator ID: ${input.elevatorId}`,
    `Inspection type: ${input.inspectionType}`,
    `Technician: ${input.technicianName}`,
    "",
    "Technician notes:",
    input.notes,
  ].join("\n");
}

function extractJson(text: string): string {
  const trimmed = text.trim();
  const fenced = trimmed.match(/```(?:json)?\s*([\s\S]*?)```/);
  if (fenced?.[1]) return fenced[1].trim();
  const start = trimmed.indexOf("{");
  const end = trimmed.lastIndexOf("}");
  if (start >= 0 && end > start) return trimmed.slice(start, end + 1);
  return trimmed;
}

function asUrgency(value: unknown): UrgencyLevel {
  if (
    value === "low" ||
    value === "medium" ||
    value === "high" ||
    value === "critical"
  ) {
    return value;
  }
  return "medium";
}

function asSeverity(value: unknown): SeverityLevel {
  return asUrgency(value);
}

function asStringArray(value: unknown): string[] {
  if (!Array.isArray(value)) return [];
  return value
    .filter((s): s is string => typeof s === "string" && s.trim().length > 0)
    .map((s) => s.trim());
}

function asFindings(value: unknown): Finding[] {
  if (!Array.isArray(value)) return [];
  const out: Finding[] = [];
  for (const item of value) {
    if (!item || typeof item !== "object") continue;
    const row = item as Record<string, unknown>;
    if (typeof row.issue !== "string" || !row.issue.trim()) continue;
    out.push({
      issue: row.issue.trim(),
      severity: asSeverity(row.severity),
      evidenceFromNotes:
        typeof row.evidenceFromNotes === "string"
          ? row.evidenceFromNotes.trim()
          : "",
    });
  }
  return out;
}

export function parseExtractionJson(raw: string): ExtractionResult {
  const parsed = JSON.parse(extractJson(raw)) as Partial<ExtractionResult>;

  if (typeof parsed.summary !== "string" || !parsed.summary.trim()) {
    throw new Error("Extraction missing summary");
  }

  const confidenceRaw =
    typeof parsed.confidence === "number" ? parsed.confidence : 0.5;
  const confidence = Math.min(1, Math.max(0, confidenceRaw));

  return {
    summary: parsed.summary.trim(),
    findings: asFindings(parsed.findings),
    partsMentioned: asStringArray(parsed.partsMentioned),
    recommendedNextActions: asStringArray(parsed.recommendedNextActions),
    missingInformation: asStringArray(parsed.missingInformation),
    overallUrgency: asUrgency(parsed.overallUrgency),
    confidence,
  };
}

export async function extractWithGemini(
  input: InspectionIntake,
): Promise<ExtractionResult> {
  const apiKey = process.env.GEMINI_API_KEY;
  if (!apiKey) {
    throw new Error("GEMINI_API_KEY is not configured");
  }

  const res = await fetch(`${GEMINI_URL}?key=${encodeURIComponent(apiKey)}`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
      systemInstruction: { parts: [{ text: EXTRACTION_SYSTEM_PROMPT }] },
      contents: [{ role: "user", parts: [{ text: buildUserPrompt(input) }] }],
      generationConfig: {
        temperature: 0.2,
        responseMimeType: "application/json",
      },
    }),
  });

  if (!res.ok) {
    const errText = await res.text();
    throw new Error(`Gemini API error ${res.status}: ${errText.slice(0, 200)}`);
  }

  const data = (await res.json()) as {
    candidates?: Array<{
      content?: { parts?: Array<{ text?: string }> };
    }>;
  };

  const text = data.candidates?.[0]?.content?.parts?.[0]?.text;
  if (!text) {
    throw new Error("Gemini returned empty content");
  }

  return parseExtractionJson(text);
}
