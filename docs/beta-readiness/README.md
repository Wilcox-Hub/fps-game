# IRON SUN beta readiness

Latest recorded gameplay planning estimate: **44%**, low confidence, assessed October 2, 2026. **NOT_BETA_READY.** The 50% target remains open; overall beta completion has not been measured.

Start with [latest.json](latest.json), then its [report](snapshots/2026-10-02-vs27/report.md), [scores](snapshots/2026-10-02-vs27/scores.json), [test results](snapshots/2026-10-02-vs27/test-results.json) and [progression guide](snapshots/2026-10-02-vs27/progression-guide.md). Older snapshots remain history.

| Area | Weight | Readiness | Points |
| --- | ---: | ---: | ---: |
| Controls and combat | 30% | 55% | 16.5 |
| Weapons and arsenal | 20% | 40% | 8.0 |
| Waves, routes and arenas | 20% | 35% | 7.0 |
| Co-op and recovery | 20% | 30% | 6.0 |
| Perks and progression | 10% | 65% | 6.5 |

For a readiness question, consult this folder before starting tests. Compare current inputs using `powershell -NoProfile -File docs/beta-readiness/Check-Readiness.ps1`. A matching hash set allows reuse only at the saved validation level. Changed inputs require affected checks; an engine/platform/package/network change requires corresponding evidence. These checks never start Unreal.

Game source includes unpublished local changes; GitHub source may differ. A score is a dated planning judgment, not a test pass rate. Preserve the ten-finished-arena beta target and the hardware, production-art, internet, performance and human-fun gates. After relevant work, add a dated snapshot and update this overview and latest pointer together.
