import { buildSystemPrompt, buildUserPrompt } from "./prompt.js";

const GEMINI_URL =
  "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent";

function extractJson(text) {
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

/**
 * @param {{ job: object, profile: object, apiKey: string }} args
 */
export async function generateApplicationAnswers({ job, profile, apiKey }) {
  if (!apiKey) throw new Error("Add your Gemini API key in Settings");

  const res = await fetch(`${GEMINI_URL}?key=${encodeURIComponent(apiKey)}`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
      systemInstruction: { parts: [{ text: buildSystemPrompt(profile) }] },
      contents: [
        { role: "user", parts: [{ text: buildUserPrompt(job, profile) }] },
      ],
      generationConfig: {
        temperature: 0.7,
        responseMimeType: "application/json",
      },
    }),
  });

  if (!res.ok) {
    const errText = await res.text().catch(() => "");
    throw new Error(`Gemini error ${res.status}: ${errText.slice(0, 200)}`);
  }

  const data = await res.json();
  const raw =
    data?.candidates?.[0]?.content?.parts?.map((p) => p.text).join("") || "";
  if (!raw) throw new Error("Empty response from Gemini");

  const parsed = extractJson(raw);

  // Normalize answers — ensure whyUs is always fillable
  const answers = Array.isArray(parsed.answers) ? parsed.answers : [];
  if (parsed.whyUs && !answers.some((a) => /interest|why/i.test(a.prompt || ""))) {
    answers.unshift({
      prompt: "What interests you about working for this company?",
      text: parsed.whyUs,
    });
  }

  return {
    whyUs: parsed.whyUs || answers[0]?.text || "",
    coverLetter: parsed.coverLetter || "",
    answers,
    highlightProjects: parsed.highlightProjects || [],
    resumeTips: parsed.resumeTips || [],
  };
}
