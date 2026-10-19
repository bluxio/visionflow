# Insurance Verification QA

Single-page demo for dental offices. Paste messy insurance portal text and get:

1. **Coverage summary** — active status, annual max, remaining, deductible, preventive/basic/major %
2. **Missing & risky fields** — waiting periods, downgrades, missing tooth clause, frequencies, last service dates, pre-auth, code-specific coverage
3. **Call checklist** — exact questions to ask the payer
4. **PMS paste** — copy-to-clipboard block for chart notes

No auth, no database. Pasted text only (v1).

## Stack

- Next.js (App Router)
- Tailwind CSS
- Google Gemini API (`gemini-2.5-flash`)

## Setup

```bash
cd insurance-verification-qa
npm install
cp .env.example .env.local
```

Add your key to `.env.local`:

```
GEMINI_API_KEY=...
```

## Run

```bash
npm run dev
```

Open [http://localhost:3000](http://localhost:3000). Use **Load sample text** to try without real portal copy.

## Core prompt

Extraction logic lives in `src/lib/extraction-prompt.ts`. Iterate there when outputs are wrong — that is the product.
