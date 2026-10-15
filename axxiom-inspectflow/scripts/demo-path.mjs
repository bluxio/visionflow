import { chromium } from "playwright";

const BASE = process.env.BASE_URL || "http://localhost:3000";
const JOB = `${BASE}/jobs/seed-riverside-b03`;

async function runOnce(page, pass) {
  console.log(`\n=== Pass ${pass} ===`);

  await page.goto(`${JOB}?reset=1`, { waitUntil: "networkidle" });
  await page.waitForTimeout(800);

  // Assigned job
  await page.waitForSelector("text=Record closeout");
  const title = await page.locator("h1").first().textContent();
  console.log("Assigned:", title?.trim());
  if (!title?.includes("Riverside")) throw new Error("Assigned job title missing");

  await page.getByRole("link", { name: /Record closeout/i }).click();
  await page.waitForURL("**/closeout");

  // Capture
  await page.getByRole("button", { name: /Load demo narration/i }).click();
  const notes = await page.locator("#notes").inputValue();
  if (!notes.includes("Phase II")) throw new Error("Narration not loaded");
  await page.getByRole("button", { name: /Check closeout/i }).click();
  await page.waitForURL("**/gaps");

  // Gaps
  await page.waitForSelector("text=Finish these before you leave");
  await page.waitForSelector("text=Can resolve now");
  await page.waitForSelector("text=Requires action");

  // Oil: Old residue
  await page.getByRole("button", { name: "Old residue" }).click();
  // Button: Take photo
  await page.getByRole("button", { name: "Take photo" }).click();
  // Phase II: Not tested
  await page.getByRole("button", { name: "Not tested" }).click();
  await page.waitForSelector("text=does not invent a completed inspection step");
  await page.waitForSelector("text=Phase II still requires");

  await page.getByRole("button", { name: /Continue to closeout summary/i }).click();
  await page.waitForURL("**/ready");

  // Ready
  await page.waitForSelector("text=Closeout captured");
  await page.waitForSelector("text=All on-site questions were answered");
  await page.waitForSelector("text=follow-up action");
  const body = await page.locator("body").innerText();
  if (/Phase II fire service tested — passed/i.test(body)) {
    throw new Error("Phase II incorrectly marked passed");
  }
  if (!/Phase II fire service not tested/i.test(body)) {
    throw new Error("Phase II not-tested finding missing");
  }
  if (/Field closeout complete|FIELDBOSS|FieldBoss|Gemini|confidence score|Source: mock/i.test(body)) {
    throw new Error("Forbidden UI copy found on ready screen");
  }

  await page.getByRole("button", { name: /Submit closeout/i }).click();
  await page.waitForURL("**/inspections/**");

  // Manager review
  await page.waitForSelector("text=Ready for manager review");
  await page.waitForSelector("text=Information capture: Complete");
  await page.waitForSelector("text=follow-up action");
  const review = await page.locator("body").innerText();
  if (/Field completeness: Incomplete/i.test(review)) {
    throw new Error("Old incomplete completeness copy still present");
  }
  if (/1 item needs manager judgment(?!s)/i.test(review) && /2 item needs/i.test(review)) {
    throw new Error("Bad pluralization");
  }
  if (!/Not tested/i.test(review) && !/not tested/i.test(review)) {
    throw new Error("Clarification Not tested missing on review");
  }
  if (/FIELDBOSS|integrated with/i.test(review)) {
    throw new Error("Integration claim on review");
  }

  await page.getByRole("button", { name: /Confirm handoff/i }).click();
  await page.waitForSelector("text=Handoff confirmed");
  await page.getByRole("button", { name: /Prepare follow-up/i }).click();
  await page.waitForSelector("text=Prepared for review");
  await page.waitForSelector("text=Work-order draft");
  await page.waitForSelector("text=Order status: Blocked pending verification");
  await page.waitForSelector("text=on-site inspection visit");
  const drafts = await page.locator("body").innerText();
  if (/We completed a periodic inspection/i.test(drafts)) {
    throw new Error("Customer update still claims inspection completed");
  }
  if (/Parts candidates:\s*\n\s*- None/i.test(drafts)) {
    throw new Error("Empty parts candidates");
  }
  // Restart demo from assigned job
  await page.goto(JOB);
  await page.getByRole("button", { name: /Restart demo/i }).click();
  await page.waitForTimeout(500);
  await page.waitForSelector("text=Record closeout");
  const status = await page.locator("text=On site").count();
  if (status < 1) throw new Error("Restart demo did not return On site");

  console.log(`Pass ${pass} OK`);
}

const browser = await chromium.launch({ headless: true });
const context = await browser.newContext();
const page = await context.newPage();
page.on("pageerror", (err) => {
  console.error("PAGEERROR", err.message);
  throw err;
});

try {
  await runOnce(page, 1);
  await runOnce(page, 2);
  console.log("\nALL PASSES OK");
} finally {
  await browser.close();
}
