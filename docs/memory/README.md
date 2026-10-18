# IRON SUN — Project Memory

Start every task here. This is the GitHub-backed memory tree for game direction and work continuity. Follow the branch relevant to the task; full chat history is unnecessary for routine work.

```text
IRON SUN
├── Current work and next action → active-work.json
├── Approved direction → ../design-decisions/README.md
│   ├── Identity, modes and naming
│   ├── Combat, movement and controls
│   ├── Waves, arenas and pressure
│   ├── Weapons, shards and arsenal
│   ├── Wildcards and physical upgrades
│   ├── Squad lobby and arrival
│   ├── Art, characters and readable UI
│   ├── Co-op, launcher and delivery
│   └── Development and validation rules
├── Original questions and answers → ../design-decisions/interviews/
├── Unresolved issues and acceptance → open-issues.json
├── Task checkpoints → work-log.jsonl
└── Test evidence and readiness → ../beta-readiness/latest.json
```

- [Workspace/tool context](ENVIRONMENT.md) and [memory setup checks](setup-validation.json).
- [Current work](active-work.json) and [open issues](open-issues.json).
- [Approved direction and original interviews](../design-decisions/README.md).
- [All-work maintenance procedure](WORKFLOW.md) and [machine-readable tree](graph.json).
- [Canonical test/readiness record](../beta-readiness/latest.json) and [evidence reuse rules](../beta-readiness/README.md).
- [Mem0 setup and scope](../MEMORY.md): optional retrieval; this file tree remains the shared record.

Use `python tools/project_memory.py search "ammo"` for a small relevant lookup; add `--history` for original answers. Current approved rules come first. Use `check` before publishing. [Recovery coverage](../design-decisions/coverage.json) explicitly lists historical gaps.

This is agent-maintained file memory with a real node/relationship index, not a new paid service or an autonomous daemon. Repository guidance requires reading and updating it. Changes become available to other PCs/accounts when they read or pull the GitHub version; it does not transfer an entire private chat or run a game deployment.
