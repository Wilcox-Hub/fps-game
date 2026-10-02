# IRON SUN beta readiness

**Latest recorded gameplay readiness: approximately 35% (34.5/100 weighted), low confidence. NOT beta-ready.** The estimate dates from September 29, 2026; the September 30 VS25 audit confirmed the same evidence and no increase. The target is 50%. Overall beta completion has not been measured by this rubric.

Start with [latest.json](latest.json). It points to the latest [scores](snapshots/2026-09-30-vs25/scores.json), [readiness report](snapshots/2026-09-30-vs25/report.md), [test results](snapshots/2026-09-30-vs25/test-results.json), and [test excerpts](snapshots/2026-09-30-vs25/test-excerpts.txt). These are archived results, not newly executed tests.

| Area | Weight | Readiness | Weighted points |
| --- | ---: | ---: | ---: |
| Controls and combat | 30% | 55% | 16.5 |
| Weapons and arsenal | 20% | 40% | 8.0 |
| Waves, routes and arenas | 20% | 30% | 6.0 |
| Co-op and recovery | 20% | 20% | 4.0 |
| Perks and progression | 10% | Unverified; 0 credited | 0.0 |
| Total | 100% | approximately 35% | 34.5 |

The controlled technical gates passed: 100 generated combat cases, scatter paths, 27 equipment switches, authoritative fire, loadout interactions, 12 focused helper tests, solo flow, local two-peer co-op, and healing-tube recovery. Existing Shipping startup survived 22.24 seconds. The [full report](snapshots/2026-09-30-vs25/report.md) preserves each gate's limits. Passing these tests does not establish normal-input packaged play, internet co-op, finished art, performance, or human fun.

## Reuse saved evidence

For a beta status question, read this folder first. Report the recorded date, scope, percentage, and remaining gates. A status question alone does not require another game test run.

Before applying saved passes to current code, compare its inputs with the snapshot:

```powershell
powershell -NoProfile -File docs/beta-readiness/Check-Readiness.ps1
```

This hashes Source/Config/Content files; it never starts Unreal. `reusable_input_evidence: true` means those tested inputs match. A mismatch lists changed/added/missing files. Reuse unaffected evidence after checking dependencies; run checks affected by changes and update the report. A match still requires fresh evidence for a different project descriptor, build/package, engine, machine, platform, network service, or validation level.

The VS25 tests cover a local working tree with unpublished changes. GitHub's game source may not match it. The [529-file manifest](snapshots/2026-09-30-vs25/input-manifest.json) identifies the tested Source/Config/Content precisely; the baseline Git commit alone does not. Raw logs, game binaries and screenshots remain local; portable summaries, log hashes and success excerpts are archived here. Missing raw artifacts limit independent verification.

## Record new work

Create a new `snapshots/YYYY-MM-DD-id/` directory after an assessment or relevant test run. Keep old snapshots as history. Include scores with the same weights, rationale for changed scores, per-gate pass/fail/untested status and validation level, commands, source hashes, engine/build/package identity, evidence, and unresolved gates. Point `latest.json` to the new snapshot and update this overview in the same GitHub commit. Record failed or partial runs too; retain the last valid passes with their original source scope.

Keep the ten-finished-arena beta target, online lifecycle, progression, art, hardware and human-testing gates visible. Change the rubric only with an explicit explanation; issue counts and test-pass rates are not readiness percentages.
