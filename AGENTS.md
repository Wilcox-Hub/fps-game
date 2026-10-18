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

- Start every game, art, research, tooling, testing or release task at `docs/memory/README.md` and `active-work.json`; load the relevant current decision branch before implementation. Historical interviews are evidence; latest human corrections govern.
- Automatically save each IRON SUN grill-me round before asking and capture/review/sync every reply immediately through `docs/memory/AUTOMATIC_INTERVIEWS.md`, without a separate save request. Any unresolved conflict activates grill-me; record both statements and block affected implementation until owner clarification/confirmation. Resolved supersessions stay resolved. Update work/issue/evidence pointers at checkpoints.
- The owner authorizes ongoing memory-only GitHub updates. Run the memory check and scoped publication preview before publishing through `tools/project_memory.py`; this authorization excludes unrelated game/source, builds, launcher feeds and server deployment.
<!-- IRON-SUN-MEMORY:END -->
