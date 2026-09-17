---
name: feedback-git-handling
description: "Skip git branch/commit ceremony for routine local Blueprint/content edits — but origin is a real shared GitHub repo other developers push to, not a throwaway"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 1a989c24-290e-4661-a002-6abe3e37d179
  modified: 2026-09-16T12:25:47.610Z
---

Don't apply the usual git safety ceremony (feature branches, checkpoint commits before risky changes, asking before committing) to *routine local edits* on this repo by default. The user said outright: "ignore github this is a copy project just make the changes."

**Why:** For day-to-day Blueprint/content iteration, the user doesn't want branch-and-commit ceremony getting in the way — that friction was the original complaint.

**Correction (2026-09-16):** This memory originally said the repo was "not the canonical/shared repo" — that's wrong, and was corrected during a git-push-planning session. `origin` is a real GitHub repo (`CJTechnology21/unreal-project-with-Parth`) with its own pushed history on `main`, and other developers (at least one teammate, Marcus Gonzalez, per a `Content/Developers/` folder) are about to start working in it too. See [[project_sotm_repo_state]] for what that session found already sitting in the pushed history.

**How to apply:** Make file/content/Blueprint changes directly without proposing a branch-and-commit workflow first — that part still stands. But never force-push, rewrite history, hard-reset, or push to `origin` without explicit confirmation — those affect a repo other people actually clone from. Deleting/overwriting a locally-tracked file directly is still fine (git has the last committed version for recovery), that's a different, genuinely-local risk.
