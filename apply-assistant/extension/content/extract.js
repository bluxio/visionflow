/**
 * Apply Assistant — page extraction
 * Works best on Wellfound; falls back to generic heuristics elsewhere.
 */

(() => {
  if (window.__applyAssistantExtract) return;

  function textOf(el) {
    return (el?.innerText || el?.textContent || "").replace(/\s+/g, " ").trim();
  }

  function firstMatch(selectors) {
    for (const sel of selectors) {
      const el = document.querySelector(sel);
      if (el && textOf(el)) return el;
    }
    return null;
  }

  function uniqueTexts(nodes) {
    const seen = new Set();
    const out = [];
    for (const n of nodes) {
      const t = textOf(n);
      if (!t || t.length < 8 || seen.has(t)) continue;
      seen.add(t);
      out.push(t);
    }
    return out;
  }

  function extractQuestions() {
    const questions = [];
    const labels = document.querySelectorAll(
      "label, [class*='question'], [data-testid*='question']"
    );

    for (const label of labels) {
      const t = textOf(label);
      if (!t || t.length < 10 || t.length > 280) continue;
      const lower = t.toLowerCase();
      if (
        lower.includes("interest") ||
        lower.includes("why") ||
        lower.includes("about") ||
        lower.includes("tell us") ||
        lower.includes("cover letter") ||
        lower.includes("experience") ||
        label.htmlFor
      ) {
        const inputId = label.getAttribute("for");
        const input = inputId
          ? document.getElementById(inputId)
          : label.querySelector("textarea, input[type='text']") ||
            label.parentElement?.querySelector("textarea, input[type='text']");

        questions.push({
          prompt: t,
          fieldHint: input
            ? input.tagName.toLowerCase() +
              (input.name ? `[name=${input.name}]` : "")
            : "unknown",
        });
      }
    }

    // Wellfound-style: textareas near headings
    document.querySelectorAll("textarea").forEach((ta, i) => {
      const nearby =
        ta.getAttribute("aria-label") ||
        ta.getAttribute("placeholder") ||
        ta.closest("div")?.querySelector("label, h2, h3, p")?.innerText ||
        `Open text field ${i + 1}`;
      const prompt = nearby.replace(/\s+/g, " ").trim();
      if (!questions.some((q) => q.prompt === prompt)) {
        questions.push({
          prompt,
          fieldHint: `textarea[${ta.name || i}]`,
        });
      }
    });

    return questions.slice(0, 8);
  }

  function extractWellfound() {
    const title =
      textOf(
        firstMatch([
          "h1",
          "[data-test='JobTitle']",
          "[class*='JobTitle']",
          "h2",
        ])
      ) || document.title;

    const company =
      textOf(
        firstMatch([
          "[data-test='StartupName']",
          "a[href*='/company/']",
          "[class*='company'] a",
          "[class*='CompanyName']",
        ])
      ) || "";

    const aboutBlocks = uniqueTexts(
      document.querySelectorAll(
        "[class*='job-description'], [class*='JobDescription'], [data-test*='description'], article, [class*='About'], section"
      )
    );

    let description = aboutBlocks
      .sort((a, b) => b.length - a.length)[0] || "";

    if (description.length > 6000) {
      description = description.slice(0, 6000) + "…";
    }

    // Prefer body text around "About the job" if present
    const body = textOf(document.body);
    const aboutIdx = body.toLowerCase().indexOf("about the job");
    if (aboutIdx >= 0) {
      const slice = body.slice(aboutIdx, aboutIdx + 4500);
      if (slice.length > description.length * 0.5) description = slice;
    }

    return {
      source: "wellfound",
      url: location.href,
      title,
      company,
      description,
      questions: extractQuestions(),
      extractedAt: new Date().toISOString(),
    };
  }

  function extractGeneric() {
    const title =
      textOf(firstMatch(["h1", "[class*='job-title']", "[class*='JobTitle']"])) ||
      document.title;
    const company = textOf(
      firstMatch([
        "[class*='company-name']",
        "[class*='CompanyName']",
        "a[href*='company']",
      ])
    );
    let description = textOf(
      firstMatch([
        "[class*='description']",
        "[class*='job-details']",
        "main",
        "article",
      ])
    );
    if (description.length > 6000) {
      description = description.slice(0, 6000) + "…";
    }

    return {
      source: "generic",
      url: location.href,
      title,
      company,
      description,
      questions: extractQuestions(),
      extractedAt: new Date().toISOString(),
    };
  }

  function extractJob() {
    const host = location.hostname;
    if (host.includes("wellfound") || host.includes("angel.co")) {
      return extractWellfound();
    }
    return extractGeneric();
  }

  window.__applyAssistantExtract = extractJob;

  chrome.runtime.onMessage.addListener((msg, _sender, sendResponse) => {
    if (msg?.type === "PING_EXTRACT") {
      sendResponse({ ok: true });
      return true;
    }
    if (msg?.type === "EXTRACT_JOB") {
      try {
        sendResponse({ ok: true, job: extractJob() });
      } catch (err) {
        sendResponse({ ok: false, error: String(err?.message || err) });
      }
      return true;
    }
  });
})();
