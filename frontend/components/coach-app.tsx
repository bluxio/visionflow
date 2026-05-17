"use client";

import { useCallback, useEffect, useState } from "react";
import { analyzeUpload, fetchHistory, fetchHistoryDetail } from "@/lib/api";
import type { AnalyzeResponse, ExerciseType, HistoryDetail, HistoryItem } from "@/lib/types";
import { AnalysisResult } from "./analysis-result";
import { HistoryModal } from "./history-modal";
import { HistoryPanel } from "./history-panel";
import { AnalysisErrorCard, QuotaExceededCard } from "./status-cards";
import { UploadForm } from "./upload-form";

export function CoachApp() {
  const [exerciseType, setExerciseType] = useState<ExerciseType>("squat");
  const [file, setFile] = useState<File | null>(null);
  const [loading, setLoading] = useState(false);
  const [result, setResult] = useState<AnalyzeResponse | null>(null);
  const [history, setHistory] = useState<HistoryItem[]>([]);
  const [historyLoading, setHistoryLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [quota, setQuota] = useState<{ message: string; limit: number; windowHours: number } | null>(
    null,
  );
  const [modalId, setModalId] = useState<string | null>(null);
  const [modalDetail, setModalDetail] = useState<HistoryDetail | null>(null);
  const [modalLoading, setModalLoading] = useState(false);

  const loadHistory = useCallback(async () => {
    setHistoryLoading(true);
    try {
      const items = await fetchHistory();
      setHistory(items);
    } catch {
      setHistory([]);
    } finally {
      setHistoryLoading(false);
    }
  }, []);

  useEffect(() => {
    loadHistory();
  }, [loadHistory]);

  const handleAnalyze = async () => {
    if (!file) return;
    setLoading(true);
    setError(null);
    setQuota(null);
    setResult(null);

    try {
      const data = await analyzeUpload(file, exerciseType);
      setResult(data);
      await loadHistory();
    } catch (err) {
      const e = err as Error & {
        quota?: { message: string; limit: number; window_hours?: number };
      };
      if (e.quota) {
        setQuota({
          message: e.quota.message,
          limit: e.quota.limit,
          windowHours: e.quota.window_hours ?? 24,
        });
      } else {
        setError(e.message || "Analysis failed");
      }
    } finally {
      setLoading(false);
    }
  };

  const openHistoryItem = async (id: string) => {
    setModalId(id);
    setModalDetail(null);
    setModalLoading(true);
    try {
      const detail = await fetchHistoryDetail(id);
      setModalDetail(detail);
    } catch (err) {
      setError((err as Error).message);
      setModalId(null);
    } finally {
      setModalLoading(false);
    }
  };

  return (
    <div className="mx-auto min-h-screen max-w-lg px-4 py-10">
      <header className="mb-10">
        <p className="text-xs font-medium uppercase tracking-[0.2em] text-[#33AFFF]">
          Performance Lab
        </p>
        <h1 className="mt-2 text-2xl font-semibold tracking-tight text-[#E2E8F0]">
          Workout Form Coach
        </h1>
        <p className="mt-2 text-sm text-[#A0AEC0]">
          Upload a set video. Get rep counts, form scores, and coaching cues.
        </p>
      </header>

      <div className="space-y-6">
        <UploadForm
          exerciseType={exerciseType}
          onExerciseChange={setExerciseType}
          onFileChange={setFile}
          onAnalyze={handleAnalyze}
          loading={loading}
          fileName={file?.name ?? null}
        />

        {loading && (
          <div className="rounded-xl border border-[#1E2635] bg-[#111826] p-6 text-center text-sm text-[#A0AEC0]">
            Running analysis… this may take a moment for squat videos.
          </div>
        )}

        {quota && (
          <QuotaExceededCard
            message={quota.message}
            limit={quota.limit}
            windowHours={quota.windowHours}
          />
        )}

        {error && <AnalysisErrorCard message={error} />}

        {result && <AnalysisResult result={result} />}

        <HistoryPanel
          items={history}
          loading={historyLoading}
          onSelect={openHistoryItem}
        />
      </div>

      <HistoryModal
        detail={modalDetail}
        loading={modalLoading && !!modalId}
        onClose={() => {
          setModalId(null);
          setModalDetail(null);
        }}
      />
    </div>
  );
}
