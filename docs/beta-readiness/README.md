# IRON SUN beta readiness

Latest gameplay planning estimate: **62%** (weighted 61.5), low confidence, assessed October 3, 2026. **NOT_BETA_READY.** The target is **75%; it remains in progress**. Overall beta completion remains unmeasured. These percentages are planning judgments, not test pass rates.

Start with [latest.json](latest.json), the [report](snapshots/2026-10-03-vs32/report.md), [scores](snapshots/2026-10-03-vs32/scores.json), [test results](snapshots/2026-10-03-vs32/test-results.json), [session guide](snapshots/2026-10-03-vs32/session-guide.md), [hosting direction](snapshots/2026-10-03-vs32/hosting-direction.md), and [remaining milestones](snapshots/2026-10-03-vs32/target-75.md). Older snapshots remain history.

| Area | Weight | Readiness | Points |
| --- | ---: | ---: | ---: |
| Controls and combat | 30% | 60% | 18.0 |
| Weapons and arsenal | 20% | 50% | 10.0 |
| Waves, routes and arenas | 20% | 60% | 12.0 |
| Co-op and recovery | 20% | 70% | 14.0 |
| Perks and progression | 10% | 75% | 7.5 |

Consult this folder before test selection or readiness answers. Run `powershell -NoProfile -File docs/beta-readiness/Check-Readiness.ps1` to compare input hashes. Reuse saved evidence only at its recorded date and validation level; changed inputs and environment/package/network differences require affected checks.

VS32 fixes hosted restart reopening the lobby, clears retained controller menus/queued choices, and creates advertised sessions only after the host's listening port binds. Two and four actual LAN peers pass two full team wipes/restarts, retained XP, normal setup, actual secondary draw, local/authoritative movement/fire/reload, late-join rejection and clean end. The four-player host renders through both travels. Fresh integrated route regression, Editor build, Shipping build/startup and package selector pass. Enemy AI/damage and input are controlled; no human, physical-controller, Internet or packaged multiplayer acceptance is inferred.

Game source includes unpublished local changes. Host-owned checkpoints, AI/reclaim, Steam/Internet/invites/public matchmaking, survival, character preselection/shared reveal, complete secondary choices, ten finished arenas/pacing, production assets/audio and minimum-PC/human testing remain open. The owner cannot currently human-test. Supervised internal alpha remains the recommendation; beta approval is false. Keep player-hosted matches initially; rented dedicated servers are a later option if sales/player activity support costs.

After relevant work, add a dated snapshot and update the latest pointer and this overview together.
