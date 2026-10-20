# How development, memory and validation work

Generated from [current.json](../current.json). Read current rules before historical interviews.

## workflow-memory — Every task uses project memory

**APPROVED** · Recorded 2026-10-18

Start all IRON SUN work from the memory index/current checkpoint and relevant current rules. Keep summaries concise, deduplicate facts and link detailed answers/evidence. Refresh affected records after each material decision, correction, outcome, blocker or release and before dependent work/task completion. Preserve exact interview answers and dated history. Automatically capture/sync every grill-me round/reply; unresolved conflicts require owner clarification. Validate and verify scoped GitHub sync; mark offline updates pending. Game/source/releases retain separate authorization.

Sources: [owner-memory-scope](../../../docs/design-decisions/sources.md#owner-memory-scope), [owner-auto-interviews](../../../docs/design-decisions/sources.md#owner-auto-interviews), [owner-concise-memory](../../../docs/design-decisions/sources.md#owner-concise-memory).

## workflow-clarification — Resolve uncertain design through grill-me

**APPROVED** · Recorded 2026-10-18

Automatically capture every IRON SUN grill-me prompt/options before asking and save/review/sync each reply immediately. Any unresolved conflict among answers, current rules or proposed changes activates grill-me, preserves both statements and blocks affected implementation until owner clarification and explicit confirmation. Resolved supersessions stay resolved. Investigate environmental facts first; the 98% instruction-confidence preference is not a measured validation score.

Sources: [owner-grill-policy](../../../docs/design-decisions/sources.md#owner-grill-policy), [owner-auto-interviews](../../../docs/design-decisions/sources.md#owner-auto-interviews).

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
