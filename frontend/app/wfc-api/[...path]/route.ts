import { NextRequest, NextResponse } from "next/server";

/** Fallback when BACKEND_URL is not set on Vercel (rewrites bake localhost at build time). */
const DEFAULT_BACKEND = "https://workout-form-coach-api.onrender.com";

export const runtime = "nodejs";
export const maxDuration = 60;

function backendUrl(): string {
  return (
    process.env.BACKEND_URL?.replace(/\/$/, "") ||
    process.env.NEXT_PUBLIC_API_URL?.replace(/\/$/, "") ||
    DEFAULT_BACKEND
  );
}

async function proxy(
  req: NextRequest,
  context: { params: Promise<{ path: string[] }> },
): Promise<NextResponse> {
  const { path } = await context.params;
  const segment = path?.join("/") ?? "";
  const target = `${backendUrl()}/${segment}${req.nextUrl.search}`;

  const headers = new Headers();
  const clientId = req.headers.get("x-client-id");
  if (clientId) headers.set("X-Client-Id", clientId);

  const hasBody = req.method !== "GET" && req.method !== "HEAD";

  try {
    const res = await fetch(target, {
      method: req.method,
      headers,
      body: hasBody ? await req.arrayBuffer() : undefined,
    });

    const outHeaders = new Headers();
    const contentType = res.headers.get("content-type");
    if (contentType) outHeaders.set("content-type", contentType);

    return new NextResponse(res.body, {
      status: res.status,
      statusText: res.statusText,
      headers: outHeaders,
    });
  } catch {
    return NextResponse.json(
      {
        detail:
          "Backend unreachable. Set BACKEND_URL on Vercel or check Render service status.",
      },
      { status: 502 },
    );
  }
}

export const GET = proxy;
export const POST = proxy;
export const PUT = proxy;
export const PATCH = proxy;
export const DELETE = proxy;
