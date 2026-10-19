# Apply Assistant

Personal Chrome extension that helps you apply to startup / AI roles **without** becoming another Jobright.

**Draft → review → autofill → you submit.**

Built for your own search first. If it ever becomes a product, the core loop is already the right one.

## What it does

1. **Extract** — reads job title, company, description, and open-text questions from the page (Wellfound first; works on other sites with heuristics).
2. **Generate** — uses Gemini + your profile JSON (projects, voice, skills) to draft:
   - Why this company?
   - Cover letter (optional)
   - Other short answers
   - Resume highlight tips
3. **Autofill** — fills text fields. **Never clicks Submit.**
4. **History** — saves company / role / answers locally in `chrome.storage`.

## Setup (2 minutes)

1. Get a free [Gemini API key](https://aistudio.google.com/apikey).
2. Chrome → `chrome://extensions` → enable **Developer mode**.
3. **Load unpacked** → select this folder:

```text
apply-assistant/extension
```

4. Click the extension icon → ⚙ Settings:
   - Paste Gemini API key
   - Fill blank fields in profile JSON (`email`, `school`, links, sponsorship note)
   - Save

5. Open a Wellfound job application → **Extract job** → **Generate answers** → edit → **Autofill form** → submit yourself.

## Your profile

Editable sources:

| File | Purpose |
|------|---------|
| `profile/bolu.json` | Canonical profile you edit in the repo |
| Extension Settings → profile JSON | Live copy in Chrome storage (what the model uses) |

After editing `profile/bolu.json`, either paste it into Settings or click **Reset profile template** then Save (template ships as `extension/profile-default.json` — copy over after big edits):

```bash
cp profile/bolu.json extension/profile-default.json
```

Then reload the extension on `chrome://extensions`.

## Folder layout

```text
apply-assistant/
  profile/bolu.json          # your context
  extension/                 # load this in Chrome
    manifest.json
    popup/                   # UI
    content/                 # extract + autofill
    lib/                     # Gemini + prompts + storage
    profile-default.json
```

## Intentional non-goals (for now)

- No mass auto-apply
- No job board / matching engine
- No cloud sync / accounts
- No “always submit for me”

Those are how tools get bloated. Add features only when *you* hit a real pain while applying.

## Roadmap (only if you need it)

- Better Wellfound selectors as their DOM shifts
- Multi-resume picker
- Learn-from-edits (save your rewrites as few-shot examples)
- YC Work at a Startup + company-site application forms
- Optional: package for other students later

## Privacy

- API key and history stay in `chrome.storage.local` on your machine.
- Job text + profile are sent to Gemini only when you click **Generate answers**.

## Business later?

Maybe. The wedge isn’t “AI auto-apply.” It’s **high-quality startup applications in your voice, with human review, absurdly fast.** Prove it saves *you* 30–60 minutes/day first.
