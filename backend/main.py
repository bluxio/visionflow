import json
import os

from fastapi import FastAPI, File, Form, UploadFile
from google import genai
from google.genai import types

from shared.schema import Action, StepResponse

app = FastAPI(title="Multimodal Agent Backend")


def _parse_last_actions(raw: str) -> list[dict]:
    try:
        parsed = json.loads(raw) if raw else []
    except json.JSONDecodeError:
        return []
    return parsed if isinstance(parsed, list) else []


def _safe_scroll(step_id: int, rationale: str) -> StepResponse:
    return StepResponse(
        step_id=step_id,
        action=Action(
            action="scroll",
            direction="down",
            amount=500,
            confidence=0.3,
            rationale=rationale,
        ),
    )


def _is_truthy(value: str | None) -> bool:
    return (value or "").strip().lower() in {"1", "true", "yes", "on"}


def _extract_profile_from_goal(goal: str) -> dict:
    marker = "PROFILE_JSON:"
    if marker not in goal:
        return {}
    raw = goal.split(marker, 1)[1].strip()
    try:
        parsed = json.loads(raw)
        return parsed if isinstance(parsed, dict) else {}
    except json.JSONDecodeError:
        return {}


def _extract_goal_mode(goal: str) -> str:
    marker = "GOAL_MODE:"
    if marker not in goal:
        return "demo_form"
    tail = goal.split(marker, 1)[1]
    first_line = tail.splitlines()[0].strip().lower()
    return first_line or "demo_form"


def _action_seen(
    actions: list[dict],
    action_name: str,
    target: str | None = None,
    value: str | None = None,
) -> bool:
    for item in actions:
        if not isinstance(item, dict):
            continue
        if item.get("action") != action_name:
            continue
        if target is not None and item.get("target") != target:
            continue
        if value is not None and item.get("value") != value:
            continue
        return True
    return False


def _scripted_step(actions: list[dict], goal: str) -> StepResponse:
    profile = _extract_profile_from_goal(goal)
    full_name = str(profile.get("full_name", "Bolu Akande"))
    email = str(profile.get("email", "bolu@example.com"))
    phone = str(profile.get("phone", "469-555-0123"))
    role = str(profile.get("role", "SWE Intern"))
    why_role = str(
        profile.get(
            "why_role",
            "I want this role to build real production systems and learn from strong engineers.",
        )
    )
    project = str(
        profile.get(
            "project",
            "Built VisionFlow, a multimodal UI navigator with schema validation and safety gating.",
        )
    )

    plan = [
        (
            1,
            Action(
                action="type_text",
                target="Full Name *",
                value=full_name,
                confidence=1.0,
                rationale="Scripted demo mode: fill full name.",
            ),
        ),
        (
            2,
            Action(
                action="type_text",
                target="Email *",
                value=email,
                confidence=1.0,
                rationale="Scripted demo mode: fill email.",
            ),
        ),
        (
            3,
            Action(
                action="type_text",
                target="Phone *",
                value=phone,
                confidence=1.0,
                rationale="Scripted demo mode: fill phone.",
            ),
        ),
        (
            4,
            Action(
                action="click_text",
                target=role,
                confidence=1.0,
                rationale="Scripted demo mode: select role.",
            ),
        ),
        (
            5,
            Action(
                action="type_text",
                target="Why do you want this role? *",
                value=why_role,
                confidence=1.0,
                rationale="Scripted demo mode: fill motivation answer.",
            ),
        ),
        (
            6,
            Action(
                action="type_text",
                target="Describe a project you built. *",
                value=project,
                confidence=1.0,
                rationale="Scripted demo mode: fill project answer.",
            ),
        ),
        (
            7,
            Action(
                action="confirm_submit",
                confidence=1.0,
                rationale="Scripted demo mode: require human approval before submit.",
            ),
        ),
        (
            8,
            Action(
                action="done",
                confidence=1.0,
                rationale="Scripted demo mode complete.",
            ),
        ),
    ]

    for step_no, planned_action in plan:
        if not _action_seen(
            actions,
            planned_action.action,
            target=planned_action.target,
            value=planned_action.value,
        ):
            return StepResponse(step_id=step_no, action=planned_action)
    return StepResponse(step_id=8, action=plan[-1][1])


