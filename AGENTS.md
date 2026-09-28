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
