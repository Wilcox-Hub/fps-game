# VS29 timed setup and four-peer recovery — October 2, 2026

**50% gameplay planning estimate**, low confidence with a judgment range of 35–60%. **NOT_BETA_READY.** Overall beta completion remains unmeasured. The six-point change from VS28 comes from arsenal +2, co-op/recovery +3 and staged progression +1; controls and arena estimates are retained. These subjective category judgments acknowledge tested functionality and remaining gaps, rather than treating passed tests as completion percentages. [Scores](scores.json) contain the rationale for each category.

## Player-facing result

Normal runs now guide players through twenty seconds for their two saved personal perks, ten seconds for a secondary, and ten seconds for the team vote, followed by up to twelve seconds for the entry primary. Each stage locks when it ends. Existing three-offer weapons are used; a selected secondary is visibly reserved from both primary slots. Missing selections keep valid saved perks, default the secondary to Phasma and the primary to Gale. Reserving Gale as secondary instead defaults the primary to Phasma, avoiding a deployment block. Keyboard/gamepad selection and the final secondary/team result are visible. Character preselection and a shared reveal animation remain unfinished.

The universal crouch brace, Stable Grip, Medic rescue harness and Combat/Medic/Movement choices from VS28 are retained. See the [guide](progression-guide.md).

## Evidence

Normal 20/10/10 timers, stage locks, invalid/duplicate requests, saved defaults, keyboard/gamepad secondary changes, primary reservation, deployment and actual secondary draw passed in real standalone Editor fixtures. Ten inspected D3D12 frames cover five setup/deployment states at 1280x720 and 800x320. The immediate-deployment frame at 1280 contains an Editor shader-warmup overlay; prototype geometry/materials remain unfinished.

Four real local Editor processes pass owner-replicated stage/loadout state, weighted ballots and unanimous skip. Disconnecting one client during secondary selection leaves exactly three eligible voters; the remaining players deploy. Repeated/invalid ballot and late secondary/primary-reservation requests are rejected. These are real local networking checks with controlled inputs, not Internet or hardware acceptance.

A further four-peer scenario downs an actual remote owned pawn while the team remains active, carries/deposits that same pawn, awards exactly one enemy-death XP payment to all four profiles including the patient, and waits through sixty real seconds of recovery. The server validates depositor XP once and every remote client observes the same restored patient. Positioning is constructed and enemies are disabled during recovery; it does not establish difficulty, four-human combat pacing, or rescue fun.

Three unlock/effect/save/isolation tests, the existing possessed combat/carry/XP/arena-transition regression, and real two-peer fire/reload/shared-wipe/progression-preserving restart checks passed. Older combat fixtures use an Editor-only short setup path to isolate their existing scenarios; new setup fixtures use the normal production timers. Some earlier VS29 default/compact/skip/disconnect evidence is retained only for unchanged branches, with precise notes in [test results](test-results.json). The later Gale-secondary fallback has a fresh dedicated regression. No earlier whole-source pass is silently reapplied.

The final Shipping prototype built/cooked successfully and survived 22 seconds of offscreen D3D12 startup observation. This is startup evidence only; normal-input packaged full-run play remains unaccepted. The local launcher selects the completed package dated `20261002-213234-746`. Source includes unpublished changes; this docs publication does not publish game implementation.

## Remaining beta gates

Ten finished species arenas, variable routes and champions; full character/online lobby; Steam/invites/Internet and disconnect takeover/reclaim; normal-input packaged two/four-human full runs; human perk/weapon/rescue balance; production art, animation and audio; agreed minimum-PC profiling and longer soaks. Prior Blender/GitHub/isolated asset-pipeline evidence remains scoped in VS27 and receives no new gameplay score credit.
