# Axxiom InspectFlow

**From field inspection to complete follow-through.**

Case-study prototype for elevator field operations: capture a closeout, **catch missing details before the technician leaves**, verify with a manager, then prepare downstream drafts.

Differentiator: **Field Closeout** — targeted on-site clarification, not another FSM.

| | |
|---|---|
| **Company context** | Axxiom Elevator — inspection, maintenance, modernization, installation |
| **Hypothesis** | Incomplete technician closeouts create avoidable office follow-up / return trips |
| **Judgment signal** | Unknown stays unknown; humans approve before follow-up drafts |

---

## Core flow

```text
Assigned job
  → Field capture (voice / notes / photos)
  → Field Closeout gap detection  ← hero moment
  → Technician resolves what they can on site
  → Complete handoff submitted
  → Manager verification (evidence | clarifications | findings)
  → Approval
  → Draft work order / parts / customer update / inspection summary
```

Statuses: **On site** → **Needs clarification** → **Ready for review** → **Approved** → **Follow-up prepared**

---

## Loom path (2–3 min)

1. Open assigned Riverside Medical Center job (`/`)
2. Record closeout → load demo narration
3. Check closeout → **3 details needed before you leave**
4. Resolve two; leave one unresolved (e.g. Unable to confirm / Part unknown)
5. Submit closeout → manager review
6. Approve findings → Prepare follow-up drafts

---

## What it deliberately does **not** include

- Replacing an existing elevator field-service / ERP platform
- Any live vendor integration (this is a standalone prototype)
- Vendor ordering or autonomous compliance decisions
- Inventing completed safety tests

---

## Stack

- Next.js App Router + TypeScript + Tailwind
- Optional Gemini for ad-hoc structuring; Riverside demo gaps are deterministic
- MongoDB Atlas optional; localStorage `v4` seeds for demos

## Setup

```bash
cd axxiom-inspectflow
cp .env.example .env.local
npm install
npm run dev
```

Open [http://localhost:3000](http://localhost:3000) — starts on the assigned Riverside job.

**Live demo:** [https://axxiom-inspectflow.vercel.app](https://axxiom-inspectflow.vercel.app)

Use **Restart demo** on the assigned job (or **Reset demo data** on Operations) for a clean Loom take.
