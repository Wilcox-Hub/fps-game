# VS37 — first cash and weapon progression loop

October 4, 2026. The approved design is implemented in the existing sci-fi sentry-arena prototype. The player confirmed upgrades at the existing arsenals during regroup breaks. Subsequent feedback that the stat display was confusing led to a compact six-stat comparison and an optional exact-details view.

## Playable changes

- Dependable entry weapon; known Gale/Phasma/Helix/Vector purchases and the existing free wildcard/reroll lane. Browse and compare before confirming; close or browse away to pass. Two primary slots plus the existing secondary lane remain.
- Server-authoritative personal run cash: 120 to each possessed teammate after an encounter, 180 each for one optional-objective completion, and 90 only to the successful difficult melee-finisher attacker. Downed teammates receive the shared rewards. Amounts and costs are provisional.
- Private weapon upgrades at a nearby real arsenal during regroup. Three levels cost 450/800/1,150. Each raises base damage, firing cadence and magazine capacity. The first choice locks Echo pulse or Through shot; subsequent levels retain it and the gun's original mode/payload/condition. Upgrades preserve ammunition; repeat owned purchases cannot charge or refill.
- Real delayed 35% direct follow-up hits or a 50% direct hit on one next sentry along the actual muzzle ray. Solid cover blocks penetration. Normal/brute/sprinter sentries weakened to 25% expose a 2.5-second finisher opening; ordinary melee/turret/champion attacks do not earn the bonus.
- 20-second regroup breaks with unanimous living-team early start. Stale requests are rejected; the next encounter closes shops. Objective interactions cannot bypass the active timer.
- Simple side-by-side damage, firing rate, accuracy, recoil, magazine and reload with rounded values/plain changes. **Tab/L3 Details** preserves exact deltas, DPS, angles and party/solo standard/brute/champion estimates. Modeling conditions are explicit. Chance payloads are expectations and splash assumes full blast; estimates are not measured hit probability or human effectiveness.

## Controls

**T** arsenal, **Left/Right** browse, **Enter** confirm, **U/I** upgrade previews, **J** primary/secondary, **4/5** primary replacement slot, **Tab** details, **G** wildcard reroll, **N** ready, **M/middle mouse** melee finisher. The screen also lists gamepad equivalents. A comparison or details toggle does not purchase a gun.

## Evidence and limits

[Test results](test-results.json) record the exact scopes and log hashes. The rendered economy integration includes shared payouts to a downed peer, private upgrade/ammo state, transaction rejection paths, real pulse damage matching 85.05 HP, an actual 26.775 HP second-target hit and a positive/independent cover control, private one-time finisher cash, a normal-world 20-second countdown and early start. A real local listen host/client pair verifies owner RPC/replication, equal payout, private spending and stale/duplicate/live-upgrade rejection. Targets are frozen/repositioned and transactions receive explicit controlled funding; these are correctness fixtures.

[Comparison frames](visual-check.json) inspect the final simple and exact-details panels in offscreen D3D12 at 1280x720. Final simulated-input and rendered PIE checks preserve selected weapons through firing/reload/actions/held sprint. Earlier affected combat/fire/collision/loadout checks are explicitly identified as preceding only the later presentation revision. A fresh accelerated-clock route fixture covers the finite prototype victory and progression gates; it does not measure normal run duration. The unfinished pre-revision route invocation was stopped to refresh the module and receives no pass credit. See [diagnosis](diagnosis.md) for screenshot, ray-alignment and cover-fixture corrections.

Final source compiles in UE 5.8.3, and matching source/config/content is built/cooked/staged/archived as a COMPLETE Shipping **Prototype**. The owned offscreen package survives the bounded 22-second D3D12 startup observation. This does not verify packaged full gameplay, friend sessions, audio, hardware input or frame-rate acceptance. The normal launcher selects the newest validated archive.

The manifest contains 559 Source/Config/Content inputs. [Changed inputs](changed-inputs.json) and [scoped reuse](scoped-reuse.json) identify changes and preserve the dates/limits of older evidence. The GitHub publication contains readiness evidence only; game source remains unpublished local work.

## Readiness

Retain the dated **62% gameplay planning estimate** (61.5 weighted, low confidence), overall beta readiness unmeasured, **NOT_BETA_READY**, and the 75% target in progress. The new loop requires normal-play cash balance, weapon value, finisher risk/reward and player comprehension/fun feedback. The main run remains three/four prototype arenas with 15/21 encounters. A successful **45–60-minute** finished main run remains a content/pacing target, not a measured result. The enemy attack rhythm, finished gates/spawn entrances, larger production arenas, art/audio, Steam/Internet and packaged four-human acceptance remain open.
