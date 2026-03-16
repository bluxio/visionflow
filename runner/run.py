import json
import os
from datetime import datetime, timezone
from pathlib import Path
from time import sleep

import requests
from playwright.sync_api import TimeoutError as PlaywrightTimeoutError
from playwright.sync_api import sync_playwright

SUBMIT_WORDS = ("submit", "apply", "send", "finish")
ROLE_OPTIONS = ("Backend Intern", "SWE Intern", "ML Intern")


def _looks_like_submit_target(target: str) -> bool:
    lowered = target.lower()
    return any(word in lowered for word in SUBMIT_WORDS)


def _approve_submit(prompt: str) -> bool:
    try:
        answer = input(prompt).strip().lower()
    except EOFError:
        return False
    return answer == "y"


def _click_first_visible_text(page, target: str) -> bool:
    candidates = page.get_by_text(target)
    count = candidates.count()
    for idx in range(count):
        item = candidates.nth(idx)
        try:
            if item.is_visible() and item.is_enabled():
                item.click(timeout=8000)
                return True
        except PlaywrightTimeoutError:
            continue
    return False


def _click_first_visible_text_force(page, target: str) -> bool:
    candidates = page.get_by_text(target, exact=False)
    count = candidates.count()
    for idx in range(count):
        item = candidates.nth(idx)
        try:
            item.scroll_into_view_if_needed(timeout=8000)
            item.click(timeout=8000, force=True)
            return True
        except Exception:
            continue
    return False


def _click_first_visible_role(page, role_name: str, target: str) -> bool:
    candidates = page.get_by_role(role_name, name=target, exact=False)
    count = candidates.count()
    for idx in range(count):
        item = candidates.nth(idx)
        try:
            if item.is_visible() and item.is_enabled():
                item.click(timeout=8000)
                return True
        except PlaywrightTimeoutError:
            continue
    return False


def _select_role_option(page, target: str) -> bool:
    select = page.locator("select").first
    try:
        if not select.is_visible():
            return False
    except Exception:
        return False

    try:
        select.select_option(label=target, timeout=3000)
        return True
    except Exception:
        pass

    try:
        options = select.locator("option").evaluate_all(
            "opts => opts.map(o => ({ label: (o.label || o.textContent || '').trim(), value: (o.value || '').trim() }))"
        )
        target_lower = target.strip().lower()
        for opt in options:
            label = str(opt.get("label", "")).strip().lower()
            value = str(opt.get("value", "")).strip()
            if label == target_lower and value:
                select.select_option(value=value, timeout=3000)
                return True
    except Exception:
        pass

    return False


def _click_text_with_fallbacks(page, target: str) -> bool:
    if _click_first_visible_role(page, "link", target):
        return True
    if _click_first_visible_role(page, "button", target):
        return True
    if _click_first_visible_role(page, "option", target):
        return True
    if _click_first_visible_text(page, target):
        return True
    return _click_first_visible_text_force(page, target)


def _fill_first_visible_input(page, value: str) -> bool:
    candidates = page.locator(
        'input[type="search"], input[name="search"], input:not([type="hidden"]), textarea'
    )
    count = candidates.count()
    for idx in range(count):
        field = candidates.nth(idx)
        try:
            if field.is_visible() and field.is_enabled():
                field.fill(value, timeout=2000)
                return True
        except PlaywrightTimeoutError:
            continue
    return False


def update_overlay(page, step_id: int | None, action: dict) -> None:
    action_type = str(action.get("action") or "—")
    target = str(action.get("target") or "—")
    try:
        confidence_val = float(action.get("confidence"))
        confidence = f"{confidence_val:.2f}"
    except Exception:
        confidence = "—"

    try:
        page.evaluate(
            """
            ({ step, actionType, target, confidence }) => {
              const id = "visionflow-agent-overlay";
              let el = document.getElementById(id);
              if (!el) {
                el = document.createElement("div");
                el.id = id;
                el.style.position = "fixed";
                el.style.top = "12px";
                el.style.right = "12px";
                el.style.background = "rgba(15, 23, 42, 0.85)";
                el.style.color = "#fff";
                el.style.padding = "10px 12px";
                el.style.borderRadius = "10px";
                el.style.fontFamily = "ui-sans-serif, system-ui, -apple-system, Segoe UI, Roboto, Arial";
                el.style.fontSize = "12px";
                el.style.lineHeight = "1.35";
                el.style.zIndex = "2147483647";
                el.style.pointerEvents = "none";
                el.style.minWidth = "220px";
                el.style.boxShadow = "0 6px 18px rgba(0,0,0,0.25)";
                document.documentElement.appendChild(el);
              }
              const safe = (v) => (v === null || v === undefined || v === "" ? "—" : String(v));
              el.innerHTML = `
                <div style="font-weight:800; font-size:13px; margin-bottom:6px;">VisionFlow Agent</div>
                <div><span style="opacity:.75;">Step:</span> ${safe(step)}</div>
                <div><span style="opacity:.75;">Action:</span> ${safe(actionType)}</div>
                <div><span style="opacity:.75;">Target:</span> ${safe(target)}</div>
                <div><span style="opacity:.75;">Confidence:</span> ${safe(confidence)}</div>
              `;
            }
            """,
            {
                "step": step_id if isinstance(step_id, int) else "—",
                "actionType": action_type,
                "target": target,
                "confidence": confidence,
            },
        )
    except Exception:
        return