@app.get("/health")
def health() -> dict[str, str]:
    return {"status": "ok"}


@app.post("/next_action", response_model=StepResponse)
async def next_action(
    goal: str = Form(...),
    screenshot: UploadFile = File(...),
    last_actions: str = Form(""),
) -> StepResponse:
    actions = _parse_last_actions(last_actions)
    screenshot_bytes = await screenshot.read()
    step_id = len(actions) + 1
    if _is_truthy(os.getenv("DEMO_SCRIPTED_MODE")):
        return _scripted_step(actions, goal)
    api_key = os.getenv("GEMINI_API_KEY")
    print(f"GEMINI_API_KEY present: {bool(api_key)}")

    if not api_key:
        return _safe_scroll(step_id, "GEMINI_API_KEY is missing; using safe fallback.")

    goal_mode = _extract_goal_mode(goal)
    if goal_mode == "greenhouse":
        system_prompt = (
            "You are a deterministic UI automation planner for public Greenhouse job "
            "application pages.\n\n"
            "Your objective is to complete reliable, visible form steps using "
            "PROFILE_JSON embedded in the user's goal.\n\n"
            "HARD RULES:\n"
            "- Prioritize text inputs, dropdowns, and easy visible fields first.\n"
            "- Prefer type_text actions with targets matching visible field labels.\n"
            "- Avoid random navigation across unrelated pages.\n"
            "- Do NOT click final submit directly.\n"
            "- When ready for final submit, return confirm_submit.\n"
            "- Output exactly one action per step.\n"
            "- Output ONLY valid JSON matching the StepResponse schema.\n"
            "- Never auto-submit any form without confirm_submit.\n\n"
            "The available action types are:\n"
            "- click_text\n"
            "- click_bbox\n"
            "- type_text\n"
            "- press\n"
            "- scroll\n"
            "- confirm_submit\n"
            "- done\n\n"
            "You must return valid JSON only."
        )
    else:
        system_prompt = (
            "You are a deterministic UI automation planner for a local demo form at "
            "http://localhost:8080/apply.\n\n"
            "Your objective is to fill the job application form using PROFILE_JSON "
            "embedded in the user's goal.\n\n"
            "HARD RULES:\n"
            "- Use PROFILE_JSON values to fill Full Name, Email, Phone, Role, "
            "Why do you want this role?, and Describe a project you built.\n"
            "- Prefer type_text actions with targets that match visible field labels.\n"
            "- Do NOT click \"Submit Application\" directly.\n"
            "- When all required fields are complete and ready to submit, return "
            "confirm_submit.\n"
            "- Output exactly one action per step.\n"
            "- Output ONLY valid JSON matching the StepResponse schema.\n"
            "- Never auto-submit any form without confirm_submit.\n\n"
            "The available action types are:\n"
            "- click_text\n"
            "- click_bbox\n"
            "- type_text\n"
            "- press\n"
            "- scroll\n"
            "- confirm_submit\n"
            "- done\n\n"
            "You must return valid JSON only."
        )
    user_text = (
        f"Goal: {goal}\n"
        f"Last actions JSON: {json.dumps(actions)}\n"
        f"Next step id: {step_id}\n"
        "Plan the next single best action."
    )

    try:
        client = genai.Client(api_key=api_key)
        resp = client.models.generate_content(
            model=os.getenv("GEMINI_MODEL", "gemini-2.5-flash"),
            config={
                "temperature": 0,
                "response_mime_type": "application/json",
                "response_schema": StepResponse,
                "system_instruction": system_prompt,
            },
            contents=[
                types.Part.from_text(text=user_text),
                types.Part.from_bytes(
                    data=screenshot_bytes,
                    mime_type=screenshot.content_type or "image/png",
                ),
            ],
        )
    except Exception as e:
        print(f"Planner call error: {type(e).__name__}: {e}")
        return _safe_scroll(step_id, "Planner call failed; using safe fallback scroll.")

    raw = resp.text or ""
    print(raw[:500])

    try:
        parsed = json.loads(raw)
        parsed["step_id"] = step_id
        return StepResponse.model_validate(parsed)
    except Exception as e:
        print(f"Planner parse/validate error: {type(e).__name__}: {e}")
        return _safe_scroll(step_id, "Planner output invalid; using safe fallback scroll.")
