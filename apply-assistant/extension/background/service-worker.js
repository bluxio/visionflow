/**
 * Background service worker — keepalive + message routing hooks
 */

chrome.runtime.onInstalled.addListener(() => {
  console.log("Apply Assistant installed");
});

chrome.runtime.onMessage.addListener((msg, sender, sendResponse) => {
  if (msg?.type === "PING") {
    sendResponse({ ok: true, tabId: sender.tab?.id });
    return true;
  }
});
