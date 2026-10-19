"use client";

import { useRouter } from "next/navigation";
import { useEffect, useRef, useState } from "react";
import { ImagePlus, Mic, MicOff } from "lucide-react";

import { AppShell } from "@/components/layout/app-shell";
import { Button, buttonVariants } from "@/components/ui/button";
import { Textarea } from "@/components/ui/textarea";
import {
  DEMO_CLOSEOUT_NARRATION,
  gapsForNotes,
} from "@/lib/closeout";
import {
  ensureSeedInspections,
  getLocalInspection,
  upsertLocalInspection,
} from "@/lib/inspections-storage";
import { PAGE_GUTTER } from "@/lib/layout";
import { cn } from "@/lib/utils";
import type { Inspection } from "@/types/inspection";
import Link from "next/link";

export function CloseoutCapture({ id }: { id: string }) {
  const router = useRouter();
  const fileRef = useRef<HTMLInputElement>(null);
  const [job, setJob] = useState<Inspection | null>(null);
  const [notes, setNotes] = useState("");
  const [dictating, setDictating] = useState(false);
  const [hint, setHint] = useState<string | null>(null);
  const [pending, setPending] = useState(false);
  const recognitionRef = useRef<{ stop: () => void } | null>(null);

  useEffect(() => {
    ensureSeedInspections();
    const row = getLocalInspection(id);
    setJob(row);
    if (row?.notes) setNotes(row.notes);
  }, [id]);

  function loadDemoNarration() {
    setNotes(DEMO_CLOSEOUT_NARRATION);
    setHint(null);
  }

  function toggleRecording() {
    type Rec = {
      continuous: boolean;
      interimResults: boolean;
      onresult: ((ev: { results: ArrayLike<{ 0: { transcript: string }; isFinal?: boolean }> }) => void) | null;
      onerror: (() => void) | null;
      onend: (() => void) | null;
      start: () => void;
      stop: () => void;
    };
    const W = window as unknown as {
      SpeechRecognition?: new () => Rec;
      webkitSpeechRecognition?: new () => Rec;
    };
    const Ctor = W.SpeechRecognition || W.webkitSpeechRecognition;

    if (dictating) {
      recognitionRef.current?.stop();
      recognitionRef.current = null;
      setDictating(false);
      setHint(null);
      return;
    }

    if (!Ctor) {
      setHint("Voice capture isn’t available here — paste or type notes, or load the demo narration.");
      return;
    }

    const recognition = new Ctor();
    recognition.continuous = true;
    recognition.interimResults = false;
    recognition.onresult = (ev) => {
      const piece = Array.from(ev.results)
        .map((r) => r[0].transcript)
        .join(" ")
        .trim();
      if (!piece) return;
      setNotes((prev) => (prev.trim() ? `${prev.trim()} ${piece}` : piece));
    };
    recognition.onerror = () => {
      setDictating(false);
      setHint("Recording stopped.");
    };
    recognition.onend = () => {
      setDictating(false);
      recognitionRef.current = null;
    };
    recognitionRef.current = recognition;
    recognition.start();
    setDictating(true);
    setHint("Listening… describe what you observed and still need confirmed.");
  }

  function handleCheckCloseout() {
    if (!job || notes.trim().length < 20 || pending) return;
    setPending(true);
    const gaps = gapsForNotes(notes);
    const next: Inspection = {
      ...job,
      notes: notes.trim(),
      status: "Needs clarification",
      closeoutGaps: gaps,
      clarifications: [],
      photoNames: job.photoNames ?? [],
    };
    upsertLocalInspection(next);
    router.push(`/jobs/${job.id}/gaps`);
  }

  if (!job) {
    return (
      <AppShell>
        <div className={cn(PAGE_GUTTER, "max-w-lg py-16 text-sm text-muted-foreground")}>
          Job not found.{" "}
          <Link href="/dashboard" className="text-primary underline">
            Back
          </Link>
        </div>
      </AppShell>
    );
  }

  return (
    <AppShell>
      <div className={cn(PAGE_GUTTER, "max-w-lg space-y-5 pt-6 pb-10")}>
        <div className="space-y-1.5">
          <p className="text-[11px] font-semibold tracking-wide text-muted-foreground uppercase">
            Field closeout
          </p>
          <h1 className="text-xl font-semibold tracking-tight text-navy sm:text-2xl">
            Finish your {job.siteName} inspection
          </h1>
          <p className="text-sm text-muted-foreground">
            Describe what you observed, tested, repaired, and still need
            confirmed.
          </p>
        </div>

        <div className="space-y-3">
          <Button
            type="button"
            size="lg"
            className={cn(
              "h-14 w-full rounded-md text-base",
              dictating && "bg-critical hover:bg-critical/90",
            )}
            onClick={toggleRecording}
          >
            {dictating ? (
              <MicOff className="size-5" />
            ) : (
              <Mic className="size-5" />
            )}
            {dictating ? "Stop recording" : "Start recording"}
          </Button>
          {hint ? (
            <p className="text-center text-xs text-muted-foreground">{hint}</p>
          ) : null}
        </div>

        <div className="space-y-2">
          <div className="flex items-center justify-between gap-2">
            <label className="text-sm font-medium" htmlFor="notes">
              Notes
            </label>
            <button
              type="button"
              className="text-xs font-medium text-muted-foreground hover:text-foreground"
              onClick={loadDemoNarration}
            >
              Load demo narration
            </button>
          </div>
          <Textarea
            id="notes"
            value={notes}
            onChange={(e) => setNotes(e.target.value)}
            className="min-h-[200px] rounded-md font-mono text-[13px] leading-relaxed"
            placeholder="Speak or type your closeout…"
          />
        </div>

        <div>
          <input
            ref={fileRef}
            type="file"
            accept="image/*"
            multiple
            className="hidden"
            onChange={(e) => {
              const names = e.target.files
                ? Array.from(e.target.files).map((f) => f.name)
                : [];
              if (!job) return;
              const next = {
                ...job,
                photoNames: [...(job.photoNames ?? []), ...names],
              };
              upsertLocalInspection(next);
              setJob(next);
            }}
          />
          <Button
            type="button"
            variant="outline"
            className="w-full rounded-md"
            onClick={() => fileRef.current?.click()}
          >
            <ImagePlus className="size-4" />
            Add photos
            {job.photoNames && job.photoNames.length > 0
              ? ` · ${job.photoNames.length}`
              : ""}
          </Button>
        </div>

        <div className="space-y-2 border-t border-border pt-4">
          <Button
            type="button"
            size="lg"
            className="h-12 w-full rounded-md text-base"
            disabled={notes.trim().length < 20 || pending}
            onClick={handleCheckCloseout}
          >
            {pending ? "Checking…" : "Check closeout"}
          </Button>
          <p className="text-center text-xs text-muted-foreground">
            Catch missing details before you leave the site.
          </p>
          <Link
            href={`/jobs/${job.id}`}
            className={cn(
              buttonVariants({ variant: "ghost", size: "sm" }),
              "mx-auto flex w-fit",
            )}
          >
            Back to job
          </Link>
        </div>
      </div>
    </AppShell>
  );
}
