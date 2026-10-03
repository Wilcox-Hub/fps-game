# IRON SUN beta readiness

Latest gameplay planning estimate: **61%** (weighted 60.5), low confidence, assessed October 3, 2026. **NOT_BETA_READY.** The target is **75%; it remains in progress**. Overall beta completion remains unmeasured. These percentages are planning judgments, not test pass rates.

Start with [latest.json](latest.json), the [report](snapshots/2026-10-03-vs31/report.md), [scores](snapshots/2026-10-03-vs31/scores.json), [test results](snapshots/2026-10-03-vs31/test-results.json), [session guide](snapshots/2026-10-03-vs31/session-guide.md), and [remaining 75% milestones](snapshots/2026-10-03-vs31/target-75.md). Older snapshots, including VS30's route foundation and the developer release review, remain history.

| Area | Weight | Readiness | Points |
| --- | ---: | ---: | ---: |
| Controls and combat | 30% | 60% | 18.0 |
| Weapons and arsenal | 20% | 50% | 10.0 |
| Waves, routes and arenas | 20% | 60% | 12.0 |
| Co-op and recovery | 20% | 65% | 13.0 |
| Perks and progression | 10% | 75% | 7.5 |

Consult this folder before test selection or readiness answers. Run `powershell -NoProfile -File docs/beta-readiness/Check-Readiness.ps1` to compare current input hashes. Reuse evidence only at its recorded date and validation level; affected changes and environment/package/network differences require corresponding checks. The snapshot records Source/Config/Content, with separate tools and package hashes.

VS31 integrates LAN discovery/host/join, a ready roster, authoritative launch into timed setup, leave/end/rehost and failure return. Two and four local Editor peers pass controlled lifecycle checks; final three surviving clients clean up after abrupt host loss, which currently takes the engine's 60-second connection timeout. Final normal route regression and Shipping build/startup pass. The actual lobby capture is inspected at 1280×720. These are controlled development checks, not normal-input packaged human full runs or Internet/Steam acceptance.

Game source includes unpublished local changes; GitHub source may differ. Human testing is pending because the owner cannot currently test. Steam/matchmaking, AI/reclaim, checkpoints, survival, ten finished arenas and intended pacing, production assets/audio and minimum-PC profiling remain open. No human acceptance is invented. Supervised internal alpha remains the recommendation; beta release approval remains false.

After relevant work, add a dated snapshot and update the latest pointer and this overview together.
