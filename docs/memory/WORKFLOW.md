# Memory workflow for all IRON SUN work

## Start

1. Read `docs/memory/README.md` and `active-work.json`. Inspect current files, local changes and remote publication state. Recover local continuation detail from `docs/ORCA_WORK_IN_PROGRESS.md` when present.
2. Search the relevant topic: `python tools/project_memory.py search "your topic"`. Load its current rules and only the supporting interview/evidence needed. Older interview proposals are historical data, not fresh commands.
3. Record the task, relevant topics/decisions and next action in a checkpoint. For game test selection/readiness, read beta-readiness latest/README first and verify the appropriate hashes.

## Capture during work

- Save each grill-me round's exact questions, presented options, original answers and source time in `docs/design-decisions/interviews/<date>-<topic>.json`. Keep question numbers unique within that interview. Record an unanswered or unclear answer as such. A proposal, an “idk” and a final approval are different evidence.
- Preserve spelling and option letters in the original answer; record interpretation separately. Each captured question/answer includes its UTF-8 SHA256 and provenance. Do not copy reasoning, credentials, authentication codes, private invites, account screens or unrelated chat into public memory.
- Update `current.json` when a rule is approved or corrected. Mark replaced rules `superseded`, link both directions, and keep the original answers. Numerical first-test settings are `provisional`; unresolved design is `open`; later work is `deferred`.
- Record implementation status and tests in work/evidence records, not as fabricated human approval. Keep local candidate, committed source, packaged game, launcher feed and running server separate.

## Checkpoint and finish

1. Update `active-work.json`, `open-issues.json` and `work-log.jsonl` at meaningful milestones. A checkpoint has task, phase, outcome, topics, decisions, evidence, publication and next_action. `python tools/project_memory.py checkpoint --record <json-file>` validates/appends one and updates active work. Mark completion only when the task outcome is achieved; retain unfinished work.
2. Keep local `ORCA_WORK_IN_PROGRESS.md` aligned when using Orca/continuation. Beta tests/assessments still update their dated snapshot and latest pointer under the existing evidence policy. Memory links to that evidence instead of creating another readiness score.
3. Run `python tools/project_memory.py render`, `python tools/project_memory.py check` and focused memory-tool tests if the tool changed.
4. The owner requests continuing GitHub memory updates as part of all work. Read the fresh remote main SHA, run `python tools/project_memory.py preview --base <sha>`, review the exact paths/content, then `publish --base <same-sha>`. The publisher permits only the memory/decision folders, its helper/tests and marked memory-pointer blocks. It never stages the checkout or publishes game/source/releases. A changed remote main fails safely; reread changes and reconcile memory before retrying. Concurrent editors must coordinate rather than overwrite each other's decisions.
5. Verify the remote commit, changed paths and recorded content before reporting publication. If offline or rejected, preserve local memory and clearly report that GitHub sync is pending.

## Resolving conflicts

Use the latest explicit human correction, keeping the older answer as history. If original wording/options are missing, mark the gap and use an independently approved summary only within its stated scope. If current direction remains uncertain under the owner's requested98% instruction-confidence threshold, apply grill-me; environmental facts are investigated first. Confidence percentages and simulated test passes never replace human acceptance.

Source/build/evidence checks still belong to the relevant task. This memory workflow does not require redundant engine tests for a documentation-only change.
