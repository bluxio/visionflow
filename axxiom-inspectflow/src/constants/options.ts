export const INSPECTION_TYPES = [
  "Periodic inspection",
  "Acceptance / new install",
  "Maintenance check",
  "Callback / complaint",
  "Modernization survey",
] as const;

export type SampleIntake = {
  siteName: string;
  elevatorId: string;
  inspectionType: string;
  technicianName: string;
  notes: string;
};

/** Realistic field notes for demo — intentionally incomplete in places. */
export const SAMPLE_INSPECTION: SampleIntake = {
  siteName: "Riverside Medical Center — Tower B",
  elevatorId: "ELV-B-03",
  inspectionType: "Periodic inspection",
  technicianName: "Marcus Chen",
  notes: `Periodic cat-1 inspection on ELV-B-03 (passenger, ~12 landings). Cab ride ok, leveling within about 1/4" at most floors. Door open/close times look normal.

Issues noted:
- Machine room: slight oil seepage under hydraulic tank — wipe marks on floor, not pooling heavily. Couldn't confirm exact leak rate; may need follow-up.
- Hall station button at floor 7 sticky (Door Close). Needs replacement or clean/adjust — part number not confirmed on site.
- Pit: some debris, water staining on wall — no standing water today. Drain status unknown.
- Rope / sheave: visual only, no unusual wear observed from accessible vantage.
- Fire service Phase I recall tested OK from lobby key switch. Phase II in-car not fully verified — key switch felt loose, need confirmation with building engineer on key inventory.

Customer (facilities) asked for written update before end of week. Recommend work order for hall button + hydraulic leak check. Parts: hall button assembly TBD; absorbent pads / absorbent for machine room. No shutdown required unless leak worsens.

Did not inspect machine room HVAC filter condition (access blocked by stored materials — flagged for building).`,
};
