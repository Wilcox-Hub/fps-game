# VS27 readiness assessment — October 2, 2026

Gameplay planning estimate: **44%**, low confidence. The target of 50% has not yet been reached. Overall beta completion remains unmeasured and the verdict is NOT_BETA_READY.

The 35% historical baseline has gained permanent XP/two-slot unlocks, a real local save ledger, temporary team rewards, a shared weighted starting ballot and progression-preserving local co-op restart. This assessment retains the prior controls and weapons credit, gives limited credit to connected wave rewards and local co-op, and credits the newly verified progression foundation. Percentages are design judgments, not test-pass rates. Connectivity and asset-pipeline setup receive no gameplay credit.

## Equipment-style perk revision

Personal perks are percentage-based equipment and training choices, with clearer names and two situational bonuses: crouched armor and head-hit damage. The player's existing perk IDs and XP are retained. There are no new active power buttons, automatic echo attacks, supernatural saves or air dashes. See [progression guide](progression-guide.md) for all ten benefits, controls and limits.

## Evidence and reuse

Current Editor build, three save/unlock/store tests, possessed keyboard/gamepad interactions, standing/crouched real damage, body/head damage and removal of the head bonus passed. The interaction fixture rendered four frames at each of 1280x720 and 800x320. Generated combat, authoritative fire and two-process co-op/restart were repeated for the final gameplay revision. See [test results](test-results.json) and [excerpts](test-excerpts.txt).

Earlier VS26 recovery, loadout, helper and solo checks remain dated historical evidence in [historical-vs26-evidence.json](historical-vs26-evidence.json); their full inputs are not claimed to match VS27. New source hashes identify the latest tested working tree, including unpublished game changes. The Shipping prototype built successfully and survived an offscreen D3D12 startup check. That does not establish physical-input packaged gameplay, internet sessions, performance or human fun. A final change to fixture-only test expectations after the package build did not alter Shipping gameplay.

## Remaining beta gates

Normal-input packaged full runs; human balance/fun sessions; ten finished arenas, branch routes and champions; the full staged character/perk/secondary lobby; internet/Steam and four-player lifecycle/reclaim; production art/animation/audio; minimum-PC profiling and longer soaks. These gates prevent claiming overall beta readiness.

## Connectivity and asset workflow

Local Codex uses the GitHub connector for reads and an independently authorized CLI for documentation writes. Blender's loopback MCP supports live inspection. A versioned background Blender job exports a scratch asset and validates it with an Unreal import; the reviewed job processed 120 meshes/32,176 triangles, and a second invocation reused its cache. Token savings have not been measured. A separate ChatGPT web connection remains unverified.
