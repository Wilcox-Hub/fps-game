# Project-memory workflow

## Start

Read the index/current work, inspect local changes and remote memory, then load only relevant current rules. Use `search` for a focused lookup. Follow the local Orca continuation record when recovering work. For readiness or test selection, read beta-readiness latest/README and verify evidence scope/hashes.

## Maintain

- Update affected records immediately after a material decision/correction, implementation result, test, blocker, scope change or release, and before dependent work or task completion.
- Keep one authoritative home for each fact. Current notes contain outcome, unresolved status, evidence links and next action. Replace stale summaries; keep original answers and dated history intact. Routine questions that add no durable fact need no duplicate entry.
- Preserve approval, implementation, testing and publication as separate states. Keep recorded dates and limits; report unknowns instead of inventing freshness. Link test records rather than copying scores/logs.
- Save every grill-me round/reply through [automatic interviews](AUTOMATIC_INTERVIEWS.md). Review related decisions; unresolved conflicts activate grill-me and block affected implementation until owner confirmation. Resolved supersessions stay resolved.
- Update `current.json` for approved changes, with reciprocal supersession links. Keep proposals/provisional settings/open choices labeled. Exclude credentials, private invites and unrelated chat.

## Finish

1. Refresh current work, affected issues/rules and one useful checkpoint. Keep the local Orca work record aligned. `checkpoint --record <json>` validates/appends task, phase, outcome, topics, decisions, evidence, publication and next_action.
2. Run `render` after decision/interview edits, then `check`. Verify affected records agree with current evidence and no original answers were lost. Run helper regressions when helper code changes.
3. Read fresh remote main; run `preview --base <sha>`, review the scoped diff, then `publish --base <sha>`. Memory-only sync is owner-authorized. The publisher preserves the checkout/index and excludes game/source/releases. Reconcile a changed remote before retrying.
4. Read GitHub back before reporting sync. If unavailable, retain local updates and explicitly mark sync pending. No background process observes unseen changes.

Use `python tools/project_memory.py <command>`. Detailed archives stay behind links; retrieve them only when the task needs them.
