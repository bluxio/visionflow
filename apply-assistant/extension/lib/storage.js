const STORAGE_KEYS = {
  apiKey: "applyAssistant.geminiApiKey",
  profile: "applyAssistant.profile",
  history: "applyAssistant.history",
};

export async function getApiKey() {
  const data = await chrome.storage.local.get(STORAGE_KEYS.apiKey);
  return data[STORAGE_KEYS.apiKey] || "";
}

export async function setApiKey(key) {
  await chrome.storage.local.set({ [STORAGE_KEYS.apiKey]: key.trim() });
}

export async function getProfile() {
  const data = await chrome.storage.local.get(STORAGE_KEYS.profile);
  return data[STORAGE_KEYS.profile] || null;
}

export async function setProfile(profile) {
  await chrome.storage.local.set({ [STORAGE_KEYS.profile]: profile });
}

export async function getHistory() {
  const data = await chrome.storage.local.get(STORAGE_KEYS.history);
  return data[STORAGE_KEYS.history] || [];
}

export async function addHistoryEntry(entry) {
  const history = await getHistory();
  history.unshift({
    ...entry,
    id: crypto.randomUUID(),
    savedAt: new Date().toISOString(),
  });
  // Keep last 100
  await chrome.storage.local.set({
    [STORAGE_KEYS.history]: history.slice(0, 100),
  });
  return history[0];
}
