# VS25 — Repeat beta-readiness assessment

Date: 2026-09-30 Pacific / 2026-10-01 UTC. Target: IRON SUN, `unreal/Ridgefire`, UE 5.8.3. Compared with the completed VS24 regression/package run, not a different historical rubric.

## Verdict

**NOT beta-ready. Technical regression gates pass again; no demonstrated increase in beta completion.** All 529 Source/Config/Content inputs match the VS24 package snapshot and remain unchanged after this audit. No gameplay or art was changed by this run.

The latest gameplay-only planning estimate remains **approximately 35/100**, low confidence (25–45 judgment range). This is not a test-pass rate, overall game completion or measured fun score. Its weights remain controls/combat 30% at 55/100, weapons/arsenal 20% at 40/100, waves/routes/arenas 20% at 30/100, co-op/recovery 20% at 20/100, and perks/progression 10% with no evidence-backed credit: weighted 34.5. Repeated success on unchanged controlled fixtures does not establish a higher player-facing score. The old 18/100 all-field assessment uses a different date/rubric and is not a comparable current overall score.

## Like-for-like comparison

| Gate | Completed VS24 | Current VS25 | Meaning |
| --- | --- | --- | --- |
| Editor build | PASS, 0.92s | PASS, 0.76s; target up to date, zero compile actions | Incremental build health, not performance or a new game build |
| Generated wildcard combat | 100 distinct cases, nine modes PASS | Same 100 cases/nine modes PASS; every recorded case matches | Reproducible sampled API behavior, not full condition/kill-chain coverage |
| Scatter paths, beam anchoring | PASS | PASS, seven cases/37 paths/three transforms, reported worst error 0.000cm | Controlled owner-transform fixture, not sustained moving play |
| Equipment presentation | 27 switches PASS | 27 switches PASS | Bounded components/materials and attachments; not art sign-off |
| Authoritative fire | Auto/Pierce/Echo/Burst PASS | Same four-mode damage/cover/cadence/release checks PASS | Constructed actor/API checks |
| Arsenal and secondary | Bound-delegate interactions PASS | PASS headless and rendered at 1280×720 and 800×320 | Actual game bindings invoked; not physical keyboard/controller input |
| Focused helpers | 12 successes | 12 successes | Same helper suite |
| Solo flow | Damage/ammo/health/wave/transition/defeat/restart PASS | Same outcomes PASS | Automated game-state path, not a complete normal-input run |
| Local co-op | Two peers, damage/reload/rewards/wipe/repeated restart PASS | Same outcomes PASS | Local NullRHI peers; no internet, Steam or four-player combat acceptance |
| Healing-tube recovery | Original-player recovery/combat PASS | PASS, normal game time and autonomous sentry impacts | Local fixture; no full human co-op rescue acceptance |
| Shipping startup | 22.43s survived | 22.24s survived, D3D12/DXGI modules loaded | Same existing package; startup only, no Shipping visual/input/audio playtest |
| Full beta acceptance | OPEN | OPEN | No new complete-run, content, art, hardware or human-fun acceptance |

Thirteen current logs were independently checked for the selected fatal/automation/Ridgefire failure markers: none found. Equip-call timing was median 0.062ms/max 0.197ms (VS24 0.060/0.211); this is **not** FPS or a meaningful performance improvement claim. The compiler remains newer than Unreal's preferred version (MSVC 14.51 versus 14.50); the successful up-to-date build does not resolve that warning.

## Fresh rendered inspection

Lead inspected nine new offscreen Editor frames, not a continuous player playtest:

- `Saved/VisualCaptures/Scatter-1042301645931/Scatter-{5,11,3}.png`: gun-origin spread is visible in a staged cover fixture. The immediate three-pellet frame still has temporal afterimages, as in VS24. Exact active-path counts come from fixture assertions, not counting bright pixels. This fixture's observer HUD is not the clone's weapon identity/ammo.
- `Saved/VisualCaptures/Loadout-1042514650174/{ArsenalOpen,PrimaryAfterSecondary,SamePrimaryReturn}.png`, 1280×720: card layout and restored primary identity/ammo are visible. An Editor shader-preparation/debug overlay remains in the arsenal frame. Primitive weapon geometry and this staged skyward camera are not proof of a finished arena or enemy presentation.
- `Saved/VisualCaptures/Loadout-1042735894574/{ArsenalOpen,PrimaryAfterSecondary,SamePrimaryReturn}.png`, 800×320: cards reflow into stacked rows and HUD panels remain within the frame, but text becomes very small. This does **not** pass readability/accessibility or aspect-ratio acceptance. The shader overlay is also present in the arsenal frame.

