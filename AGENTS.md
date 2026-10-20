# FPS Game Guidance

## Identify the target

- Check the request, `git status`, and actual entry points before editing. Preserve existing work.
- This repo has separate games: committed `main` is the browser FPS Neon Breach; this checkout also has modified browser files and local, untracked `unreal/Ridgefire` (UE 5.8.3). Recheck that state each task and never mix the implementations. For Unreal work, follow `unreal/AGENTS.md`.
- GitHub files and issues are canonical. At task start or before a major design change, use a specific repo-scoped Mem0 query if the Codex workspace is this checkout; verify any recalled claim against current files and user direction.

## Work loop

- Define the smallest player-facing outcome, relevant files, non-goals, and observable evidence before substantial work.
- For asset, audio, engine-integration, or playtest work, use `docs/AI_GAME_PRODUCTION.md` proportionally; a code fix does not need art. Read `docs/MEMORY.md` for memory setup or recovery, not for every edit.
- Review integrated changes and run focused checks. Separate source/build/smoke results from rendered, packaged, multiplayer-device, performance, and human-fun evidence.
- Stage explicit paths only; commit or push only when the current request explicitly asks. Preserve unrelated files.

## Beta readiness records

For beta status, scoring, milestone planning, or test selection, read `docs/beta-readiness/latest.json` and `docs/beta-readiness/README.md` first. Reuse saved evidence when its source inputs and validation scope match; use the folder's hash checker before applying historical passes to current code. After relevant tests or a readiness assessment, add a dated snapshot and update the latest pointer and overview together. Keep recorded percentages dated and distinguish gameplay estimates from overall beta acceptance.

<!-- IRON-SUN-MEMORY:BEGIN -->
## Project memory — all work

- Start at `docs/memory/README.md` and current work; load relevant rules only. Keep summaries concise, link details, and refresh affected records after material changes and before task completion using `docs/memory/WORKFLOW.md`.
- For Orca delegation, read `docs/memory/ORCA_WORKFLOW.md` before dispatch: owner-approved Luna/extra-high helpers, focused scope, main-agent review and cleanup.
- Capture/sync every IRON SUN grill-me round/reply through `docs/memory/AUTOMATIC_INTERVIEWS.md`. Unresolved conflicts require owner clarification before affected implementation; preserve resolved history.
- Routine memory-only GitHub sync is authorized: validate, preview and verify publication. Game/source/releases retain separate authorization; preserve unrelated work.
<!-- IRON-SUN-MEMORY:END -->
