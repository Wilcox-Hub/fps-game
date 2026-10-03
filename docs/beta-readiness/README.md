# IRON SUN beta readiness

Latest gameplay planning estimate: **62%** (weighted 61.5), low confidence, assessed October 3, 2026. **NOT_BETA_READY.** The target is **75%; it remains in progress**. Overall beta completion remains unmeasured. These percentages are planning judgments, not test pass rates.

Start with [latest.json](latest.json), the [new video review](snapshots/2026-10-03-vs34/report.md), [scores](snapshots/2026-10-03-vs34/scores.json), [current evidence/reuse](snapshots/2026-10-03-vs34/test-results.json), [VS33 implementation results](snapshots/2026-10-03-vs33/test-results.json), [run guide](snapshots/2026-10-03-vs33/run-guide.md), and [remaining milestones](snapshots/2026-10-03-vs33/target-75.md). Older snapshots remain history.

| Area | Weight | Readiness | Points |
| --- | ---: | ---: | ---: |
| Controls and combat | 30% | 60% | 18.0 |
| Weapons and arsenal | 20% | 50% | 10.0 |
| Waves, routes and arenas | 20% | 60% | 12.0 |
| Co-op and recovery | 20% | 70% | 14.0 |
| Perks and progression | 10% | 75% | 7.5 |

Consult this folder before test selection or readiness answers. Run `powershell -NoProfile -File docs/beta-readiness/Check-Readiness.ps1` to compare input hashes. Reuse saved evidence only at its recorded date and validation level; changed inputs and environment/package/network differences require affected checks.

VS34 reviews a 4.99-second owner recording of the perk menu. It exposes small text and controls; no combat or confirmed selection action is shown. All 550 VS33 input hashes match. Source is unchanged, prior evidence keeps its original scope, and the score remains 62%.

VS33 corrects upward physical recoil, adds six reinforced encounters per regular arena and two finale approach encounters, raises champion HP and scales enemy numbers/role health/damage for connected one-to-four-player parties. Final controlled solo and four-peer routes, a two-peer hosted restart, focused recoil and encounter rules, Editor/Shipping builds and bounded package startup pass. The score stays 62%; correctness and numeric tuning do not prove human balance or production completion. Review the dated test scopes before reuse.

Game source includes unpublished local changes. Host-owned checkpoints, AI/reclaim, Steam/Internet/invites/public matchmaking, survival, character preselection/shared reveal, complete secondary choices, ten finished arenas/pacing, production assets/audio and minimum-PC/human testing remain open. The owner supplied an Editor recording and reported the previous levels too short/easy and recoil reversed. Post-fix normal-input packaged human/co-op feedback remains open. Supervised internal alpha remains the recommendation; beta approval is false. Keep player-hosted matches initially; rented dedicated servers are a later option if sales/player activity support costs.

After relevant work, add a dated snapshot and update the latest pointer and this overview together.
