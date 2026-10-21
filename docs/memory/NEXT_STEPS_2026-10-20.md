# IRON SUN next steps - October 20, 2026

Suggested sequencing, based on current owner direction, local evidence and the
open GitHub tracker. This is a work inventory, not a new readiness score or a
claim that every open ticket still lacks implementation. No implementation,
build, test, source publication or deployment is started by this planning task.

October20 progress: items 1–4 below are prepared in the identified local
[VS69 candidate](BETA_CANDIDATE_2026-10-20.md). Diagnostics, extended controlled
co-op checks, matching Windows/Linux artifacts and the human card are complete.
Scoped 1080p refit/rack/optics and lobby/version checks also pass. The original
ammo failure and normal two-PC/human acceptance remain open; remaining parts of
items 5–8 and the larger roadmap still need work. No public rollout occurred.

## Best next block without a human play session

| Order | Work | Concrete result | Boundary |
| --- | --- | --- | --- |
| 1 | Add focused ammo diagnostics | Identified build plus per-orb type/age, collecting owner, actual gain/rejection and both carried gun pools; usable in a normal playtest | Original late-arena failure stays open until reproduced |
| 2 | Extend the real two-process scenario runner | Uninterrupted first-arena encounters with firing, reloads, both owners collecting, rapid swaps, repeated pickups and bounded network delay; additional carry/recovery regressions where useful | Add assertions for actual useful ammo, not merely vanished orbs; preserve all existing checks |
| 3 | Prepare a matching test candidate | Review changed RPC/network compatibility; build matching local Windows clients and server artifacts when needed; record exact versions, hashes and rollback instructions | Package preparation is separate from publishing to the launcher/server |
| 4 | Prepare the next human test | Short first-arena checklist and recording/log collection covering ammo, held-E rescue, refit and stair repair | Normal two-PC/human acceptance remains pending |
| 5 | Inspect current presentation | Actual current pod/medical states, interaction prompts, refit price/hints, rack/ADS views and readable menus at 1080p/1440p/4K | Screenshots/renders can be reviewed remotely; control feel requires playing |
| 6 | Review performance on available hardware | CPU/GPU frame-time, memory and long-session evidence; actionable bottlenecks while retaining approved quality | This machine cannot certify an unspecified minimum PC or normal multi-player server load |
| 7 | Audit remaining delivery/session paths | Targeted local automation for Continue/rejoin, both guns/resources retained, bad-build rejection and team-wipe/restart; rerun only affected or genuinely new scopes | Existing matching evidence keeps its original limits |
| 8 | Reconcile the tracker with actual work | Mark each milestone as missing, locally implemented, technically checked, awaiting delivery or awaiting player acceptance in the work record | Prepare issue edits for review; do not equate an open issue with missing code |

The first three items were the initial sequence and now have local technical
evidence. Next use the human card when normal co-op is available; while away,
remaining presentation/performance/session audits can follow their approved
dependencies without repeating unchanged checks.

## Decisions that can be discussed remotely

1. Define the active shield's form and behavior before implementing it. A passive
   team Shield bonus does not settle this tactical's design.
2. Review current healing-pod screenshots and approved character/weapon references.
   Missing reference sheets need recovery before matching their appearance.
3. Set the intended minimum PC and performance target before claiming that gate.
4. Revisit the final title/mark when wanted; IRON SUN remains the working title.
5. Settle remaining menu/lobby motion and original audio treatment during its
   future polish pass. Revisit numerical weapon/refit tuning using play evidence.

Design questions use the automatic interview workflow when that frontier is
chosen: save exact prompts/options, capture replies and check current decisions.
Resolved two-slot, preparation and control decisions stay resolved.

## When normal two-player play is available

1. Use an identified matching candidate on both PCs and the matching host/server.
   Trace the original Gale/Phasma pickup failure across every first-arena wave,
   including depleted magazines and reserves. Preserve video and diagnostic logs.
