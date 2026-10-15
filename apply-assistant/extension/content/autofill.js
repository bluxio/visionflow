/**
 * Apply Assistant — autofill text fields (never auto-submits)
 */

(() => {
  if (window.__applyAssistantFill) return;

  function setNativeValue(el, value) {
    const proto =
      el.tagName === "TEXTAREA"
        ? window.HTMLTextAreaElement.prototype
        : window.HTMLInputElement.prototype;
    const setter = Object.getOwnPropertyDescriptor(proto, "value")?.set;
    if (setter) setter.call(el, value);
    else el.value = value;

    el.dispatchEvent(new Event("input", { bubbles: true }));
    el.dispatchEvent(new Event("change", { bubbles: true }));
  }

  function scoreField(el, needle) {
    const blob = [
      el.name,
      el.id,
      el.placeholder,
      el.getAttribute("aria-label"),
      el.closest("label")?.innerText,
      document.querySelector(`label[for="${el.id}"]`)?.innerText,
    ]
      .filter(Boolean)
      .join(" ")
      .toLowerCase();

    const n = needle.toLowerCase();
    let score = 0;
    for (const word of n.split(/\W+/).filter((w) => w.length > 3)) {
      if (blob.includes(word)) score += 1;
    }
    if (blob.includes("why") && n.includes("interest")) score += 2;
    if (blob.includes("cover") && n.includes("cover")) score += 3;
    return score;
  }

  function fillAnswers(answers) {
    const fields = [
      ...document.querySelectorAll("textarea"),
      ...document.querySelectorAll(
        "input[type='text'], input:not([type]), input[type='search']"
      ),
    ].filter((el) => !el.disabled && el.type !== "hidden" && el.offsetParent);

    const used = new Set();
    const filled = [];

    for (const answer of answers) {
      const prompt = answer.prompt || "";
      const text = answer.text || "";
      if (!text) continue;

      let best = null;
      let bestScore = 0;
      for (const el of fields) {
        if (used.has(el)) continue;
        const s = scoreField(el, prompt);
        if (s > bestScore) {
          bestScore = s;
          best = el;
        }
      }

      // Fallback: first empty textarea
      if (!best || bestScore === 0) {
        best = fields.find(
          (el) => !used.has(el) && el.tagName === "TEXTAREA" && !el.value
        );
      }

      if (best) {
        setNativeValue(best, text);
        used.add(best);
        filled.push({ prompt, length: text.length });
      }
    }

    return { filledCount: filled.length, filled };
  }

  function fillProfile(profile) {
    const map = [
      { keys: ["email", "e-mail"], value: profile.email },
      { keys: ["phone", "mobile", "tel"], value: profile.phone },
      { keys: ["name", "full name"], value: profile.name },
      { keys: ["linkedin"], value: profile.linkedin },
      { keys: ["github"], value: profile.github },
      { keys: ["portfolio", "website", "personal site"], value: profile.portfolio },
      { keys: ["location", "city"], value: profile.location },
    ];

    const inputs = [
      ...document.querySelectorAll("input[type='text'], input[type='email'], input[type='tel'], input[type='url'], input:not([type])"),
    ];

    let count = 0;
    for (const { keys, value } of map) {
      if (!value) continue;
      for (const el of inputs) {
        const blob = `${el.name} ${el.id} ${el.placeholder} ${el.getAttribute("aria-label") || ""}`.toLowerCase();
        if (keys.some((k) => blob.includes(k)) && !el.value) {
          setNativeValue(el, value);
          count += 1;
          break;
        }
      }
    }
    return count;
  }

  window.__applyAssistantFill = { fillAnswers, fillProfile };

  chrome.runtime.onMessage.addListener((msg, _sender, sendResponse) => {
    if (msg?.type === "AUTOFILL") {
      try {
        const profileCount = msg.profile
          ? fillProfile(msg.profile)
          : 0;
        const result = fillAnswers(msg.answers || []);
        sendResponse({
          ok: true,
          ...result,
          profileFieldsFilled: profileCount,
        });
      } catch (err) {
        sendResponse({ ok: false, error: String(err?.message || err) });
      }
      return true;
    }
  });
})();
