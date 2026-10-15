export function buildSystemPrompt(profile) {
  const projects = (profile.projects || [])
    .map(
      (p) =>
        `- ${p.name}: ${p.summary}\n  Stack: ${(p.stack || []).join(", ")}\n  Bullets:\n${(p.bullets || []).map((b) => `    • ${b}`).join("\n")}`
    )
    .join("\n");

  const avoid = (profile.voice?.avoid || []).join("; ");
  const prefer = (profile.voice?.prefer || []).join("; ");

  return `You write job application answers for ${profile.name || "the candidate"}.
Location: ${profile.location || "US"}.
Target roles: ${(profile.targetRoles || []).join(", ")}.
Skills: ${(profile.skills || []).join(", ")}.

Projects:
${projects || "(none listed)"}

Voice: ${profile.voice?.tone || "direct and specific"}.
Avoid: ${avoid || "generic fluff"}.
Prefer: ${prefer || "concrete evidence"}.

Rules:
- Sound like a real person, not ChatGPT.
- Reference 1–2 specific projects when relevant.
- Keep "Why this company?" to 3–5 sentences.
- Cover letters: 150–220 words max.
- Short answers: 2–4 sentences unless the question needs more.
- Never invent employers, degrees, or metrics.
- Never claim equity/visa details unless in the profile.
- Return ONLY valid JSON.`;
}

export function buildUserPrompt(job, profile) {
  const existingQs =
    job.questions?.length > 0
      ? job.questions.map((q, i) => `${i + 1}. ${q.prompt}`).join("\n")
      : "1. What interests you about working for this company?";

  return `Job posting:
Company: ${job.company || "Unknown"}
Title: ${job.title || "Unknown"}
URL: ${job.url || ""}
Description:
${job.description || "(no description extracted)"}

Application questions to answer:
${existingQs}

Candidate defaults:
${JSON.stringify(profile.defaultAnswers || {}, null, 2)}

Return JSON with this shape:
{
  "whyUs": "string — answer for interest / why this company",
  "coverLetter": "string — short cover letter if useful, else empty string",
  "answers": [
    { "prompt": "exact question text", "text": "tailored answer" }
  ],
  "highlightProjects": ["project names to emphasize"],
  "resumeTips": ["1-3 bullet suggestions tailored to this role"]
}`;
}
