# VS33 — recoil, longer encounters and party difficulty

**62% gameplay planning estimate (61.5 weighted), low confidence. NOT_BETA_READY. The 75% target remains open.** Overall beta completion is unmeasured. Test passes are correctness evidence, not a readiness percentage.

The owner supplied a 7:42 Unreal Editor recording and reported levels ending too quickly, downward recoil, low boss health, low difficulty and insufficient party scaling. Sampled frames show one short victory and another defeat. This is owner playtest evidence; exact binary/source correspondence is unknown. The earlier statement that the owner cannot currently test is superseded.

Physical recoil now raises the owner's camera independent of look inversion. A focused regression failed twice before the fix and passes afterward. Actual shots raise aim during the final two-peer hosted restart check. Camera limits and input suppression still apply; visual weapon animation and fresh human recoil feel are unaccepted.

Regular arenas now have six encounters with four reinforcement groups. The finale has two approach encounters and the champion encounter. The route requires 15 or 21 encounters. Live counts are capped at 12–24; no victory or encounter-clear reward is available while groups remain. Between-encounter healing drops from 60 to 20 and reserve resupply rises to 72. These tuning choices need human ammo/difficulty feedback.

Connected party size, including downed teammates, is sampled per encounter. Enemy budgets, role HP and damage increase with player count; damage also scales with route depth. Boss health is 5,000 / 8,750 / 12,500 / 16,250 for one / two / three / four players. Legacy isolated combat fixtures keep their previous values. Foundry encounters include turrets. Deliberately strong or weak wildcards remain allowed.

Final validation covers encounter properties, a rendered controlled solo route, a four-process route and a two-peer LAN hosted restart. Solo verifies real trace damage at 2× baseline, live-cap fill, reinforcement/reward guards, objectives and champion phase. Four peers verify role HP/damage, replicated counters, cap fill, 21 encounters and shared victory. Hosted restart verifies two wipes, fresh pending state, retained XP, owned recoil/movement/fire/reload, late-join rejection and clean end. Editor and Shipping builds, bounded D3D12 package startup and the latest-package selector pass. Consult test-results.json for exact dates/scopes and attempt-corrections.md for failed attempts.

Controlled NPC freezes/defeats, teleports and synthetic key events do not measure normal difficulty, frame-time, human input or Internet play. Offscreen frames inspect the prototype HUD only. The 4–6-minute arena goal remains a tuning proposal; it is not measured, and current routes do not establish the intended 45–60-minute final run. Source changes remain local and unpublished; this GitHub snapshot publishes readiness evidence only.

Continue supervised internal alpha testing. Friends' external beta approval needs a normal-input packaged hosted session and the remaining gates in target-75.md. Preserve the player-hosted design; dedicated hosting is a later option.
