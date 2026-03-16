from html import escape

from fastapi import FastAPI, File, Form, UploadFile
from fastapi.responses import HTMLResponse, RedirectResponse

app = FastAPI(title="VisionFlow Demo Job App")


def _page(content: str) -> HTMLResponse:
    return HTMLResponse(
        f"""<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>VisionFlow Demo Application</title>
  <style>
    body {{
      font-family: Arial, sans-serif;
      background: linear-gradient(180deg, #f4f7ff 0%, #eef3fb 100%);
      margin: 0;
      color: #0f172a;
    }}
    .topbar {{
      background: #0f172a;
      color: #e2e8f0;
      padding: 12px 18px;
      font-size: 14px;
      letter-spacing: 0.2px;
    }}
    .container {{
      max-width: 860px;
      margin: 32px auto;
      background: #ffffff;
      border: 1px solid #dbe3f0;
      border-radius: 12px;
      padding: 28px;
      box-shadow: 0 8px 24px rgba(15, 23, 42, 0.06);
    }}
    h1 {{ margin-top: 0; font-size: 30px; }}
    h2 {{ margin-top: 30px; margin-bottom: 12px; font-size: 22px; }}
    label {{
      display: block;
      margin-top: 14px;
      margin-bottom: 6px;
      font-weight: 700;
      font-size: 20px;
    }}
    input, select, textarea {{
      width: 100%;
      box-sizing: border-box;
      border: 1px solid #b9c6dd;
      border-radius: 8px;
      padding: 14px 16px;
      font-size: 18px;
      background: #fff;
    }}
    textarea {{ min-height: 130px; resize: vertical; }}
    .note {{ color: #3b4a67; margin-top: 6px; }}
    .section {{
      border-top: 2px solid #e8eef8;
      padding-top: 20px;
      margin-top: 18px;
    }}
    .row {{ margin-top: 10px; }}
    .label {{ font-weight: 700; }}
    .actions {{ margin-top: 26px; display: flex; gap: 12px; }}
    .meta {{
      display: inline-block;
      font-size: 13px;
      font-weight: 700;
      background: #e6f0ff;
      color: #0f4ab8;
      border-radius: 999px;
      padding: 6px 10px;
      margin-bottom: 10px;
    }}
    .button {{
      border: none;
      border-radius: 10px;
      padding: 18px 22px;
      font-weight: 700;
      cursor: pointer;
      font-size: 24px;
    }}
    .button-primary {{
      background: #0e9f6e;
      color: #fff;
      width: 100%;
    }}
    .success {{
      border: 1px solid #b5ebd2;
      background: #ecfdf3;
      color: #065f46;
      border-radius: 10px;
      padding: 12px 14px;
      margin: 12px 0 0 0;
      font-weight: 700;
    }}
  </style>
</head>
<body>
  <div class="topbar">VisionFlow Demo | Stable Application Sandbox</div>
  <main class="container">
    {content}
  </main>
</body>
</html>"""
    )


@app.get("/", include_in_schema=False)
def root() -> RedirectResponse:
    return RedirectResponse(url="/apply", status_code=302)


@app.get("/apply", response_class=HTMLResponse)
def apply_page() -> HTMLResponse:
    return _page(
        """
<h1>Internship Application Demo</h1>
<div class="meta">Step 1 of 1: Application Form</div>
<p class="note">Complete all required fields, then submit your application.</p>

<form action="/submit" method="post" enctype="multipart/form-data">
  <section class="section">
    <h2>Applicant Information</h2>
    <label for="full_name">Full Name *</label>
    <input id="full_name" name="full_name" type="text" required />

    <label for="email">Email *</label>
    <input id="email" name="email" type="email" required />

    <label for="phone">Phone *</label>
    <input id="phone" name="phone" type="tel" required />
  </section>

  <section class="section">
    <h2>Role Selection</h2>
    <label for="role">Role *</label>
    <select id="role" name="role" required>
      <option value="Backend Intern">Backend Intern</option>
      <option value="SWE Intern">SWE Intern</option>
      <option value="ML Intern">ML Intern</option>
    </select>
  </section>

  <section class="section">
    <h2>Resume</h2>
    <label for="resume">Resume upload *</label>
    <input id="resume" name="resume" type="file" required />
  </section>

  <section class="section">
    <h2>Short Answers</h2>
    <label for="answer_why">Why do you want this role? *</label>
    <textarea id="answer_why" name="answer_why" required></textarea>

    <label for="answer_project">Describe a project you built. *</label>
    <textarea id="answer_project" name="answer_project" required></textarea>
  </section>

  <div class="actions">
    <button class="button button-primary" type="submit">Submit Application</button>
  </div>
</form>
"""
    )


@app.post("/submit", response_class=HTMLResponse)
async def submit(
    full_name: str = Form(""),
    email: str = Form(""),
    phone: str = Form(""),
    role: str = Form(""),
    answer_why: str = Form(""),
    answer_project: str = Form(""),
    resume: UploadFile | None = File(None),
) -> HTMLResponse:
    if (
        not full_name.strip()
        or not email.strip()
        or not phone.strip()
        or not role.strip()
        or not answer_why.strip()
        or not answer_project.strip()
        or resume is None
    ):
        return _page(
            "<h1>Missing required fields</h1>"
            '<p class="note">Please go back and complete all required inputs.</p>'
        )

    _ = await resume.read()
    resume_filename = resume.filename or "uploaded_resume"
    return _page(
        f"""
<h1>Application Submitted</h1>
<p class="success">Success! Your application has been submitted.</p>
<section class="section">
  <div class="row"><span class="label">Full Name:</span> {escape(full_name.strip())}</div>
  <div class="row"><span class="label">Email:</span> {escape(email.strip())}</div>
  <div class="row"><span class="label">Phone:</span> {escape(phone.strip())}</div>
  <div class="row"><span class="label">Role:</span> {escape(role.strip())}</div>
  <div class="row"><span class="label">Resume file:</span> {escape(resume_filename)}</div>
  <div class="row"><span class="label">Why do you want this role?</span><br />{escape(answer_why.strip())}</div>
  <div class="row"><span class="label">Describe a project you built.</span><br />{escape(answer_project.strip())}</div>
</section>
"""
    )
