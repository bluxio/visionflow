import {
  buildUserPrompt,
  EXTRACTION_SYSTEM_PROMPT,
} from "@/lib/extraction-prompt";
import type { AnalysisResult } from "@/lib/types";

const GEMINI_URL =
  "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent";

function extractJson(text: string): unknown {
  const trimmed = text.trim();
  const fenced = trimmed.match(/```(?:json)?\s*([\s\S]*?)```/);
  const candidate = fenced?.[1]?.trim() ?? trimmed;

  try {
    return JSON.parse(candidate);
  } catch {
    const start = candidate.indexOf("{");
    const end = candidate.lastIndexOf("}");
    if (start === -1 || end === -1 || end <= start) {
      throw new Error("Model response was not valid JSON");
    }
    return JSON.parse(candidate.slice(start, end + 1));
  }
}

function isAnalysisResult(value: unknown): value is AnalysisResult {
  if (!value || typeof value !== "object") return false;
  const v = value as Record<string, unknown>;
  return (
    typeof v.coverageSummary === "object" &&
    v.coverageSummary !== null &&
    Array.isArray(v.missingAndRiskyFields) &&
    Array.isArray(v.callChecklist) &&
    typeof v.riskLevel === "string" &&
    typeof v.pmsPasteText === "string"
  );
}

export async function analyzeInsuranceWithGemini(
  insuranceText: string,
): Promise<AnalysisResult> {
  const apiKey = process.env.GEMINI_API_KEY;
  if (!apiKey) {
    throw new Error("GEMINI_API_KEY is not configured in .env.local");
  }

  const res = await fetch(`${GEMINI_URL}?key=${encodeURIComponent(apiKey)}`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
      systemInstruction: { parts: [{ text: EXTRACTION_SYSTEM_PROMPT }] },
      contents: [
        { role: "user", parts: [{ text: buildUserPrompt(insuranceText) }] },
      ],
      generationConfig: {
        temperature: 0,
        responseMimeType: "application/json",
      },
    }),
  });

  if (!res.ok) {
    const errText = await res.text();
    throw new Error(`Gemini API error ${res.status}: ${errText.slice(0, 300)}`);
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

  const parsed = extractJson(text);
  if (!isAnalysisResult(parsed)) {
    throw new Error("Gemini returned an unexpected shape");
  }

  return parsed;
}