def _log_step(
    log_path: Path,
    step_id: int | None,
    action: dict,
    url: str,
    outcome: str,
    error: str | None = None,
) -> None:
    entry = {
        "time": datetime.now(timezone.utc).isoformat(),
        "step_id": step_id,
        "action": action,
        "url": url,
        "outcome": outcome,
        "error": error,
    }
    log_path.parent.mkdir(parents=True, exist_ok=True)
    with log_path.open("a", encoding="utf-8") as f:
        f.write(json.dumps(entry) + "\n")


def main() -> None:
    backend_url = os.getenv("BACKEND_URL", "http://localhost:8000/next_action")
    demo_app_url = os.getenv("DEMO_APP_URL", "http://localhost:8080")
    goal_mode = os.getenv("GOAL_MODE", "demo_form").strip().lower()
    max_steps = int(os.getenv("MAX_STEPS", "30"))
    slow_ms = int(os.getenv("SLOW_MS", "300"))
    settle_ms = int(os.getenv("SETTLE_MS", "250"))
    fallback_scroll_limit = int(os.getenv("FALLBACK_SCROLL_LIMIT", "5"))
    screenshot_path = Path("runner_screenshot.png")
    log_path = Path("runner/session_log.jsonl")
    if goal_mode == "greenhouse":
        goal = (
            "Navigate this public Greenhouse page and complete as much of the job "
            "application flow as possible using PROFILE_JSON. Prioritize reliable "
            "form fields and avoid random navigation. When you reach submit, return "
            "confirm_submit instead of clicking submit."
        )
    else:
        goal = (
            "Fill out the job application form using the PROFILE_JSON. "
            "When you reach the final step, do NOT click Submit Application. "
            "Instead return confirm_submit."
        )
    profile = json.loads(Path("runner/profile.json").read_text())
    goal = (
        goal
        + f"\nGOAL_MODE: {goal_mode}\n"
        + "PROFILE_JSON:\n"
        + json.dumps(profile)
    )

    last_actions = []
    print(f"DEMO_APP_URL={demo_app_url}")
    print(f"BACKEND_URL={backend_url}")
    print(f"GOAL_MODE={goal_mode}")
    print(f"MAX_STEPS={max_steps}")
    print(f"SLOW_MS={slow_ms}")
    print(f"SETTLE_MS={settle_ms}")
    print(f"FALLBACK_SCROLL_LIMIT={fallback_scroll_limit}")

    with sync_playwright() as p:
        browser = p.chromium.launch(headless=False)
        page = browser.new_page(viewport={"width": 1280, "height": 720})
        try:
            page.goto(f"{demo_app_url.rstrip('/')}/apply", wait_until="domcontentloaded")
        except Exception as e:
            print(f"startup warning: {type(e).__name__}: {e}")
            browser.close()
            return

        fallback_scroll_streak = 0
        for _ in range(max_steps):
            try:
                page.wait_for_load_state("domcontentloaded", timeout=10000)
                page.screenshot(path=str(screenshot_path), full_page=False)
            except Exception as e:
                print(f"screenshot warning: {type(e).__name__}: {e}")
                _log_step(
                    log_path=log_path,
                    step_id=None,
                    action={"action": "screenshot"},
                    url=page.url,
                    outcome="fail",
                    error=f"{type(e).__name__}: {e}",
                )
                sleep(max(slow_ms, 0) / 1000.0)
                continue

            try:
                with screenshot_path.open("rb") as screenshot_file:
                    response = requests.post(
                        backend_url,
                        data={
                            "goal": goal,
                            "last_actions": json.dumps(last_actions),
                        },
                        files={"screenshot": ("screenshot.png", screenshot_file, "image/png")},
                        timeout=30,
                    )
                response.raise_for_status()
                step = response.json()
                print(json.dumps(step, indent=2))
            except Exception as e:
                print(f"planner warning: {type(e).__name__}: {e}")
                _log_step(
                    log_path=log_path,
                    step_id=None,
                    action={"action": "planner_request"},
                    url=page.url,
                    outcome="fail",
                    error=f"{type(e).__name__}: {e}",
                )
                sleep(max(slow_ms, 0) / 1000.0)
                continue

            action = step.get("action", {})
            step_id = step.get("step_id")
            update_overlay(page, step_id if isinstance(step_id, int) else None, action)
            action_type = action.get("action")
            action_to_record = action
            should_stop = False
            action_succeeded = False
            action_error = None
            url_before = page.url

            try:
                if action_type == "click_text":
                    target = action.get("target", "")
                    x = action.get("x")
                    y = action.get("y")
                    if target and _looks_like_submit_target(target):
                        print("Intercepted submit-like click. Requiring confirm_submit.")
                        if not _approve_submit("Approve submit-like action? (y/N): "):
                            print("Canceled.")
                            action_error = "confirm_submit canceled by user"
                            should_stop = True
                        else:
                            action_succeeded = True
                            action_to_record = {
                                "action": "confirm_submit",
                                "target": target,
                                "confidence": action.get("confidence"),
                                "rationale": "Client safety intercept before submit-like click.",
                            }
                    else:
                        handled = False
                        if target:
                            if target in ROLE_OPTIONS:
                                handled = _select_role_option(page, target)
                                if not handled:
                                    print(
                                        f'click_text warning: select_option failed for "{target}", '
                                        "falling back to click logic"
                                    )
                            if not handled:
                                handled = _click_text_with_fallbacks(page, target)
                        if not handled and x is not None and y is not None:
                            page.mouse.click(float(x), float(y))
                            handled = True
                        if not handled:
                            print(
                                f'click_text warning: no visible clickable match for "{target}"'
                            )
                            action_error = (
                                f'no visible clickable match for "{target}"'
                            )
                        action_succeeded = handled
                elif action_type == "click_bbox":
                    x = action.get("x")
                    y = action.get("y")
                    if x is not None and y is not None:
                        page.mouse.click(float(x), float(y))
                        action_succeeded = True
                    else:
                        print("click_bbox warning: missing x/y")
                        action_error = "missing x/y"
                elif action_type == "type_text":
                    target = action.get("target")
                    value = action.get("value", "")
                    x = action.get("x")
                    y = action.get("y")
                    handled = False

                    if not value:
                        handled = True

                    if target and not handled:
                        try:
                            page.get_by_label(target, exact=False).first.fill(
                                value, timeout=5000
                            )
                            handled = True
                        except PlaywrightTimeoutError:
                            if x is not None and y is not None:
                                page.mouse.click(float(x), float(y))
                                page.keyboard.type(value, delay=15)
                                handled = True

                    if not target and not handled and x is not None and y is not None:
                        page.mouse.click(float(x), float(y))
                        page.keyboard.type(value, delay=15)
                        handled = True

                    if not handled:
                        handled = _fill_first_visible_input(page, value)

                    if not handled:
                        print("type_text warning: no visible editable input found")
                        action_error = "no visible editable input found"
                    action_succeeded = handled
                elif action_type == "press":
                    key = action.get("key", "")
                    if key:
                        page.keyboard.press(key)
                        action_succeeded = True
                    else:
                        print("press warning: missing key")
                        action_error = "missing key"
                elif action_type == "scroll":
                    direction = action.get("direction", "down")
                    amount = int(action.get("amount", 600))
                    delta_y = amount if direction == "down" else -amount
                    page.mouse.wheel(0, delta_y)
                    action_succeeded = True
                elif action_type == "confirm_submit":
                    if not _approve_submit("Approve Submit Application? (y/N): "):
                        print("Canceled.")
                        action_error = "confirm_submit canceled by user"
                        should_stop = True
                    else:
                        try:
                            page.get_by_text("Submit Application", exact=False).first.click(
                                timeout=7000
                            )
                            action_succeeded = True
                        except Exception as e:
                            print(f"confirm_submit warning: {type(e).__name__}: {e}")
                            action_error = f"{type(e).__name__}: {e}"
                elif action_type == "done":
                    should_stop = True
                    action_succeeded = True
                else:
                    print(f"warning: unknown action type: {action_type}")
                    action_error = f"unknown action type: {action_type}"
            except Exception as e:
                print(f"{action_type} warning: {type(e).__name__}: {e}")
                action_error = f"{type(e).__name__}: {e}"

            if action_succeeded:
                if page.url != url_before:
                    try:
                        page.wait_for_load_state("networkidle", timeout=10000)
                    except Exception as e:
                        print(f"navigation settle warning: {type(e).__name__}: {e}")
                sleep(max(settle_ms, 0) / 1000.0)

            if (
                action.get("action") == "scroll"
                and "fallback scroll" in (action.get("rationale") or "").lower()
            ):
                fallback_scroll_streak += 1
            else:
                fallback_scroll_streak = 0
            if fallback_scroll_streak >= fallback_scroll_limit:
                print(
                    "Stopping early due to repeated planner fallback scroll actions "
                    f"({fallback_scroll_streak} in a row)."
                )
                should_stop = True

            _log_step(
                log_path=log_path,
                step_id=step_id if isinstance(step_id, int) else None,
                action=action,
                url=page.url,
                outcome="ok" if action_succeeded else "fail",
                error=action_error,
            )
            last_actions.append(action_to_record)
            sleep(max(slow_ms, 0) / 1000.0)
            if should_stop:
                break

        page.wait_for_timeout(1000)
        browser.close()


if __name__ == "__main__":
    main()