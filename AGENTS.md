# FPS Game Repository Guidance

## Identify the game before editing

- Start with `git status`, the current user request, the relevant project instructions, and the files that actually exist. Never reset, overwrite, or silently absorb existing work.
- This repository currently has two distinct game trees. The committed `main` README at the last inspection describes the dependency-free browser FPS **Neon Breach**. In the local worktree, the README and browser files are modified to describe **RIDGEFIRE**, while `unreal/Ridgefire/Ridgefire.uproject` and the rest of `unreal/` are untracked Unreal Engine 5.8.3 work. This state can change: re-check Git and the target project's entry points every task.
- Treat the root browser game and `unreal/Ridgefire` as separate implementations. Work only in the one requested. Do not copy, replace, publish, or delete one project's files to reconcile them unless the user explicitly asks.
- For Unreal work, read and follow [`unreal/AGENTS.md`](unreal/AGENTS.md). Do not infer that the Unreal project is on GitHub just because it exists locally.
- GitHub files are the durable project record. Memory is supplemental and may be stale; current user instructions and the checked-out project take precedence.

## Production workflow

Before substantial work, check the current player-facing outcome, relevant source of truth and repo-scoped Mem0 recall, smallest useful step, and observable completion evidence. Follow [`docs/AI_GAME_PRODUCTION.md`](docs/AI_GAME_PRODUCTION.md) for the proportional player-outcome, asset, engine, audio, and play-verification loop. For memory setup, scope, safety, and recovery, follow [`docs/MEMORY.md`](docs/MEMORY.md).

## Preserve and verify

- State relevant files and non-goals before editing; verify against the outcome defined above.
- Keep parallel work bounded to independent file sets. Review integrated changes and run the narrowest useful checks.
- Distinguish source/build/smoke evidence from rendered PIE, packaged, multiplayer-device, performance, and human-fun evidence. Never claim a test that did not run.
- Stage explicit paths only. Preserve unrelated changes, and commit or push only when the current user explicitly requests it.
