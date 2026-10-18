# How development, memory and validation work

Generated from [current.json](../current.json). Read current rules before historical interviews.

## workflow-memory — Every task uses project memory

**APPROVED** · Recorded 2026-10-18

Before any game, art, tooling, research, testing or release task, read the memory index/current work and only the relevant decision branches. Capture new user answers, corrections, outcomes, open questions and evidence at checkpoints. Publish scoped memory updates to GitHub under this standing request; preserve unrelated code and separate release authorization. Keep original answers and supersession history.

Sources: [owner-memory-scope](../../../docs/design-decisions/sources.md#owner-memory-scope).

## workflow-clarification — Resolve uncertain design through grill-me

**APPROVED** · Recorded 2026-10-18

When instructions/direction remain uncertain under the owner's requested 98% threshold, apply the grilling skill, asking the current frontier in batches with recommendations and waiting for answers/confirmation. Look up environmental facts first. Save exact questions/options/answers and clarify corrections; never claim numerical confidence is an observed test result.

Sources: [owner-grill-policy](../../../docs/design-decisions/sources.md#owner-grill-policy).

## workflow-evidence — Dated evidence with scoped reuse

**APPROVED** · Recorded 2026-10-18

Beta-readiness latest.json/README are canonical for tested status. Match input hashes, package/environment and validation level before reusing passes. Keep source, build, controlled simulation, rendered, packaged, network/device, performance and human-fun evidence distinct. Do not rerun unchanged matching tests by default or increase readiness based on documentation.

Sources: [current-evidence](../../../docs/design-decisions/sources.md#current-evidence).

## workflow-scenarios — Reusable controlled scenarios

**APPROVED** · Recorded 2026-10-18

Maintain prompt/scene/count-style scenario testing for specific functions, including two actual player-controlled roles reviving/carrying each other with varied angle, gun, arena and owner/client role. Store results and input identity in beta-readiness; automation supplements real-device and human-feel testing.

Sources: [owner-scenario-runner](../../../docs/design-decisions/sources.md#owner-scenario-runner), [current-evidence](../../../docs/design-decisions/sources.md#current-evidence).

## workflow-roll-audit — 100-roll audit stays paused

**DEFERRED** · Recorded 2026-10-18

The isolated 100-roll wildcard output/distribution review remains paused at the owner's release request. Keep its requested summary/change suggestion as pending work; do not mark it run or completed.

Sources: [owner-100-roll-pause](../../../docs/design-decisions/sources.md#owner-100-roll-pause).
