import {
  getApiKey,
  setApiKey,
  getProfile,
  setProfile,
  getHistory,
  addHistoryEntry,
} from "../lib/storage.js";
import { generateApplicationAnswers } from "../lib/gemini.js";

const $ = (id) => document.getElementById(id);

let currentJob = null;
let currentGenerated = null;

function setStatus(text, kind = "") {
  const el = $("status");
  el.textContent = text || "";
  el.className = "status" + (kind ? ` ${kind}` : "");
}

async function ensureContentScripts(tabId) {
  try {
    await chrome.tabs.sendMessage(tabId, { type: "PING_EXTRACT" });
    return true;
  } catch {
    await chrome.scripting.executeScript({
      target: { tabId },
      files: ["content/extract.js", "content/autofill.js"],
    });
    return true;
  }
}

async function activeTab() {
  const [tab] = await chrome.tabs.query({ active: true, currentWindow: true });
  if (!tab?.id) throw new Error("No active tab");
  if (!tab.url || tab.url.startsWith("chrome://") || tab.url.startsWith("chrome-extension://")) {
    throw new Error("Open a job application page first (e.g. Wellfound)");
  }
  return tab;
}

function renderJob(job) {
  currentJob = job;
  const card = $("job-card");
  card.classList.remove("muted");
  card.innerHTML = `
    <h2>${escapeHtml(job.title || "Untitled role")}</h2>
    <div class="meta">${escapeHtml(job.company || "Company unknown")} · ${escapeHtml(job.source || "page")}</div>
    <div class="desc">${escapeHtml((job.description || "").slice(0, 280))}${(job.description || "").length > 280 ? "…" : ""}</div>
    <div class="meta">${(job.questions || []).length} question field(s) detected</div>
  `;
  $("btn-generate").disabled = false;
}

function renderAnswers(gen) {
  currentGenerated = gen;
  $("answers-wrap").hidden = false;
  $("why-us").value = gen.whyUs || "";
  $("cover-letter").value = gen.coverLetter || "";

  const extra = $("extra-answers");
  extra.innerHTML = "";
  for (const a of gen.answers || []) {
    if (/interest|why this company/i.test(a.prompt || "") && a.text === gen.whyUs) {
      continue;
    }
    const label = document.createElement("label");
    label.className = "field";
    label.innerHTML = `<span>${escapeHtml(a.prompt || "Answer")}</span>`;
    const ta = document.createElement("textarea");
    ta.rows = 4;
    ta.value = a.text || "";
    ta.dataset.prompt = a.prompt || "";
    label.appendChild(ta);
    extra.appendChild(label);
  }

  const tips = $("tips");
  const items = [
    ...(gen.highlightProjects || []).map((p) => `Highlight: ${p}`),
    ...(gen.resumeTips || []),
  ];
  tips.innerHTML = items.length
    ? `<ul class="tips">${items.map((t) => `<li>${escapeHtml(t)}</li>`).join("")}</ul>`
    : "";
}

function collectAnswersFromUi() {
  const answers = [
    {
      prompt: "What interests you about working for this company?",
      text: $("why-us").value.trim(),
    },
  ];
  const cover = $("cover-letter").value.trim();
  if (cover) {
    answers.push({ prompt: "Cover letter", text: cover });
  }
  for (const ta of $("extra-answers").querySelectorAll("textarea")) {
    answers.push({
      prompt: ta.dataset.prompt || "Question",
      text: ta.value.trim(),
    });
  }
  return answers;
}

function escapeHtml(s) {
  return String(s)
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;");
}

async function loadDefaultProfileJson() {
  const url = chrome.runtime.getURL("profile-default.json");
  const res = await fetch(url);
  return res.json();
}

async function initSettingsForm() {
  const key = await getApiKey();
  $("api-key").value = key;
  let profile = await getProfile();
  if (!profile) {
    profile = await loadDefaultProfileJson();
    await setProfile(profile);
  }
  $("profile-json").value = JSON.stringify(profile, null, 2);
}

async function renderHistory() {
  const history = await getHistory();
  const list = $("history-list");
  if (!history.length) {
    list.innerHTML = `<p class="empty">No saved applications yet.</p>`;
    return;
  }
  list.innerHTML = history
    .map(
      (h) => `
    <div class="history-item">
      <strong>${escapeHtml(h.title || "Role")} @ ${escapeHtml(h.company || "?")}</strong>
      <span>${escapeHtml((h.savedAt || "").slice(0, 10))} · ${escapeHtml(h.url || "")}</span>
    </div>`
    )
    .join("");
}