2. Try E/View hold rescue, cancellation/progress, carry, tube deposit, interruption,
   recovery and useful ammo afterward, alternating who rescues whom. Judge the
   provisional 0.65-second hold by actual feel.
3. Try rack selection and walk-up E refit: cost/general hint, shared occupancy,
   collection by the owner, original slot, firing the other gun during refit.
4. Check the repaired stair join, arena gates/walls, optics and moving/covered
   enemy hit registration; review the actual healing tube and readable HUD.
5. Test real separate-PC/controller input, different-network friend joins, both
   guns firing, save/close/reopen and original-party Continue. Internet/Steam
   features need their implemented service paths before acceptance testing.
6. Do longer normal sessions: opening-round ease, later pressure/role variety,
   cover use, ammo/shard economy, wildcard/refit usefulness and team hazards.
   Include four-human/server-load checks as the candidate supports them.
7. Run blind readability/fun tests and a successful 45-60-minute progression run.
   Those human results are needed for beta acceptance; automated passes do not
   decide pacing, quality or fun.

## Remaining larger roadmap

| Area | Finish and verify | Tracker |
| --- | --- | --- |
| Foundation | Controls, recoil/hit feedback, menus/run/failure/restart, visual/performance targets, easy installation and smooth launching | #3-7, #57-60 |
| Arena systems | Modular objectives, recoverable arena/species resupply, optional-objective outcomes and fair progression | #8, #23 |
| Online | Session lifecycle, authoritative combat/objectives, private play, public queue/Steam invites, disconnect/AI takeover/reclaim | #9-12, #19 |
| Lobby/progression | Character preselection/shared reveal, approved perks/team vote, routes, regroup and host-owned save/Continue | #13-18 |
| Weapons | Approved two carried slots, Gale/Phasma differentiation, safe meaningful wildcards, physical upgrades and earned rerolls | #20-24 |
| Enemies | Purposeful pursuit/navigation, seven-plus roles, species variants, rotating turrets, elites and champions | #25-28 |
| Presentation | Sparse readable menus/HUD, original weapon/enemy/impact audio, broadcast lore and reactive audience | #29-31 |
| Arena content | Complete the ten-arena objective/layout and species presentation target; audit existing prototypes before filling missing slices | #32-51 |
| Modes | Future HOLDOUT single-arena survival; retain only currently playable modes in the UI | #52 |
| Final quality | Matching packages, profiling/soaks, multiplayer odd paths, blind co-op tests and explicit beta-gate audit | #53-56 |

Many of these have partial/local implementation. Use current decisions and
dated evidence before selecting a ticket. Historical titles such as three-slot
loadouts, timed preparation and a ten-node perk tree require reconciliation
with the approved two carried guns, no forced initial countdown and four
immediately available perks with two equipped. Do not reintroduce old rules.

## Paused and optional setup

- The 100-roll wildcard output/distribution audit remains paused by the owner.
  Its requested review/change suggestion is still pending; this plan does not
  resume it. Ordinary focused safety regressions are a separate scope.
- Roll/dash and lobby motion retain their recorded deferred status.
- The second ChatGPT login and Windows account-switch-and-return check can wait.
- When asset work actually needs them, finish/check Meshy credentials, Tripo
  credits and the live Blender/add-on connection. Paid services require their
  separately approved costs; game coding is not blocked by them.
- Recover the five historical missing answers only when relevant source material
  is available. Do not invent answers or reopen already superseded decisions.

## Sources and validation limits

- [Current work](active-work.json), [issues](open-issues.json),
  [current rules](../design-decisions/README.md), and [conflicts](conflicts.json).
- [October20 ammo checks](../beta-readiness/snapshots/2026-10-20-host-ammo-scenarios/report.md):
  12/12 actual two-process cases plus one existing rescue recipe passed locally;
  original normal-play failure remains unreproduced. All 760 current game input
  hashes matched that snapshot during this planning task. No new engine tests ran.
