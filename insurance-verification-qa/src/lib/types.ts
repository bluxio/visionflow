export type RiskLevel = "low" | "medium" | "high";

export type FieldSeverity = "missing" | "ambiguous" | "risky";

export interface CoverageSummary {
  activeStatus: string | null;
  annualMax: string | null;
  remainingBenefits: string | null;
  deductible: string | null;
  preventivePercent: string | null;
  basicPercent: string | null;
  majorPercent: string | null;
}

export interface MissingOrRiskyField {
  field: string;
  severity: FieldSeverity;
  detail: string;
}

export interface AnalysisResult {
  coverageSummary: CoverageSummary;
  missingAndRiskyFields: MissingOrRiskyField[];
  callChecklist: string[];
  riskLevel: RiskLevel;
  riskRationale: string;
  pmsPasteText: string;
}
