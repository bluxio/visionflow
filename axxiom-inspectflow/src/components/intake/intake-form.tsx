"use client";

import { useRouter } from "next/navigation";
import { useMemo, useRef, useState } from "react";
import { ChevronDown, ImagePlus, Mic, MicOff } from "lucide-react";

import { AppShell } from "@/components/layout/app-shell";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select";
import { Textarea } from "@/components/ui/textarea";
import { INSPECTION_TYPES, SAMPLE_INSPECTION } from "@/constants/options";
import { submitInspectionExtract } from "@/lib/inspections-api-client";
import { upsertLocalInspection } from "@/lib/inspections-storage";
import { PAGE_GUTTER } from "@/lib/layout";
import { cn } from "@/lib/utils";

export function IntakeForm() {
  const router = useRouter();
  const fileRef = useRef<HTMLInputElement>(null);
  const [pending, setPending] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [demoOpen, setDemoOpen] = useState(false);
  const [dictating, setDictating] = useState(false);
  const [dictateNote, setDictateNote] = useState<string | null>(null);
  const recognitionRef = useRef<{ stop: () => void } | null>(null);

  const [siteName, setSiteName] = useState("");
  const [elevatorId, setElevatorId] = useState("");
  const [inspectionType, setInspectionType] = useState<string>(
    INSPECTION_TYPES[0],
  );
  const [technicianName, setTechnicianName] = useState("");
  const [notes, setNotes] = useState("");
  const [photos, setPhotos] = useState<File[]>([]);

  const canSubmit = useMemo(() => {
    return (
      siteName.trim().length > 1 &&
      elevatorId.trim().length > 0 &&
      technicianName.trim().length > 1 &&
      notes.trim().length > 20
    );
  }, [siteName, elevatorId, technicianName, notes]);

  function loadSample() {
    setSiteName(SAMPLE_INSPECTION.siteName);
    setElevatorId(SAMPLE_INSPECTION.elevatorId);
    setInspectionType(SAMPLE_INSPECTION.inspectionType);
    setTechnicianName(SAMPLE_INSPECTION.technicianName);
    setNotes(SAMPLE_INSPECTION.notes);
    setError(null);
    setDemoOpen(false);
  }

  function toggleDictate() {
    const SpeechRecognition =
      typeof window !== "undefined"
        ? (
            window as unknown as {
              SpeechRecognition?: new () => {
                continuous: boolean;
                interimResults: boolean;
                onresult: ((ev: { results: ArrayLike<{ 0: { transcript: string } }> }) => void) | null;
                onerror: (() => void) | null;
                onend: (() => void) | null;
                start: () => void;
                stop: () => void;
              };
              webkitSpeechRecognition?: new () => {
                continuous: boolean;
                interimResults: boolean;
                onresult: ((ev: { results: ArrayLike<{ 0: { transcript: string } }> }) => void) | null;
                onerror: (() => void) | null;
                onend: (() => void) | null;
                start: () => void;
                stop: () => void;
              };
            }
          ).SpeechRecognition ||
          (
            window as unknown as {
              webkitSpeechRecognition?: new () => {
                continuous: boolean;
                interimResults: boolean;
                onresult: ((ev: { results: ArrayLike<{ 0: { transcript: string } }> }) => void) | null;
                onerror: (() => void) | null;
                onend: (() => void) | null;
                start: () => void;
                stop: () => void;
              };
            }
          ).webkitSpeechRecognition
        : undefined;

    if (dictating) {
      recognitionRef.current?.stop();
      recognitionRef.current = null;
      setDictating(false);
      setDictateNote(null);
      return;
    }

    if (!SpeechRecognition) {
      setDictateNote(
        "Dictation isn’t available in this browser. Type notes or paste from your recorder.",
      );
      return;
    }

    const recognition = new SpeechRecognition();
    recognition.continuous = true;
    recognition.interimResults = true;
    recognition.onresult = (ev) => {
      const chunk = Array.from(ev.results)
        .map((r) => r[0].transcript)
        .join(" ");
      setNotes((prev) => {
        const base = prev.trim();
        const next = chunk.trim();
        if (!next) return prev;
        return base ? `${base} ${next}` : next;
      });
    };
    recognition.onerror = () => {
      setDictating(false);
      setDictateNote("Dictation stopped.");
    };
    recognition.onend = () => {
      setDictating(false);
      recognitionRef.current = null;
    };
    recognitionRef.current = recognition;
    recognition.start();
    setDictating(true);
    setDictateNote("Listening… speak your field notes.");
  }

  async function handleSubmit(e: React.FormEvent) {
    e.preventDefault();
    if (!canSubmit || pending) return;
    setPending(true);
    setError(null);

    try {
      const result = await submitInspectionExtract({
        siteName: siteName.trim(),
        elevatorId: elevatorId.trim(),
        inspectionType,
        technicianName: technicianName.trim(),
        notes: notes.trim(),
      });
      upsertLocalInspection(result.inspection);
      router.push(`/inspections/${result.inspection.id}`);
    } catch (err) {
      setError(err instanceof Error ? err.message : "Could not submit inspection");
      setPending(false);
    }
  }

  return (
    <AppShell>
      <div className={cn(PAGE_GUTTER, "max-w-3xl pt-8")}>
        <div className="mb-6 flex flex-wrap items-start justify-between gap-3">
          <div className="space-y-1.5">
            <h1 className="text-2xl font-semibold tracking-tight text-navy">
              Log field inspection
            </h1>
            <p className="max-w-xl text-sm text-muted-foreground">
              Capture what happened on site. A manager will review the findings
              before any action is created.
            </p>
          </div>
          <div className="relative">
            <Button
              type="button"
              variant="ghost"
              size="sm"
              className="text-muted-foreground"
              onClick={() => setDemoOpen((o) => !o)}
            >
              Demo data
              <ChevronDown className="size-3.5" />
            </Button>
            {demoOpen ? (
              <div className="absolute right-0 z-20 mt-1 w-52 rounded-md border border-border bg-card py-1 text-sm">
                <button
                  type="button"
                  className="block w-full px-3 py-2 text-left hover:bg-muted"
                  onClick={loadSample}
                >
                  Load sample inspection
                </button>
              </div>
            ) : null}
          </div>
        </div>

        <form
          onSubmit={handleSubmit}
          className="space-y-5 rounded-md border border-border bg-card p-5 sm:p-6"
        >
          <div className="grid gap-4 sm:grid-cols-2">
            <div className="space-y-1.5 sm:col-span-2">
              <Label htmlFor="siteName">Site / customer</Label>
              <Input
                id="siteName"
                value={siteName}
                onChange={(e) => setSiteName(e.target.value)}
                placeholder="Building or campus name"
                disabled={pending}
                className="rounded-md"
              />
            </div>
            <div className="space-y-1.5">
              <Label htmlFor="elevatorId">Elevator ID</Label>
              <Input
                id="elevatorId"
                value={elevatorId}
                onChange={(e) => setElevatorId(e.target.value)}
                placeholder="e.g. ELV-B-03"
                disabled={pending}
                className="rounded-md font-mono tabular"
              />
            </div>
            <div className="space-y-1.5">
              <Label htmlFor="inspectionType">Inspection type</Label>
              <Select
                value={inspectionType}
                onValueChange={(v) => {
                  if (v) setInspectionType(v);
                }}
                disabled={pending}
              >
                <SelectTrigger id="inspectionType" className="w-full rounded-md">
                  <SelectValue />
                </SelectTrigger>
                <SelectContent>
                  {INSPECTION_TYPES.map((t) => (
                    <SelectItem key={t} value={t}>
                      {t}
                    </SelectItem>
                  ))}
                </SelectContent>
              </Select>
            </div>
            <div className="space-y-1.5 sm:col-span-2">
              <Label htmlFor="technicianName">Technician</Label>
              <Input
                id="technicianName"
                value={technicianName}
                onChange={(e) => setTechnicianName(e.target.value)}
                placeholder="Field technician name"
                disabled={pending}
                className="rounded-md"
              />
            </div>
          </div>

          <div className="space-y-2">
            <div className="flex flex-wrap items-center justify-between gap-2">
              <Label htmlFor="notes">Field notes</Label>
              <Button
                type="button"
                variant={dictating ? "secondary" : "outline"}
                size="sm"
                onClick={toggleDictate}
                disabled={pending}
                className={cn(
                  "rounded-md",
                  dictating && "border-primary text-primary",
                )}
              >
                {dictating ? (
                  <MicOff className="size-3.5" />
                ) : (
                  <Mic className="size-3.5" />
                )}
                {dictating ? "Stop dictation" : "Dictate notes"}
              </Button>
            </div>
            {dictateNote ? (
              <p className="text-xs text-muted-foreground">{dictateNote}</p>
            ) : null}
            <Textarea
              id="notes"
              value={notes}
              onChange={(e) => setNotes(e.target.value)}
              placeholder="What you observed, tested, and still need confirmed…"
              className="min-h-[200px] rounded-md font-mono text-[13px] leading-relaxed"
              disabled={pending}
            />
            <p className="text-xs text-muted-foreground">
              Turn rough field notes into a review-ready inspection.
            </p>
          </div>

          <div className="space-y-2">
            <Label>Photos (optional)</Label>
            <input
              ref={fileRef}
              type="file"
              accept="image/*"
              multiple
              className="hidden"
              onChange={(e) => {
                const list = e.target.files ? Array.from(e.target.files) : [];
                setPhotos(list);
              }}
            />
            <button
              type="button"
              onClick={() => fileRef.current?.click()}
              disabled={pending}
              className="flex w-full items-center justify-center gap-2 rounded-md border border-dashed border-border bg-muted/40 px-3 py-6 text-sm text-muted-foreground transition-colors hover:bg-muted/70 hover:text-foreground"
            >
              <ImagePlus className="size-4" />
              Attach site photos
            </button>
            {photos.length > 0 ? (
              <ul className="space-y-1 text-xs text-muted-foreground">
                {photos.map((f) => (
                  <li key={f.name + f.size} className="truncate">
                    {f.name}
                  </li>
                ))}
              </ul>
            ) : null}
          </div>

          {error ? (
            <p className="rounded-md border border-critical/30 bg-critical/5 px-3 py-2 text-sm text-critical">
              {error}
            </p>
          ) : null}

          <div className="space-y-2 border-t border-border pt-4">
            <Button type="submit" disabled={!canSubmit || pending} className="rounded-md">
              {pending ? "Preparing review…" : "Send for review"}
            </Button>
            <p className="text-xs text-muted-foreground">
              You will review the findings before any action is created.
            </p>
          </div>
        </form>
      </div>
    </AppShell>
  );
}
