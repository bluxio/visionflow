export type ExerciseType = "squat" | "deadlift" | "bench_press" | "barbell_row";

export type Severity = "info" | "warning" | "critical";

export interface FormFeedback {
  aspect: string;
  score: number;
  feedback: string;
  severity: Severity;
}

export interface AnalyzeResponse {
  exercise_type: ExerciseType;
  overall_score: number;
  rep_count: number;
  feedback: FormFeedback[];
  recommendations: string[];
}

export interface HistoryItem {
  id: string;
  exercise_type: ExerciseType;
  overall_score: number;
  rep_count: number;
  severity_max: Severity;
  created_at: string;
}

export interface HistoryDetail extends HistoryItem {
  feedback: FormFeedback[];
  recommendations: string[];
  video_path?: string | null;
}

export interface QuotaError {
  error: {
    code: "quota_exceeded";
    message: string;
    limit: number;
    window_hours: number;
  };
}
