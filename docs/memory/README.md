# IRON SUN — Project Memory

Start every task with this index and [current work](active-work.json), then load only the relevant branch. Keep current summaries short; original answers and evidence remain linked.

| Need | Read |
| --- | --- |
| Approved rules | [Design decisions](../design-decisions/README.md) |
| Original answers and gaps | [Interview coverage](../design-decisions/coverage.json) |
| Full next-step inventory | [October20 work queue](NEXT_STEPS_2026-10-20.md) |
| Bugs and pending work | [Issues](open-issues.json) |
| Conflicting direction | [Conflict register](conflicts.json) |
| Tested status | [Latest evidence](../beta-readiness/latest.json) and [reuse rules](../beta-readiness/README.md) |
| Local test candidate | [October20 VS69](BETA_CANDIDATE_2026-10-20.md) — technical preparation complete; normal co-op/human acceptance open |
| Capture and sync | [Workflow](WORKFLOW.md); [automatic interviews](AUTOMATIC_INTERVIEWS.md) |
| Tools and phone help | [Environment](ENVIRONMENT.md); [Orca mobile](ORCA_MOBILE.md); [Mem0](../MEMORY.md) |
| History | [Task checkpoints](work-log.jsonl); [memory graph](graph.json) |

Search: `python tools/project_memory.py search "topic"`; add `--history` only when original answers are needed.

Refresh affected records after each material decision, correction, result, blocker or release. Replace stale current summaries, retain history, validate and sync before ending the task. Stored memory persists; reading retrieved content uses context tokens. Updates run during active project work.