- [Latest tested record](../beta-readiness/latest.json) and
  [scope/reuse guidance](../beta-readiness/README.md).
- [GitHub roadmap](https://github.com/Wilcox-Hub/fps-game/issues/1), queried live
  for this inventory. Public game remains VS68; newer local work is unpublished.

## Complete open GitHub inventory

The tracker currently has 58 open work tickets plus its roadmap. Dependencies
below are the tracker metadata, not a fresh implementation assessment. Current
human corrections govern stale wording. This section includes every open issue
returned by the repository query; selecting work still requires its relevant
rules and evidence.

### Roadmap

- [#1 IRON SUN beta — wayfinder map](https://github.com/Wilcox-Hub/fps-game/issues/1)

### Foundation

- [#3 [IRON SUN] 02: Repair responsive FPS controls](https://github.com/Wilcox-Hub/fps-game/issues/3) - tracker dependencies: #2
- [#4 [IRON SUN] 03: Repair combat damage and feedback](https://github.com/Wilcox-Hub/fps-game/issues/4) - tracker dependencies: #2, #3
- [#5 [IRON SUN] 04: Stabilize run, menu, and failure states](https://github.com/Wilcox-Hub/fps-game/issues/5) - tracker dependencies: #2, #3
- [#6 [IRON SUN] 05: Establish repeatable beta test harness](https://github.com/Wilcox-Hub/fps-game/issues/6) - tracker dependencies: #2, #4, #5
- [#7 [IRON SUN] 06: Set visual and performance targets](https://github.com/Wilcox-Hub/fps-game/issues/7) - tracker dependencies: #2

### Systems

- [#8 [IRON SUN] 07: Create modular arena objective framework](https://github.com/Wilcox-Hub/fps-game/issues/8) - tracker dependencies: #4, #5

### Online

- [#9 [IRON SUN] 08: Create online session lifecycle](https://github.com/Wilcox-Hub/fps-game/issues/9) - tracker dependencies: #5
- [#10 [IRON SUN] 09: Replicate authoritative combat and objectives](https://github.com/Wilcox-Hub/fps-game/issues/10) - tracker dependencies: #4, #8, #9
- [#11 [IRON SUN] 10: Support disconnect, AI takeover, and reclaim](https://github.com/Wilcox-Hub/fps-game/issues/11) - tracker dependencies: #9, #10, #19
- [#12 [IRON SUN] 11: Add private sessions, public queue, and Steam invites](https://github.com/Wilcox-Hub/fps-game/issues/12) - tracker dependencies: #9, #10
- [#19 [IRON SUN] 18: Build downed carry and healing-tube revive](https://github.com/Wilcox-Hub/fps-game/issues/19) - tracker dependencies: #8, #10

### Progression

- [#13 [IRON SUN] 12: Build timed pre-run lobby and shared perk vote](https://github.com/Wilcox-Hub/fps-game/issues/13) - tracker dependencies: #9, #14, #15
- [#14 [IRON SUN] 13: Build three playable species and blades](https://github.com/Wilcox-Hub/fps-game/issues/14) - tracker dependencies: #3, #4, #7
- [#15 [IRON SUN] 14: Build personal XP and ten-node perk tree](https://github.com/Wilcox-Hub/fps-game/issues/15) - tracker dependencies: #5, #10
- [#16 [IRON SUN] 15: Build temporary team arena perks](https://github.com/Wilcox-Hub/fps-game/issues/16) - tracker dependencies: #8, #10, #15
- [#17 [IRON SUN] 16: Generate and vote on progression routes](https://github.com/Wilcox-Hub/fps-game/issues/17) - tracker dependencies: #8, #9
- [#18 [IRON SUN] 17: Implement safe intermissions and resume](https://github.com/Wilcox-Hub/fps-game/issues/18) - tracker dependencies: #8, #10, #17

### Weapons

- [#20 [IRON SUN] 19: Build arsenal and three-slot loadout rules](https://github.com/Wilcox-Hub/fps-game/issues/20) - tracker dependencies: #4, #8, #9
- [#21 [IRON SUN] 20: Differentiate Gale and Phasma end to end](https://github.com/Wilcox-Hub/fps-game/issues/21) - tracker dependencies: #4, #20
- [#22 [IRON SUN] 21: Make wildcard combinations real and safe](https://github.com/Wilcox-Hub/fps-game/issues/22) - tracker dependencies: #4, #20
- [#23 [IRON SUN] 22: Implement limited ammo and species resupply](https://github.com/Wilcox-Hub/fps-game/issues/23) - tracker dependencies: #8, #20
- [#24 [IRON SUN] 23: Add earned wildcard rerolls](https://github.com/Wilcox-Hub/fps-game/issues/24) - tracker dependencies: #20, #22

### Enemies

- [#25 [IRON SUN] 24: Repair enemy pursuit, attacks, and navigation](https://github.com/Wilcox-Hub/fps-game/issues/25) - tracker dependencies: #4, #8
- [#26 [IRON SUN] 25: Build rotating three-cannon turrets](https://github.com/Wilcox-Hub/fps-game/issues/26) - tracker dependencies: #25, #7
- [#27 [IRON SUN] 26: Build seven-plus enemy roles and species variants](https://github.com/Wilcox-Hub/fps-game/issues/27) - tracker dependencies: #25, #7
- [#28 [IRON SUN] 27: Build arena elites and route-changing champion](https://github.com/Wilcox-Hub/fps-game/issues/28) - tracker dependencies: #8, #25, #27

### Presentation

- [#29 [IRON SUN] 28: Add broadcast lore and reactive audience audio](https://github.com/Wilcox-Hub/fps-game/issues/29) - tracker dependencies: #8, #7
- [#30 [IRON SUN] 29: Polish menus, HUD, and resolution behavior](https://github.com/Wilcox-Hub/fps-game/issues/30) - tracker dependencies: #5, #13, #17, #20
- [#31 [IRON SUN] 30: Polish weapons, enemies, and impact audio](https://github.com/Wilcox-Hub/fps-game/issues/31) - tracker dependencies: #4, #7, #21, #25

### Arenas

- [#32 [IRON SUN] 31: Brassfall Crucible playable objective](https://github.com/Wilcox-Hub/fps-game/issues/32) - tracker dependencies: #8, #25, #26
- [#33 [IRON SUN] 32: Brassfall Crucible species presentation](https://github.com/Wilcox-Hub/fps-game/issues/33) - tracker dependencies: #7, #27, #28, #31, #32
- [#34 [IRON SUN] 33: The Breathing Garden playable objective](https://github.com/Wilcox-Hub/fps-game/issues/34) - tracker dependencies: #8, #25
- [#35 [IRON SUN] 34: The Breathing Garden species presentation](https://github.com/Wilcox-Hub/fps-game/issues/35) - tracker dependencies: #7, #27, #28, #31, #34
- [#36 [IRON SUN] 35: Halo Wreckfield playable objective](https://github.com/Wilcox-Hub/fps-game/issues/36) - tracker dependencies: #8, #25
- [#37 [IRON SUN] 36: Halo Wreckfield species presentation](https://github.com/Wilcox-Hub/fps-game/issues/37) - tracker dependencies: #7, #27, #28, #31, #36
- [#38 [IRON SUN] 37: Nacre Crown playable objective](https://github.com/Wilcox-Hub/fps-game/issues/38) - tracker dependencies: #8, #25
- [#39 [IRON SUN] 38: Nacre Crown species presentation](https://github.com/Wilcox-Hub/fps-game/issues/39) - tracker dependencies: #7, #27, #28, #31, #38
- [#40 [IRON SUN] 39: Mirror Delta playable objective](https://github.com/Wilcox-Hub/fps-game/issues/40) - tracker dependencies: #8, #25
- [#41 [IRON SUN] 40: Mirror Delta species presentation](https://github.com/Wilcox-Hub/fps-game/issues/41) - tracker dependencies: #7, #27, #28, #31, #40
- [#42 [IRON SUN] 41: Storm Loom playable objective](https://github.com/Wilcox-Hub/fps-game/issues/42) - tracker dependencies: #8, #25
- [#43 [IRON SUN] 42: Storm Loom species presentation](https://github.com/Wilcox-Hub/fps-game/issues/43) - tracker dependencies: #7, #27, #28, #31, #42
- [#44 [IRON SUN] 43: Below the Crown playable objective](https://github.com/Wilcox-Hub/fps-game/issues/44) - tracker dependencies: #8, #25
- [#45 [IRON SUN] 44: Below the Crown species presentation](https://github.com/Wilcox-Hub/fps-game/issues/45) - tracker dependencies: #7, #27, #28, #31, #44
- [#46 [IRON SUN] 45: The Silent Choir playable objective](https://github.com/Wilcox-Hub/fps-game/issues/46) - tracker dependencies: #8, #25
- [#47 [IRON SUN] 46: The Silent Choir species presentation](https://github.com/Wilcox-Hub/fps-game/issues/47) - tracker dependencies: #7, #27, #28, #31, #46
- [#48 [IRON SUN] 47: Redline Bazaar playable objective](https://github.com/Wilcox-Hub/fps-game/issues/48) - tracker dependencies: #8, #25
- [#49 [IRON SUN] 48: Redline Bazaar species presentation](https://github.com/Wilcox-Hub/fps-game/issues/49) - tracker dependencies: #7, #27, #28, #31, #48
- [#50 [IRON SUN] 49: Last-Star Observatory playable objective](https://github.com/Wilcox-Hub/fps-game/issues/50) - tracker dependencies: #8, #25, #28
- [#51 [IRON SUN] 50: Last-Star Observatory species presentation](https://github.com/Wilcox-Hub/fps-game/issues/51) - tracker dependencies: #7, #27, #28, #31, #50

### Modes

- [#52 [IRON SUN] 51: Finish single-arena survival mode](https://github.com/Wilcox-Hub/fps-game/issues/52) - tracker dependencies: #8, #19, #20, #23, #25

### Qa

- [#53 [IRON SUN] 52: Package and profile the beta candidate](https://github.com/Wilcox-Hub/fps-game/issues/53) - tracker dependencies: #12, #18, #24, #27, #30, #33, #35, #37, #39, #41, #43, #45, #47, #49, #51, #52
- [#54 [IRON SUN] 53: Automate multiplayer and odd-path regression](https://github.com/Wilcox-Hub/fps-game/issues/54) - tracker dependencies: #6, #11, #12, #13, #17, #18, #19, #23, #24, #28, #52
- [#55 [IRON SUN] 54: Run blind co-op fun and readability tests](https://github.com/Wilcox-Hub/fps-game/issues/55) - tracker dependencies: #53, #54
- [#56 [IRON SUN] 55: Audit and sign off the beta gate](https://github.com/Wilcox-Hub/fps-game/issues/56) - tracker dependencies: #53, #54, #55
- [#57 [IRON SUN] Make the PC build easy to launch and smooth without reducing quality](https://github.com/Wilcox-Hub/fps-game/issues/57) - tracker dependencies: #7

### Follow Up

- [#58 [IRON SUN] Rebuild sentry death, scale, arena footprint, and physical arsenal](https://github.com/Wilcox-Hub/fps-game/issues/58)
- [#59 [IRON SUN] Replicate rebuilt arena geometry and blockout removal to clients](https://github.com/Wilcox-Hub/fps-game/issues/59)
- [#60 [IRON SUN] End co-op runs on team wipe, not first player defeat](https://github.com/Wilcox-Hub/fps-game/issues/60)

