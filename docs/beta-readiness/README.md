# IRON SUN beta readiness

Latest gameplay planning estimate: **50%**, low confidence, assessed October 2, 2026. **NOT_BETA_READY.** The provisional 50% gameplay milestone is reached; overall beta completion remains unmeasured. This is a planning judgment, not a test pass rate.

Start with [latest.json](latest.json), its [report](snapshots/2026-10-02-vs29/report.md), [scores](snapshots/2026-10-02-vs29/scores.json), [test results](snapshots/2026-10-02-vs29/test-results.json), and [progression guide](snapshots/2026-10-02-vs29/progression-guide.md). Older snapshots remain history.

| Area | Weight | Readiness | Points |
| --- | ---: | ---: | ---: |
| Controls and combat | 30% | 55% | 16.5 |
| Weapons and arsenal | 20% | 50% | 10.0 |
| Waves, routes and arenas | 20% | 35% | 7.0 |
| Co-op and recovery | 20% | 45% | 9.0 |
| Perks and progression | 10% | 75% | 7.5 |

For readiness questions, consult this folder before starting tests. Compare current inputs using `powershell -NoProfile -File docs/beta-readiness/Check-Readiness.ps1`. Matching hashes allow reuse only at the recorded validation level. Changed inputs require affected checks; an engine, platform, package or network change requires corresponding evidence. These checks never start Unreal. See individual gate reuse notes for scoped evidence retained from earlier VS29 iterations.

Game source includes unpublished local changes; GitHub source may differ. Preserve the ten-finished-arena beta target and the hardware, production-art, Internet, performance and human-fun gates. After relevant work, add a dated snapshot and update this overview and the latest pointer together.

VS29 adds timed perk/secondary/vote stages, secondary reservation and timeout deployment. Four actual local Editor peers exercise ballots and owner loadouts; a disconnect scenario and real sixty-second remote-player recovery also pass. This does not establish four-human full runs or Internet readiness.