function showPanel(name) {
  $("panel-work").hidden = name !== "work";
  $("panel-history").hidden = name !== "history";
  $("panel-settings").hidden = name !== "settings";
  $("main-tabs").hidden = name === "settings";
  document.querySelectorAll(".tab").forEach((btn) => {
    btn.classList.toggle("active", btn.dataset.panel === name);
  });
  if (name === "history") renderHistory();
}

$("tab-settings").addEventListener("click", () => showPanel("settings"));
$("back-to-work").addEventListener("click", () => showPanel("work"));
document.querySelectorAll(".tab").forEach((btn) => {
  btn.addEventListener("click", () => showPanel(btn.dataset.panel));
});
$("main-tabs").hidden = false;

$("save-settings").addEventListener("click", async () => {
  try {
    await setApiKey($("api-key").value);
    const profile = JSON.parse($("profile-json").value);
    await setProfile(profile);
    setStatus("Settings saved.", "ok");
    showPanel("work");
  } catch (err) {
    setStatus(String(err.message || err), "err");
  }
});

$("load-default-profile").addEventListener("click", async () => {
  const profile = await loadDefaultProfileJson();
  $("profile-json").value = JSON.stringify(profile, null, 2);
  setStatus("Template loaded — click Save settings to keep it.", "ok");
});

$("btn-extract").addEventListener("click", async () => {
  setStatus("Reading page…");
  try {
    const tab = await activeTab();
    await ensureContentScripts(tab.id);
    const res = await chrome.tabs.sendMessage(tab.id, { type: "EXTRACT_JOB" });
    if (!res?.ok) throw new Error(res?.error || "Extract failed");
    renderJob(res.job);
    setStatus(
      `Extracted${res.job.company ? ` · ${res.job.company}` : ""}. Generate next.`,
      "ok"
    );
  } catch (err) {
    setStatus(String(err.message || err), "err");
  }
});

$("btn-generate").addEventListener("click", async () => {
  if (!currentJob) return;
  setStatus("Generating with Gemini…");
  $("btn-generate").disabled = true;
  try {
    const apiKey = await getApiKey();
    const profile = (await getProfile()) || (await loadDefaultProfileJson());
    const gen = await generateApplicationAnswers({
      job: currentJob,
      profile,
      apiKey,
    });
    renderAnswers(gen);
    setStatus("Draft ready — edit anything that doesn’t sound like you.", "ok");
  } catch (err) {
    setStatus(String(err.message || err), "err");
    showPanel("settings");
  } finally {
    $("btn-generate").disabled = !currentJob;
  }
});

$("btn-fill").addEventListener("click", async () => {
  setStatus("Filling fields…");
  try {
    const tab = await activeTab();
    await ensureContentScripts(tab.id);
    const profile = await getProfile();
    const answers = collectAnswersFromUi();
    const res = await chrome.tabs.sendMessage(tab.id, {
      type: "AUTOFILL",
      answers,
      profile,
    });
    if (!res?.ok) throw new Error(res?.error || "Autofill failed");
    setStatus(
      `Filled ${res.filledCount} answer field(s)` +
        (res.profileFieldsFilled
          ? `, ${res.profileFieldsFilled} profile field(s)`
          : "") +
        ". Review, then submit yourself.",
      "ok"
    );
  } catch (err) {
    setStatus(String(err.message || err), "err");
  }
});

$("btn-save").addEventListener("click", async () => {
  if (!currentJob) return;
  const answers = collectAnswersFromUi();
  await addHistoryEntry({
    company: currentJob.company,
    title: currentJob.title,
    url: currentJob.url,
    whyUs: $("why-us").value.trim(),
    answers,
  });
  setStatus("Saved to local history.", "ok");
});

$("btn-copy-why").addEventListener("click", async () => {
  const text = $("why-us").value.trim();
  if (!text) return;
  await navigator.clipboard.writeText(text);
  setStatus("Copied Why us.", "ok");
});

await initSettingsForm();
setStatus("Ready. Open a job page → Extract job.");
