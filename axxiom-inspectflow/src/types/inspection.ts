export const INSPECTION_STATUSES = [
  "On site",
  "Needs clarification",
  "Ready for review",
  "Approved",
  "Follow-up prepared",
] as const;

export type InspectionStatus = (typeof INSPECTION_STATUSES)[number];

export const URGENCY_LEVELS = [
  "low",
  "medium",
  "high",
  "critical",
] as const;

export type UrgencyLevel = (typeof URGENCY_LEVELS)[number];

export const SEVERITY_LEVELS = [
  "low",
  "medium",
  "high",
  "critical",
] as const;

export type SeverityLevel = (typeof SEVERITY_LEVELS)[number];

export type Finding = {
  issue: string;
  severity: SeverityLevel;
  evidenceFromNotes: string;
};

export type ExtractionResult = {
  summary: string;
  findings: Finding[];
  partsMentioned: string[];
  recommendedNextActions: string[];
  missingInformation: string[];
  overallUrgency: UrgencyLevel;
  confidence: number;
};

export type GeneratedDrafts = {
  workOrder: string;
  partsRequest: string;
  customerUpdate: string;
  inspectionSummary: string;
  generatedAt: string;
};

export type GapOption = {
  id: string;
  label: string;
  unresolved?: boolean;
};

export type CloseoutGap = {
  id: string;
  question: string;
  why: string;
  /** Missing info the tech can clarify now vs. an inspection action that still needs follow-up. */
  kind: "can_resolve" | "requires_action";
  options: GapOption[];
  allowNote?: boolean;
};

export type ClarificationAnswer = {
  gapId: string;
  question: string;
  answerLabel: string;
  note?: string;
  unresolved: boolean;
};

export type EquipmentHistory = {
  lastInspection: string;
  lastService: string;
  knownOpenItem: string;
  equipmentType: string;
};

export type InspectionIntake = {
  siteName: string;
  elevatorId: string;
  inspectionType: string;
  technicianName: string;
  notes: string;
};

export type Inspection = InspectionIntake & {
  id: string;
  extraction: ExtractionResult | null;
  status: InspectionStatus;
  drafts: GeneratedDrafts | null;
  createdAt: string;
  reviewedAt?: string;
  approvedAt?: string;
  extractionSource: "gemini" | "mock" | "none";
  workOrderNumber?: string;
  scheduledLabel?: string;
  building?: string;
  equipmentHistory?: EquipmentHistory;
  closeoutGaps?: CloseoutGap[];
  clarifications?: ClarificationAnswer[];
  photoNames?: string[];
  unresolvedForManager?: number;
};
