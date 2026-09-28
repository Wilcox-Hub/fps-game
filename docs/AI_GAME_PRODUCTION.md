# AI Game Production Loop

Use this workflow when it helps the task; keep it proportional to the player-facing outcome.

## Before substantial work

1. Check the current user goal, repository and nested instructions, Git state, target project, existing design decisions, and work already in progress.
2. Define the smallest useful player-facing result, its relevant files, non-goals, and the observable evidence that will show whether it works.
3. Prefer a playable vertical slice over a pile of disconnected content. A focused code fix does not need concept art, a Blender model, or new audio.

## Build the slice

1. **Outcome:** Describe what the player will be able to see, feel, or do, and how to observe it.
2. **Visual assets, when needed:** Establish the concept and constraints first. Create or revise the Blender model; rig or animate only when gameplay needs it. Keep scale, silhouette, readability, collision, and performance constraints explicit.
3. **Engine integration:** Import the asset into the actual requested game engine and pipeline. Wire materials, collision, animation, gameplay behavior, and replication as needed. Do not stop at an attractive asset that the game never uses.
4. **Audio, when useful:** Add original or appropriately licensed sound where it improves feedback, readability, or atmosphere. Keep audio changes testable and avoid using copied copyrighted recordings.
5. **Play and inspect:** Run the actual game in the strongest available mode. Observe the player experience, exercise likely edge cases, fix what fails, and record the exact command, build, platform, resolution, and outcome. If only a headless test or source check ran, say so; do not call it a visual playtest.

## Close the loop

- Record concise evidence and remaining risks in Git-tracked project docs when it will help future work.
- Keep decisions canonical in repository files and GitHub issues. Recheck those sources before acting on remembered context.
- For Unreal work, follow `unreal/AGENTS.md`; for the root browser prototype, follow its actual HTML/CSS/JavaScript pipeline. Never assume an engine or asset workflow from another project applies.
- The current Unreal target is `unreal/Ridgefire` on UE 5.8.3. Use the installed Blender 5.2.2 LTS for Blender MCP; the older installed Blender 3.0.0 cannot parse the current add-on. When a new mesh is needed, retain its editable source, validate export/import scale and orientation, set up materials/collision or animation in Unreal, and inspect the asset in actual gameplay. Do not require Blender for code-only fixes or assume a successful import proves a playable result.
- A slice is complete only when the stated player-facing outcome works at the level claimed. Asset creation, compilation, smoke tests, rendered play, packaged play, and human feedback are different evidence levels.