These captures are stronger evidence of those specific staged layouts, not new gameplay features, finished textures/animation, full visual acceptance, remote tracer delivery, normal-input packaged combat, audio or fun. No current Shipping frames were captured or inspected. The native desktop-control/capture surface is unavailable; it was not bypassed.

## Remaining beta gates and next order

1. Finish and validate a responsive, readable single-arena vertical slice: normal-input current Shipping launch/selection/move/aim/fire/reload/swap/pause/death/restart/exit, plus sustained combat and arsenal use. Use `VS24_PLAYER_TEST_CARD.md`; source/API checks must not substitute for this (#22, #52, #58).
2. Integrate approved production arena/sentry/weapon assets and animations, and inspect actual enemies/deaths, close-camera occlusion, muzzle effects, arena traversal and original audio. Existing blockout and staged frames are not finished visuals (#7, #30, #58–59).
3. Complete/accept internet session lifecycle, Steam/public/private discovery, rescue/revive, disconnect AI takeover and reclaim, with real-device co-op evidence. Local two-peer success and earlier rescue-claim contention are not a four-player online run (#9–12).
4. Deliver tested personal XP/perks, temporary team perks, lobby/route voting, bosses/branches and the full progression loop (#13, #15–17, #28). Reviewed GameMode/controller interfaces provide no complete accepted progression loop.
5. Deliver **ten distinct finished beta arenas**, not the obsolete three-arena minimum. Current Iron Sun/Brassfall prototype phases and one shooter map do not satisfy that goal; themed blockouts are not completed content (#32–51).
6. Run minimum-PC profiling, longer crash/edge-case soaks, controller/resolution testing, and blind human co-op fun/readability sessions. Human fun remains unmeasured (#53, #55–56). The existing prototype archive has no staged prerequisite installer; its documented manual-runtime fallback still applies.

GitHub review found 59 open issues. Issue counts are not completion percentages; no issue was closed and no acceptance checkbox was advanced. Existing Gale Luna performed only a bounded read-only scope audit, saved separately as `VS25_SCOPE_AUDIT.md`; lead verified the cited transition/session seams and issue titles. No new chat, source edits, asset generation, commits or pushes.

## Evidence index

Project-relative logs under `Saved/Logs`:

- Motion/render: `IRON-SUN-Scatter-20260930-190600-044.log`; rendered loadouts: `IRON-SUN-Loadout-20260930-190623-456.log`, `190646-746`.
- Controlled reruns: `IRON-SUN-Wildcard-20260930-190728-902.log`, `IRON-SUN-Scatter-20260930-190744-730.log`, `IRON-SUN-Weapon-Presentation-20260930-190759-451.log`, `IRON-SUN-Weapon-Fire-20260930-190817-187-044ba91c.log`, `IRON-SUN-Loadout-20260930-190836-428.log`, `IRON-SUN-Beta-Smoke-20260930-190906.log`.
- Co-op: `RIDGEFIRE-COOP-20260930-190950-{Host,Client}.log`; recovery: `IRON-SUN-Recovery-20260930-191046-354.log`; helpers: `Saved/Automation/IRON-SUN-Focused-20260930-190853/Automation.log`.
- Existing COMPLETE Shipping Prototype: `Saved/Packages/20260930-185436-409/Windows/TP_FirstPerson.exe`; no redundant recook of unchanged inputs. Owned hidden/offscreen process was stopped after the startup check; user windows were not controlled.

Writable audit artifacts at `[local audit workspace]/research`: `VS25-Inputs-{Before,After}.json`, `Check-VS25-Evidence.ps1`, `VS25-Log-Checks.json`, `VS25-Comparison-Checks.json`, `IRON-SUN-VS25-Editor-Build.log`, and `VS24-Packaged-Startup-20260930-190730-258.json` (existing startup harness reused for VS25). The first evidence-check wrapper incorrectly tested an unset native exit-code variable after a PowerShell script; the script's assertions passed. Corrected invocation passed again plus the scoped Unreal diff check; this was not a game failure.

