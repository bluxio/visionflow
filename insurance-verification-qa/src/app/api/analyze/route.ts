import { NextResponse } from "next/server";
import { analyzeInsuranceWithGemini } from "@/lib/gemini-analyze";
import type { AnalysisResult } from "@/lib/types";

export async function POST(request: Request) {
  let body: { text?: string };
  try {
    body = await request.json();
  } catch {
    return NextResponse.json({ error: "Invalid JSON body" }, { status: 400 });
  }

  const text = body.text?.trim();
  if (!text) {
    return NextResponse.json(
      { error: "Paste insurance info before analyzing." },
      { status: 400 },
    );
  }

  if (text.length > 50_000) {
    return NextResponse.json(
      { error: "Text is too long. Paste under 50,000 characters." },
      { status: 400 },
    );
  }

  try {
    const result = await analyzeInsuranceWithGemini(text);
    return NextResponse.json(result satisfies AnalysisResult);
  } catch (error) {
    const message =
      error instanceof Error ? error.message : "Failed to analyze coverage";
    console.error("analyze error:", error);
    const status = message.includes("not configured") ? 500 : 502;
    return NextResponse.json({ error: message }, { status });
  }
}
