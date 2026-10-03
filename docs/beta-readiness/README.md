# IRON SUN beta readiness

Latest gameplay planning estimate: **58%** (weighted 57.5), low confidence, assessed October 3, 2026. **NOT_BETA_READY.** The current target is **75%; it remains in progress**. Overall beta completion remains unmeasured. These percentages are planning judgments, not test pass rates.

Start with [latest.json](latest.json), the [report](snapshots/2026-10-03-vs30/report.md), [scores](snapshots/2026-10-03-vs30/scores.json), [test results](snapshots/2026-10-03-vs30/test-results.json), [progression guide](snapshots/2026-10-03-vs30/progression-guide.md), and [remaining 75% milestones](snapshots/2026-10-03-vs30/target-75.md). Older snapshots remain history, including the [developer release review](snapshots/2026-10-03-release-review/report.md).

| Area | Weight | Readiness | Points |
| --- | ---: | ---: | ---: |
| Controls and combat | 30% | 60% | 18.0 |
| Weapons and arsenal | 20% | 50% | 10.0 |
| Waves, routes and arenas | 20% | 60% | 12.0 |
| Co-op and recovery | 20% | 50% | 10.0 |
| Perks and progression | 10% | 75% | 7.5 |

Consult this folder before selecting tests or answering readiness questions. Compare current inputs with `powershell -NoProfile -File docs/beta-readiness/Check-Readiness.ps1`. Matching hashes permit reuse only at the recorded date and validation level. Changed inputs require affected checks; engine, platform, package and network changes require corresponding evidence. See individual gate notes for scoped reuse and the rendered report wrapper limitation.

VS30 adds finite three/four-arena prototype routes, majority ballots with seeded ties, deadline/disconnect handling, Mirror Delta relays, Last Star lens controls, a guarded champion phase and one terminal victory. Twenty thousand seeded route cases pass. Four local Editor peers share a controlled victory; a departing route voter leaves three eligible peers who finish. Final four-peer actual sixty-second recovery passes. A new Windows Shipping prototype builds and survives a D3D12 startup check. None of these is a normal-input packaged human full run.

Game source includes unpublished local changes; GitHub source may differ. Four playable prototypes do not satisfy the ten-finished-arena target. Human playtests are pending because the owner cannot test currently; no human acceptance was invented. Integrated lobby/Steam, takeover/reclaim, saved checkpoints, survival mode, production art/audio and minimum-PC profiling remain open.

Supervised internal alpha remains the testing recommendation. Beta release approval remains false. After relevant work, add a dated snapshot and update this overview and the latest pointer together.
