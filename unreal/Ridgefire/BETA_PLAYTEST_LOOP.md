# IRON SUN: learn-to-beta loop

This is a **playability and fun process**, not a build checklist. A compiled game, a passing smoke test, and a good screenshot are separate pieces of evidence. None alone earns a beta sign-off. The lead game developer owns the scorecard and must play the packaged build before asking anyone else to test it.

## What we are making

The promise is a compact sci-fi gladiator FPS: choose Gale, chrome Phasma, or an intentionally unbalanced wildcard; survive sentry waves in a sunset arena; discover a new, sometimes absurd weapon story each run. The wildcard may be terrible or overpowered. **Technical correctness and readable feedback are mandatory; equal weapon balance is not.** Additional arenas must remain modular.

The enjoyable loop should be: **understand the offer → make a choice → move and aim → see a clear consequence → survive or fail → want another roll**. If any link is weak, add no new content until it is fixed.

## Evidence ladder

1. **Mechanic check:** a focused test proves inputs, state, collision, damage, ammo, transitions, and edge cases. A logged “shot fired” is insufficient; the intended target must react and the result must be visible.
2. **Packaged game check:** launch a freshly cooked Windows build, not just PIE. Play the whole loop with normal inputs: armory, three weapons, at least two waves, damage, healing, pause/resume, defeat, restart, and exit. Repeat unusual inputs (rapid swapping/firing, reload at zero/full ammo, pause mid-combat) and check 960×540, 1280×720, and 1920×1080. Check controller only if a controller is available; otherwise mark it untested.
3. **Visual/readability check:** capture before/after images and observe gun size and muzzle origin, enemy motion and attack telegraph, hit/kill feedback, HUD contrast, arena navigation, and effects/audio identity. Inspect moving action, not just a still frame.
4. **Blind human check:** at least five first-time players, 15–20 minutes each, no coaching for the first run. Record what they *do* before asking what they think. No test substitutes for this step when scoring fun.

For each feature, write a one-line hypothesis and a falsifiable prediction before coding. Example: “A visible sentry charge plus a short dodge window makes damage feel fair; at least four of five testers identify the attack and try to evade it.” Then record observed behavior, the smallest next change, and a retest. Do not count a feature as done merely because its code exists.

## Throughput: one vertical slice at a time

Use a short cycle: **choose the highest-impact problem → reproduce → change one slice → focused test → package/play → watch testers → score → keep, revise, or revert**. Prioritize by player impact, then frequency, then fix effort. Fix “cannot understand/shoot/survive/retry” before adding arenas or variants. Limit work in progress to one gameplay slice and one presentation slice; do not pile unverified features onto an unstable build.

Every slice has an owner, expected player-visible result, relevant files, acceptance criteria, and test evidence. Save the exact build path, test date, platform/resolution, result, and any blocker. A failure is useful learning, not a reason to soften the acceptance criterion. The lead reviews the integrated diff and plays the resulting package; no worker's self-report is a sign-off.

### Test card template

| Item | Record |
| --- | --- |
| Hypothesis / predicted player behavior | What should become easier or more enjoyable? |
| Build and environment | Exact package path, date, PC specs, resolution, controls |
| Reproduction / change | Inputs and expected observable result |
| Mechanic evidence | Test command and pass/fail; damage/state/ammo proof |
| Packaged play evidence | What actually happened through a full run; screenshot/video if useful |
| Human evidence | First-time player actions, confusion, delight, quotes with consent |
| Decision | Keep, revise, or revert; next highest-impact issue |

## Fun meter

After each blind session, ask four short 1–5 questions: **“How fun was that?”**, **“How much do you want one more run?”**, **“How surprising or distinct did the weapons feel?”**, and **“Could you tell why you won or lost?”** Also record whether the player voluntarily starts another run, time to first satisfying hit, abandoned runs, and one moment of delight or frustration. Do not coach or explain the wildcard before the first run.

For five or more independent first-time sessions, convert answers to a 0–25 fun score:

`25 × [0.40 × (fun−1)/4 + 0.30 × (one-more-run−1)/4 + 0.20 × (surprise−1)/4 + 0.10 × voluntary-replay-rate]`

Use the median response for each question and the observed replay rate. The clarity question is a diagnostic, not a way to inflate fun. Publish sample size and score distribution, not just the average. Below five sessions, **fun is unmeasured**; any planning score is a low-confidence proxy, not a player verdict.

Until human sessions are available, track a separate **expert fun-potential proxy**: score agency, combat feedback, surprise/uniqueness, pacing, and replay pull from 0–5 each. Use 0 for absent/unobservable, 1 for weak, 2 for functional but unrewarding, 3 for engaging, 4 for strong, and 5 for exceptional. Record the exact build, inputs, observations, and limitations. This proxy helps pick the next fix; it does **not** replace the human fun gate or increase beta readiness by itself.

## Beta gate: 85/100 is necessary, not sufficient

| Field | Weight | Beta minimum | Evidence required |
| --- | ---: | ---: | --- |
| Playability | 25 | 22 | Complete packaged runs with mouse/keyboard; no blocker in start, move, aim, fire, reload, swap, pause, death, restart, exit, or level transition. |
| Fun | 25 | 20 | Five-plus blind sessions; median fun and “one more run” at least 4/5; at least 4/5 players voluntarily replay or explicitly ask to. |
| Mechanics | 20 | 17 | Gale, Phasma, and wildcard feel functionally distinct; every authored mode, payload, and condition has a behavior check; 100+ generated combinations are technically valid and understandable, not necessarily balanced. |
| Content | 15 | 11 | At least **three playable arenas** with working transitions, distinct layouts and one arena-specific twist each; at least three meaningful enemy behaviors and three additional tested fun features (for example hazards, wave modifiers, or risk/reward pickups). |
| Visuals/audio | 10 | 7 | Coherent chrome/sunset identity; convincing sentry and gun silhouettes, muzzle/effect origin, readable HUD at the test resolutions, distinct and original weapon/audio feedback. |
| Reliability/QA | 5 | 4 | Clean Shipping package, automated smoke checks, 30-minute crash-free sessions on the agreed minimum PC, and recorded edge-path coverage. |

**Sign-off requires ≥85/100, every field above its minimum, and zero P0/P1 defects.** A P0/P1 is a crash, blocked loop, broken combat, lost controls, unreadable essential feedback, or a progression failure. Agree a minimum PC before claiming a performance pass. “Several levels” means three FPS arenas, not the unrelated FirstPerson/Horror template maps. A polished single arena cannot pass this gate.

## Baseline: 2026-09-23, revised after player feedback

**Revised provisional readiness: 18/100 (67 points below the 85-point beta gate; low confidence).** The earlier 37/100 estimate overvalued code-level checks and screenshots. The player's hands-on report that visuals are terrible and mechanics are slow or nonfunctional outweighs those checks. This is a readiness estimate, **not** a measured fun percentage or a prediction of development time.

| Field | Current | Why |
| --- | ---: | --- |
| Playability | 7/25 | The build launches and some isolated paths work, but the player's latest hands-on report says key mechanics are slow or not working; a full, reliable run is not demonstrated. |
| Fun | 3/25 proxy | The expert observation scored fun potential 8/25, but no human ratings or voluntary replay data exist. Actual player fun is **unmeasured**. |
| Mechanics | 3/20 | Offers and traits exist in code, but authored behavior and smoke assertions did not translate into a satisfying, reliable player experience. Treat weapon swap, weapon identity, hit response, and sentry behavior as unverified until reproduced in the current package. |
| Content | 2/15 | One intended FPS arena and one sentry family; no three-arena progression or three validated additional fun features. |
| Visuals/audio | 1/10 | The player describes the visuals as terrible. The palette and HUD panels do not make the guns, enemies, effects, or arena look finished. |
| Reliability/QA | 2/5 | Editor smoke logged 20 passes and a Shipping package built, but those checks missed issues the player encountered. There is no completed, repeatable end-to-end playtest or minimum-spec evidence. |

The last recorded package is `Saved/IRON-SUN-Core-Repair-QA/Windows/TP_FirstPerson.exe`; the smoke log is `Saved/Logs/IRON-SUN-Final-Core-Smoke.log`. Those are snapshots, not permanent guarantees. Re-run after changes. The shipped FPS map is `/Game/Variant_Shooter/Lvl_Shooter`; the other `.umap` files are template variants, not extra IRON SUN arenas.

**Next learning order:** (1) reproduce the player's reported failures in the exact package and write down inputs/results; (2) make movement, aiming, damage, firing, swapping, and sentry attacks reliable and legible; (3) replace the weak gun/enemy visuals and open up the arena while preserving the sunset/chrome identity; (4) prove wildcard modes and payloads create distinct play; (5) build additional arenas and replay features; (6) repeat the packaged playtest before changing the score. Never present this estimate as beta sign-off.

## Expert fun-potential observation: 2026-09-23

**Proxy: 8/25, low confidence. Human fun: unmeasured. The earlier 37/100 readiness estimate is superseded by the 18/100 reassessment above.** This was a short controlled input/visual observation of the Shipping package at 1280×720, not a full unassisted human session. The observer selected Gale, waited for wave one, moved forward while holding fire for roughly three seconds, then turned to search for threats. Screenshots: `Saved/IRON-SUN-Fun-Entry.png`, `Saved/IRON-SUN-Fun-Combat.png`, and `Saved/IRON-SUN-Fun-Look.png`.

| Dimension | Proxy | Evidence and limitation |
| --- | ---: | --- |
| Agency | 2/5 | Movement and sustained firing worked; ammunition fell from 24 to 5. No useful target choice appeared in the initial view. |
| Combat feedback | 2/5 | Muzzle beam, ammo, and health changes were visible; score stayed zero and there was no observed hit or kill payoff. Damage direction was not clear from the captured views. |
| Surprise / uniqueness | 2/5 | Three-offer and wildcard systems exist, but this short Gale run did not demonstrate a surprising interaction. Wildcard effects need observation, not just code inspection. |
| Pacing | 1/5 | Wave one had five sentries, yet the opening view showed none while health dropped from 92% to 91%, then later to 70%. The player could be hurt before locating a threat. |
| Replay pull | 1/5 | A new wildcard roll is a reason to retry, but only one compact arena and one sentry family are currently playable; voluntary replay cannot be measured without people. |

**Next experiment:** first reproduce and fix the reported input/mechanics failures in the exact package the player is testing. Then make at least one opening sentry visible and its first attack readable before it damages the player. Prediction: a blind player spots a threat, lands or intentionally attempts a shot within 30 seconds, and can explain the source of first damage. Then test distinct weapon feedback and ask whether the wildcard motivates another run. This observation did not test audio, a complete run, controller, or subjective enjoyment.

## Engineering regression check: 2026-09-27

**Readiness score unchanged at 18/100; human fun remains unmeasured.** The editor target compiled successfully with the current source. Headless editor smoke on `/Game/Variant_Shooter/Lvl_Shooter` passed repeated keyboard sprint press/release restoration, pause/resume, an actual weapon trace damaging a naturally positioned visible sentinel, sentry damage to the player, incoming-hit cue activation, hit/kill feedback, wave progression, resupply, defeat, and restart. Logs: `Saved/Logs/IRON-SUN-Ticket04-Incoming-Hit-Smoke.log` and `Saved/Logs/IRON-SUN-Ticket03-CombatTrace-Smoke.log`.

The old combat smoke had teleported its target into the reticle; that false-confidence setup was removed. The replacement still uses editor-only automation and sets the camera/controller aim programmatically. It is not raw hardware input or an ordinary packaged playtest. A current-source Shipping package including the hit cue and sentry telegraph was built at `Saved/Package-Ticket25-20260927`; earlier `Package-Ticket02-20260927` and `Package-Ticket04-20260927` snapshots are superseded. No package has been played through. Do not treat a successful cook as visual QA.

Code updates since the baseline disable legacy mouse smoothing and add a short source-direction hit arrow around the reticle. No controlled screenshots at all target resolutions, ordinary sentry firing session, all-mode weapon audit, minimum-PC profile, full packaged run, or human fun session has been completed. The opening-threat visibility concern remains unresolved.

## Weapon identity slice: 2026-09-27

Added two chrome/cyan emitter prongs to the Phasma first-person view model in `Source/TP_FirstPerson/Variant_Shooter/ShooterCharacter.cpp`, leaving Gale's model unchanged. UE 5.8.3 editor target build succeeded; the headless smoke log `Saved/Logs/IRON-SUN-Ticket21-Phasma-Smoke.log` recorded 22 named passes and zero smoke failures. A fresh Win64 Shipping package built successfully to `Saved/Package-Ticket21-20260927`.

This is a small visual differentiation pass, not a verified A/B playtest: no current packaged screenshot or rendered observation confirms the prongs read at runtime, and Phasma's original firing audio/impact identity remains untested in motion. Do not raise the 18/100 provisional beta-readiness score or close weapon ticket #21 from this build evidence alone.

## Wildcard description slice: 2026-09-27

Wildcard offer copy now describes the generated mode, payload, and condition effects instead of listing only their labels. The editor smoke validates the current offer, then independently generates and checks 100 additional wildcard rolls for finite bounded damage/refire values, positive ammo/pellet counts, reserve at least one magazine, and descriptions matching each roll's traits (including Scatter pellet count). The first assertion exposed an initialization-order bug: copy was composed from default enum values before the random enums were assigned. The generator now assigns the roll first, then builds the description from those stored fields. The editor target rebuilt successfully; `Saved/Logs/IRON-SUN-Ticket22-100Roll-Smoke.log` recorded the 100-roll check passing, 21 smoke pass lines, and zero failure/fatal/assertion-failure matches.

This verifies generated data consistency, not the feel or runtime behavior of 100 sampled weapons in play, and no endpoint frequency/distribution guarantee. Damage/tempo randomness remains intentionally wide. The beta score remains 18/100.

## Responsive sprint slice: 2026-09-27

Sprint now scales from the currently possessed character's walk speed and restores that exact speed on release or pause. Repeated sprint presses do not replace the saved base speed, and possession changes restore the previous pawn before switching. The editor smoke exercises a nonstandard starting speed and pausing while sprint is held. It now fires through the character's bound left mouse button and confirms the muzzle beam appears. The editor target built successfully; `Saved/Logs/IRON-SUN-Ticket03-Sprint-Smoke.log` records 23 named passes and zero smoke failure/fatal/assertion lines.

This was an editor `-NullRHI` smoke with simulated input bindings. It does not verify raw hardware input, controller feel, HUD behavior by sight, or rapid input in a packaged game. Keep control ticket #3 open until the packaged input path is observed.

## Run-state regression slice: 2026-09-27

Added a live-wave smoke assertion that sends three rounds of the bound 1/2/3 weapon-selection inputs and verifies the wave number and remaining-enemy count do not change. The editor target compiled successfully. `Saved/Logs/IRON-SUN-Ticket05-RunState-Smoke.log` records 26 named smoke passes, including repeated selection, wave-one clear/wave-two count, defeat, no automatic respawn, and restart into a fresh armory with a living player and three offers; there were zero smoke failures. The test process remained in its new run after those assertions and was stopped after the 40-second harness timeout, so the timeout is not a game crash. This is editor automation, not packaged repeated-input or HUD verification; ticket #5 stays open for remaining integration checks.

## Win64 Shipping package slice: 2026-09-27

The `TP_FirstPerson` Win64 Shipping target compiled, then `BuildCookRun` completed a full Windows archive at `Saved/Package-Ticket57-Win64-20260927/Windows/TP_FirstPerson.exe`. A direct launch of its Shipping binary rendered the arena and HUD in a visible 1280x720 window on the RTX 3060 test PC. A short controlled input pass changed the view, reduced ammo on firing, and showed incoming damage reducing vitals from 76% to 51%; the process remained responsive. The follow-up pause input could not be delivered after the window lost foreground focus, and weapon swapping/HUD alignment were not confirmed visually. This is not a minimum-PC profile, long session, or side-by-side quality comparison. Keep PC launch/performance ticket #57 open.

## Parallel integration check: 2026-09-27

Two bounded Luna code slices added a close-range sentry strike and modest HUD/Phasma readability changes. The UE 5.8.3 Editor target built successfully after those edits. `Saved/Logs/IRON-SUN-Luna-Integration-Smoke.log` records 26 named passes and zero smoke failures, including baseline sentry movement, incoming/outgoing damage, weapon swap, repeated selection, wave progression, defeat, and restart. This run used `-NullRHI` and simulated bound input; it did not render HUD at target resolutions, observe the new melee strike, test cover navigation, or compare Gale and Phasma in a packaged A/B capture. Issues #21, #25, and #30 remain open. The provisional beta score remains 18/100; human fun is unmeasured.

Run the repeatable headless smoke from PowerShell with `& .\unreal\Ridgefire\Tools\Invoke-BetaSmoke.ps1` after rebuilding the Editor target. The runner saves a timestamped log, requires specific gameplay outcomes including a numeric ammo decrease and the final restart assertion, fails on any smoke failure, fatal error, assertion, early exit, or timeout, and stops only the Editor process it started. It is not a visual or packaged-game test.

The first integrated run of that runner passed 26 checks with zero failures at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-055534.log` after a successful UE 5.8.3 Editor build. This build includes the first explicit wave/optional-objective model, collision-probed sentry detours, wildcard runtime bounds, and a Null/LAN session subsystem. The first online compile caught two outdated session constants; those were corrected before the successful rebuild. None of these slices establishes tested online co-op, a second arena, actual optional-objective rewards, or a natural melee/cover visual playtest. The 18/100 beta score and unmeasured human-fun status remain unchanged.

The next authority/turret integration built after fixing a mistyped turret state field. `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-060605.log` passed the same 26 standalone assertions with zero failures. This version includes first-slice server RPC/replication guards for player combat and an **opt-in** Foundry Turret. The smoke did not spawn that turret or connect a client, so neither feature is a multiplayer or turret playtest. Session review also fixed search-result lifetime/index mapping; host/join remains untested with two clients. Client-visible objectives, replicated enemies, turret weak point, and network-latency tests remain open.

The next replicated-state build passed in UE 5.8.3; `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-061805.log` records 26 standalone smoke passes and zero failures. A separate headless loopback test launched one listen host plus three direct-IP clients. The host log `Saved/Logs/IRON-SUN-Network-Host-20260927.log` records three distinct joins, and `Saved/Logs/IRON-SUN-Network-Client-20260927.log`, `Saved/Logs/IRON-SUN-Network-Client2-20260927.log`, and `Saved/Logs/IRON-SUN-Network-Client3-20260927.log` each record a server welcome. All four exact test processes remained running until stopped. This proves only direct-travel connectivity, not session discovery/invites, unique pawns, replicated combat/objectives, latency behavior, or visual co-op play. The provisional beta score remains 18/100.

Run `& .\unreal\Ridgefire\Tools\Invoke-LANSmoke.ps1 -PlayerCount 2` for a repeatable headless direct-travel check, or use `-PlayerCount 4`; the script accepts a custom port, saves each log, requires host joins and client welcomes, times out on failure, and stops only its launched processes. Its first two-process run passed at `Saved/Logs/IRON-SUN-LAN-20260927-062436-Host.log` and `Saved/Logs/IRON-SUN-LAN-20260927-062436-Client1.log`. It does not call the session subsystem or play/test combat.

## Shipping visual check: 2026-09-27

`Tools/Build-WindowsShipping.ps1` completed a full UE 5.8.3 Shipping build/cook/archive (683 cooked packages) to `Saved/Packages/20260927-063831/Windows/TP_FirstPerson.exe`; the archive is about 0.69 GB. A visible 1280x720 package launch rendered the arena and HUD, captured at `Saved/IRON-SUN-Shipping-Visual-20260927.png`. The screenshot shows very small objective/arsenal text, wildcard-preview text floating between top panels, a small-looking starting gun, and no visible threat ahead while wave 1 still has five sentries and vitals are 24%. The screenshot includes a desktop overlay at the right edge, so it is not a clean presentation capture. It does not identify the damage source or prove sentries failed to move. This is a genuine visual check, not a complete run, input test, resolution matrix, or fun test. Issues #25 and #30 remain open. The provisional beta score is still 18/100 and human fun remains unmeasured.

## Foundry integration check: 2026-09-27

An in-place second preset, Brassfall Foundry, now follows wave one and offers a one-time furnace-core interaction. The current Editor target builds; `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-065513.log` passed 28 named checks and zero failures, including transition and objective registration. This source also contains an opt-in turret weak point and staggered first-attack gating; neither was exercised naturally in the smoke. Brassfall is not a separate map, and the optional core interaction, reward-once behavior, wave-two presentation, first-damage pacing, and two-client arena transition still need live tests. Do not treat this as a finished second arena or raise the 18/100 provisional score yet.

The next Editor build and `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-070206.log` passed 29 named checks with zero failures, including furnace-core out-of-range rejection, one-time reserve reward, and repeat rejection. A new Shipping package built at `Saved/Packages/20260927-070246/Windows/TP_FirstPerson.exe`. Its 1280x720 capture `Saved/IRON-SUN-Shipping-Visual-AfterHUD-20260927.png` visibly improves text size and contains the wildcard preview inside the right panel; the larger Gale view model reads more like a primary weapon, and one sentry is visible ahead with vitals at 92%. This is a single frame from a short session, not proof of sustained pacing, Phasma identity, muzzle-hit alignment, alternate resolution behavior, or fun. The desktop overlay at the right edge still makes it an imperfect presentation capture. Keep #21 and #30 open and the beta score unchanged.

The latest built Editor snapshot also passed `Tools/Invoke-LANSmoke.ps1 -PlayerCount 4 -Port 7793`: `Saved/Logs/IRON-SUN-LAN-20260927-070554-Host.log` records three host joins, and the three adjacent client logs each record a welcome. The harness stopped its four exact processes. This remains a transport-only check, not verified four-player combat or session UI.

## Live roles, session discovery, and compact HUD: 2026-09-27

Later waves now select a bounded composition of existing special NPC roles: wave two contains one Foundry Turret and one Sprinter; wave three adds one Brute. The opening wave remains five baseline sentries. The UE 5.8.3 Editor target built successfully. `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-104745.log` passed 32 named checks with zero failures, including live-wave role counts and server-side Foundry weak-point 3x versus body 1x damage. The first run failed two new assertions because the test counted dead first-wave ragdolls as wave-two enemies; filtering dead actors corrected the test. This is not moving-action or multiplayer enemy evidence.

The Null/LAN session subsystem now has an opt-in host/discover/join test mode. `Tools/Invoke-SessionSmoke.ps1 -Port 7794 -TimeoutSeconds 90` passed with one host and one client: `Saved/Logs/RIDGEFIRE-SESSION-20260927-104820-Host.log` and the adjacent Client log record session discovery, subsystem join request, host acceptance, and client welcome. Integration review corrected a callback-order bug that had attempted to join while search was still marked in flight, and the Editor build caught and corrected an Unreal ticker-handle type mismatch. This does not test a ready/lobby lifecycle, two distinct client pawns, four-player session discovery, replicated combat, or Steam/public matchmaking; #9 and #10 remain open.

The current-source Win64 Shipping package built successfully at `Saved/Packages/20260927-105126/Windows/TP_FirstPerson.exe`. A visible windowed 960x540 launch on the RTX 3060 development PC produced desktop/window captures `Saved/IRON-SUN-Shipping-960-AfterScale-20260927.png` and `Saved/IRON-SUN-Shipping-960-Wave-20260927.png` (window including border: 976x579). The captured wave-one frame shows the HUD, 90% vitals, and a visible nearby sentry; compact HUD labels are larger after a narrow scale adjustment. The captured sentry still has a small hovering-robot silhouette rather than the requested tall armored-machine presence. These are desktop window captures, not native Unreal viewport screenshots; they do not establish precise viewport pixel dimensions, moving-action readability, performance, or fun. The capture through `-ExecCmds="HighResShot 1"` produced no image, so the full resolution matrix is still open. The provisional beta score remains 18/100, with human fun unmeasured.

On the same Shipping package at requested 1280x720, a foreground `P` key paused the live game and a second `P` resumed it. Window captures are `Saved/IRON-SUN-Shipping-1280-Pause-20260927.png` and `Saved/IRON-SUN-Shipping-1280-Resumed-20260927.png`; the paused overlay appeared and the combat HUD returned. This verifies only that keyboard path and visible transition in one short local session, not an extended pause under attack, controller input, restart, network pause behavior, or pixel-accurate viewport layout. The pause instruction text remains visually small.

## Player gameplay capture: 2026-09-27

The player supplied `C:/Users/YaYa/Videos/Captures/Ridgefire - Unreal Editor 2026-09-27 09-08-26.mp4` (103 seconds, 1296x732 recording). Local frame review of this earlier Editor build shows armory entry, weapon firing/kills, score rising to 440, health falling to 91%, and wave-one progress reaching 4/5. From about 60 seconds through the end, one sentry remains while the player searches across the arena, including elevated routes; it is not visibly engaged in the sampled frames. This is evidence of poor last-enemy findability/pacing, not proof the enemy is permanently stuck. The footage also shows the previous small hovering-robot silhouette and sparse combat feedback. It predates the new armored-biped NPC code, which still needs a fresh visual comparison. Contact sheet: `Saved/IRON-SUN-User-Gameplay-ContactSheet-20260927.png`; full-resolution sampled frames are adjacent. Audio and human fun were not scored from this silent frame review.

## Sentry and arena rebuild check: 2026-09-27

Player feedback identified a human ragdoll after sentry death, small non-threatening sentries, broken turrets, a missing physical arsenal, and a cramped arena. The death path was traced to re-enabling the template skeletal mesh and its ragdoll physics; mechanical sentries now keep that mesh hidden, and the standalone smoke checks this state after lethal damage. Foundry Turret spawning is disabled pending redesign. Runtime arena dressing now supplies a 64 m collidable floor and perimeter in both arena phases and hides the original 30 x 35 m static-mesh blockout without changing `Lvl_Shooter.umap`. A three-display arsenal landmark is visible, but its in-world interaction is not wired; selection still uses the existing UI. Issue #58 and arsenal/turret tickets remain open.

The first enlarged-floor smoke exposed a real height mismatch: the old first-PlayerStart estimate placed the new collision floor above the active player. Anchoring floor height to the live pawn fixed the obstruction. `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-114821.log` then passed with no failures, including death mesh, firing, damage, wave/arena transition, game-over, and restart. A two-process NullRHI combat fixture passed at `Saved/Logs/RIDGEFIRE-COOP-20260927-114856-{Host,Client}.log`; it is not natural multiplayer play.

Shipping packages `Saved/Packages/20260927-114915/Windows/TP_FirstPerson.exe` and `Saved/Packages/20260927-115215/Windows/TP_FirstPerson.exe` built successfully. Desktop captures at `Saved/IRON-SUN-Expanded-Wave-20260927.png` and `Saved/IRON-SUN-Large-Sentry-20260927.png` show the old enclosing walls gone, a larger, darker armored biped, and the arsenal landmark. These are static views of a bordered 1280x720 window, not a traversal, enemy-death, performance, or human-fun playtest. The arena is now visually sparse, the station still reads like blockout geometry, and biped animation/intimidation remain below beta art quality. The provisional 18/100 beta score remains unchanged.

Final integration raised the enlarged NPC capsule clear of the new collision floor. UE 5.8.3 Editor build and `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-115902.log` passed with zero smoke failures. A two-process NullRHI combat fixture passed at `Saved/Logs/RIDGEFIRE-COOP-20260927-115844-{Host,Client}.log` after the spawn fix; this is still a synthetic target-placement test, not visual co-op play. The exact final source was packaged successfully at `Saved/Packages/20260927-115949/Windows/TP_FirstPerson.exe`. The final package itself has not yet been visually captured; the cited captures are from the immediately preceding builds.

## Arena replication slice: 2026-09-27

`ARidgefireArenaDressing` now replicates its arena center, floor height, and arena variant; each client reconstructs the visual and collision components from that state. The actor is always relevant so a late-joining client receives the active arena state, and the server destroys the previous dressing actor during the Brassfall transition so its collision is removed on clients as well. Generated collision components receive stable network names so client movement-base references resolve without unsupported-component warnings. Added `RIDGEFIRE ARENA STATE` logs and host/client parity assertions to `Tools/Invoke-CoopCombatSmoke.ps1` and `Tools/Invoke-LANSmoke.ps1`. The checks compare arena kind, build revision, component count, center, and floor height.

UE 5.8.3 Editor target build succeeded. `Saved/Logs/IRON-SUN-LAN-20260927-160127-{Host,Client1,Client2,Client3}.log` passed with three clients matching the host’s Iron Sun state: 64 generated components, center `(-800,-1050,98.46)`, ground `2.5`. `Saved/Logs/RIDGEFIRE-COOP-20260927-160149-{Host,Client}.log` passed remote fire/damage/score replication and matching host/client state through Iron Sun and the Brassfall transition; neither log contained unsupported arena-component references. `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-160210.log` passed the required firing/ammo, player/enemy health, wave, arena transition, defeat, and restart checks with zero failures. These were NullRHI/editor process tests, not rendered client play, traversal, late join after Brassfall, or restart visuals. Issue #59 remains open pending those checks. The new video capture was not reviewed in this run: the local-file browser action was blocked and I cannot switch to a media player to bypass that restriction. Provisional readiness remains 18/100; human fun remains unmeasured.

The combat HUD panels now use lower opacity and a scale tied to viewport dimensions; layout calculations were checked at 960x540, 1280x720, 1920x1080, 2560x1080, and 720x1280. This was source/layout-math review only; rendered text readability, raw input, and actual placement still need visual confirmation. Keep issue #30 open.

## Co-op pursuit targeting slice: 2026-09-27

Sentries now choose a living possessed pawn from all player controllers instead of always targeting only the first controller. They retain their current target unless another eligible target is meaningfully closer; a failed recovery path temporarily deprioritizes that target when another teammate is available, while a lone player remains the fallback. Target changes stop stale path recovery. The authority-only editor build succeeded with `Build.bat TP_FirstPersonEditor Win64 Development -Project=...Ridgefire.uproject -WaitMutex -NoHotReloadFromIDE`.

`Tools/Invoke-BetaSmoke.ps1 -TimeoutSeconds 180` passed firing/ammo, naturally positioned enemy damage, wave transition, arena transition, defeat, and restart checks at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-161156.log`. `Tools/Invoke-CoopCombatSmoke.ps1 -Port 7822 -TimeoutSeconds 120` passed distinct host/client pawns, remote aim, authoritative hit/damage/reward, health/ammo/score replication, and arena state parity at `Saved/Logs/RIDGEFIRE-COOP-20260927-161251-{Host,Client}.log`. These fixtures do **not** assert sentry target switching when the first player is down/disconnected, force a navigation failure, or visually play a packaged pursuit. Keep issue #25 open until those cases are tested in a rendered multiplayer/package session.

## Per-player primary loadout slice: 2026-09-27

Primary slots, active weapon, and cached ammo now belong to each server-side player-controller loadout; validated selection and cycling equip only that controller's pawn. The per-controller run-state feed carries that player's weapon slots/active offer for its HUD. Editor co-op automation now chooses different two-primary loadouts for host and remote, swaps both players, and checks that remote firing consumes only the remote player's ammo. The parent weapon ticket #20 remains open: personal/shared mode-specific claiming, secondary and blade slots, and packaged/UI verification are still incomplete.

Lead verification: UE 5.8.3 editor build target is up to date and succeeded. `Tools/Invoke-CoopCombatSmoke.ps1 -Port 7823 -TimeoutSeconds 120` passed independent loadout/swap assertions plus remote authoritative firing and reward; host log `Saved/Logs/RIDGEFIRE-COOP-20260927-164453-Host.log` records different host/remote weapons and isolated ammo. `Tools/Invoke-BetaSmoke.ps1 -TimeoutSeconds 180` passed with zero smoke failures at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-164515.log`. These are headless editor-process tests, not visual packaged co-op play. The two-player smoke drives server-side selection directly, so it does not validate whether each client sees a separate opening-armory screen.

## Team opening-armory flow slice: 2026-09-27

The opening armory remains open while any connected player lacks a starting primary. A player's offer equips only their pawn; when every current player has a starting primary, the party armory closes and the first wave timer starts. The timeout now supplies a default primary to each connected player still unselected, rather than only the first controller. The co-op smoke exercises host and remote choosing different opening offers, asserts the remote still receives an open-armory run state after the host chooses, then checks the armory closes after both choices and proceeds to wave one. UE 5.8.3 editor build succeeded. `Tools/Invoke-CoopCombatSmoke.ps1 -Port 7824 -TimeoutSeconds 120` passed; `Saved/Logs/RIDGEFIRE-COOP-20260927-165126-Host.log` records the new armory assertion, two independent starting offers, and wave one. `Tools/Invoke-BetaSmoke.ps1 -TimeoutSeconds 180` passed with zero failures at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-165154.log`. This remains headless editor automation, not visual packaged co-op, timeout-path, or human testing. Parent issue #20 remains open for progression one-copy claims, secondary/blade loadouts, station interaction, and packaged UI verification.

## Sentry target eligibility smoke slice: 2026-09-27

The live server AI and co-op smoke now call the same deterministic target selector. The smoke marks the current host target unavailable and verifies handoff to the living remote player, then verifies the sole eligible player remains available as fallback even while temporarily path-deprioritized. The UE 5.8.3 Editor target rebuilt successfully after correcting the smoke fixture's candidate array type. `Tools/Invoke-CoopCombatSmoke.ps1 -Port 7827 -TimeoutSeconds 120` passed; `Saved/Logs/RIDGEFIRE-COOP-20260927-171410-Host.log` contains both named target-selection pass markers. `Tools/Invoke-BetaSmoke.ps1 -TimeoutSeconds 180` passed with zero failures at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-171431.log`.

This exercises selector logic headlessly, not a real death/disconnect, navigation failure, moving sentry, readable attack, or rendered co-op encounter. Issue #25 remains open for pathing/attack and packaged visual acceptance.

## Compact HUD control-hint slice: 2026-09-27

At viewports narrower than 1000 pixels, keyboard and gamepad guidance now wraps into three centered lines rather than shrinking all instructions into one long line. All current control help remains present, including primary replacement, wildcard reroll, reload, patch, surge, and pause. The rows are placed above the ammo panel at 960x540; wider screens retain the prior single-line placement.

The UE 5.8.3 `TP_FirstPersonEditor` target rebuilt successfully after this change. `Tools/Invoke-BetaSmoke.ps1 -TimeoutSeconds 180` passed firing/ammo, health, wave, arena-transition, defeat, and restart checks with zero failures at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-165642.log`. `Tools/Invoke-CoopCombatSmoke.ps1 -Port 7825 -TimeoutSeconds 120` passed host/client arena parity through Brassfall plus remote authoritative hit/damage and replicated health/ammo/score at `Saved/Logs/RIDGEFIRE-COOP-20260927-165750-{Host,Client}.log`. These headless checks do not render HUD text. No 960x540, 1280x720, ultrawide, or portrait capture was made, so readability and non-overlap still need visual resolution-matrix verification; issue #30 remains open.

## Per-player secondary-offer ownership slice: 2026-09-27

The secondary offer index now belongs to each server-side player loadout and is copied into that player's replicated run state. Server selection rejects invalid offer indices and offers already assigned to that player's primary slots. Co-op editor automation selects distinct secondary offers for host and remote, verifies the first selection leaves the other player's state empty, and confirms invalid and primary-owned selections do not replace either valid choice.

Lead verification: UE 5.8.3 Editor target rebuilt successfully. `Tools/Invoke-CoopCombatSmoke.ps1 -Port 7826 -TimeoutSeconds 120` passed; `Saved/Logs/RIDGEFIRE-COOP-20260927-170505-Host.log` records independent secondary state and rejection assertions. `Tools/Invoke-BetaSmoke.ps1 -TimeoutSeconds 180` passed with zero failures at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-170527.log`. This is state/offer ownership only: no secondary weapon is equipped or fired, no selection UI/input is exposed, and progression-mode shared one-copy rules are not implemented. Issue #20 remains open.

## Deterministic wildcard-property smoke: 2026-09-27

The 100-generated-roll property check now uses a dedicated `FRandomStream` seed (`0x51A7`, decimal 20903), leaving production randomness unchanged. Each roll checks finite damage/refire values, allowed magazine/reserve/pellet values, and the mode, payload, and condition description. The smoke restores global offers, ammo, player loadouts, run states, active slot, and reroll state afterward. During review, this check also exposed that the effect description was composed before assigning the final rolled enum values; the description is now built from the stored roll. A later audit found Siphon's generated description understated its discrete kill-heal values; it now documents the extra heal roll (2, 8, 20, or 45), with an editor-smoke assertion.

Lead re-verification: UE 5.8.3 `TP_FirstPersonEditor` build succeeded and `Tools/Invoke-BetaSmoke.ps1 -TimeoutSeconds 180` passed with zero failures at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-193600.log`; the log records the Siphon description assertion and `RIDGEFIRE SMOKE PASS: deterministic wildcard property smoke seed=20903 validated 100 rolls`. This proves reproducible generated-data/property validation, not that every combination was equipped/fired or that wildcard balance is fun. Keep issue #22 open for those execution and gameplay requirements.

## Phasma co-op audio spatialization slice: 2026-09-27

Phasma shot effects still use the existing procedural descending tone, impact, and ringing tail, but the replicated shot now passes its muzzle location into playback. A transient character-owned attenuation setting enables spatialization with logarithmic falloff to 2200 cm, replacing flat `PlaySound2D` playback for the co-op multicast.

The UE 5.8.3 Editor target compiled successfully, and the two-process NullRHI co-op smoke passed at `Saved/Logs/RIDGEFIRE-COOP-20260927-173120-{Host,Client}.log`. This smoke did not play Phasma or emit audio (`-NoSound`), so spatial perception, sound mix, and the original intended metallic character still require a packaged, human-ear A/B check. Issue #21 remains open.

## Portrait combat HUD layout slice: 2026-09-27

Combat panels now stack at portrait aspect ratios, using the available width rather than remaining side-by-side and shrinking text. Source-level layout arithmetic keeps the panel stack well above the compact controls at 540x960 and 720x1280; 960x540 and 1280x720 retain the existing two-column arrangement. The integrated Editor target compiled successfully and its beta smoke passed at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-173715.log`, but that headless run does not render HUD text. No rendered capture or input-navigation test was performed. Issue #30 stays open until the requested packaged resolution matrix confirms legibility and menu/route behavior.

## Robot-only sentry death replication slice: 2026-09-27

Sentry-role RepNotify handlers now reapply the mechanical death presentation when a role flag arrives after the replicated dead flag, hiding the inherited skeletal mesh/ragdoll, disabling its physics/collision, and showing the existing robot body. An editor-only assertion logs named pass/fail evidence when a mechanical NPC dies. Actor scale was deliberately left unchanged because hitbox and traversal safety cannot be inferred from this slice.

UE 5.8.3 Editor target build succeeded. `Tools/Invoke-BetaSmoke.ps1 -TimeoutSeconds 180` passed with zero failures at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-173715.log`; it records five mechanical-death assertion passes. `Tools/Invoke-CoopCombatSmoke.ps1 -Port 7829 -TimeoutSeconds 120` passed at `Saved/Logs/RIDGEFIRE-COOP-20260927-173807-{Host,Client}.log`; the host log includes the death assertion. These tests validate source state in headless processes, not out-of-order replicated arrival on a rendered remote client. The sentinel silhouette, map footprint, physical arsenal, and packaged visual death remain unverified; issue #58 stays open.

## Brassfall entry reserve refill slice: 2026-09-27

The first successful Iron Sun → Brassfall transition grants each player up to 10% of each carried primary's configured reserve capacity, capped at that weapon's offer reserve. The one-time guard prevents repeated transitions from granting it again; survival wave changes do not call this refill. The Editor smoke temporarily gives each player partial reserves, verifies the player-specific refill and cap, repeats the transition to check no duplicate grant, then restores the prior loadout/ammo.

UE 5.8.3 Editor build succeeded. `Tools/Invoke-BetaSmoke.ps1 -TimeoutSeconds 180` passed with zero failures at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-174938.log`, recording the refill assertion for one player. `Tools/Invoke-CoopCombatSmoke.ps1 -Port 7830 -TimeoutSeconds 120` passed at `Saved/Logs/RIDGEFIRE-COOP-20260927-175027-{Host,Client}.log`, verifying separate host/remote refill amounts and no duplicate transition. This is not a recoverable carried resupply object, arena/species-specific delivery, optional objective penalty, survival economy, or empty-ammo package test. Issue #23 remains open.

## Sentry telegraph audio source slice: 2026-09-27

The first source implementation adds a short original double-chirp warning at successful ranged/melee telegraph starts, sent from authority by reliable multicast and played at the sentry location with spatialization and 1600 cm attenuation. Five attack-start branches dispatch once after the visual telegraph succeeds.

The integrated UE 5.8.3 Editor build succeeded. `Tools/Invoke-CoopCombatSmoke.ps1 -Port 7840 -TimeoutSeconds 120` passed at `Saved/Logs/RIDGEFIRE-COOP-20260927-184507-{Host,Client}.log`; the host records exactly one living-target sentry-warning cue dispatch and explicitly labels it as RPC-dispatch-only. The same co-op smoke verifies separate Brassfall reserve-refill amounts for both connected players and zero refill on a duplicate transition. `Tools/Invoke-BetaSmoke.ps1 -TimeoutSeconds 180` passed with zero failures at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-184546.log`, including fixed-seed wildcard property checks, firing/ammo, damage, mechanical death, arena transition, defeat, and restart. Both smoke paths use NullRHI/`-NoSound`; client RPC arrival, audio playback, spatial direction, mix, falloff, packaged behavior, and human-ear review remain unverified. Issue #31 remains open.

## Secondary weapon toggle slice: 2026-09-27

The player can select a secondary offer, toggle it with keyboard `B` or gamepad `Y`, and equip it through the existing active-weapon path; gamepad reload moves from `Y` to right-stick click (`R3`) to avoid conflict. The active weapon display derives its slot/name from the equipped roll. The server owns the toggle and each slot's magazine/reserve values are cached separately. The physical Iron Sun station now opens a per-player secondary selection panel within 500 cm using keyboard `T` or gamepad View; while open, existing 1/2/3 and A/B/X offer inputs select a secondary. Range, host/remote isolation, invalid or primary-owned offers, panel close, and unchanged global starting-armory state have co-op smoke assertions. `Tools/Invoke-CoopCombatSmoke.ps1 -Port 7841 -TimeoutSeconds 120` passed in `Saved/Logs/RIDGEFIRE-COOP-20260927-190736-{Host,Client}.log`; the remote player's Gale secondary consumed one round through normal firing, damaged a living sentry, and retained its spent magazine/reserve after switching. The headless source test does not prove rendered HUD/input behavior, controller hardware, reload/full-run ammo persistence, or the complete three-offer primary-replacement/blade rules. Issue #20 remains open.

## First-arena armored sentry visual slice: 2026-09-27

The first-arena baseline sentry now has broader layered chrome shoulder armor and a taller chrome helm with an amber crown when its arena state resolves as `IRON SUN ARENA`. Other arena names retain their existing visual profile. The editor-only assertion requires 12 new attached, non-colliding armor components and measured visual bounds above 245 cm tall and 80 cm half-width; capsule, navigation, movement, and attack behavior are unchanged.

Lead verification: UE 5.8.3 Editor build succeeded; `Tools/Invoke-CoopCombatSmoke.ps1 -Port 7841 -TimeoutSeconds 120` passed with the per-player station/range assertions and the armored-sentry bounds assertion in `Saved/Logs/RIDGEFIRE-COOP-20260927-190736-Host.log`; `Tools/Invoke-BetaSmoke.ps1 -TimeoutSeconds 180` passed at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-190757.log`. These NullRHI runs do not show the silhouette, prove remote rendered parity, or judge readability/hittability in combat.

The later read-only UE 5.8.3 bounds query measured the `Lvl_Shooter` NavMeshBoundsVolume at origin `(0, 0, 250)` cm, extent `(1468.261, 1674.748, 341.372)` cm (bounds X ±1468.261, Y ±1674.748, Z −91.372…591.372). The Recast actor's serialized bounds were origin `(0, 0, 208.628)` cm, extent `(1976, 1976, 200)` cm; these are actor/component bounds, not proof of generated navigation-tile coverage. A headless navigation projection found no sample points and logged a Recast serialization `maxTiles` mismatch / missing RecastNavMesh instance, so actual tile coverage remains unknown. No arena expansion was made. Inspect/rebuild navigation in the Editor and confirm safe traversable lanes before extending the 6400 × 6400 cm floor and ±3100 cm boundaries.

`Tools/Invoke-CoopCombatSmoke.ps1 -Port 7843 -TimeoutSeconds 120` later passed the existing co-op suite and a restoration-safe editor assertion that forces the sentry telegraph's missing-project-material fallback; the host log `Saved/Logs/RIDGEFIRE-COOP-20260927-192655-Host.log` records `default-material fallback showed telegraph without triggering caller cancellation`, with no smoke failure/fatal/assertion markers in host or client logs. This verifies the fallback path in headless Editor automation, not a visible attack tell in PIE or packaged play. The station signs/layout, controller input, arena footprint, turret redesign, and packaged visual play remain outstanding; issue #58 stays open. User direction is distinct sentry designs by arena, starting with tall armored giants.

## Replicated one-shot field drops: 2026-09-27

`ARidgefirePickup` now replicates its actor and health/ammo type. Every instance applies the default ammo tint on `BeginPlay`, and type updates refresh the light. Collection is authority-only and one-shot: a collected drop hides, stops colliding, and expires shortly after its reward. The existing beta smoke checks that a second collection cannot grant another reward. The co-op smoke forces a drop from its test kill so client actor receipt is observable.

Lead verification: the UE 5.8.3 `TP_FirstPersonEditor` build succeeded; `Tools/Invoke-BetaSmoke.ps1 -TimeoutSeconds 180` passed with zero failures at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-195031.log`, including `replicated sentinel field drop granted once; repeated collection rejected`. `Tools/Invoke-CoopCombatSmoke.ps1 -Port 7844 -TimeoutSeconds 120` passed at `Saved/Logs/RIDGEFIRE-COOP-20260927-195104-{Host,Client}.log`; the client log records receipt of a replicated ammo field-drop actor. NullRHI proves actor receipt and source state, not visible tint, pickup readability, or packaged co-op. Species-specific carried resupply, downed-carrier recovery, and survival/progression ammo economies remain outstanding; issue #23 stays open.

## Initial blockout and game-over input slices: 2026-09-27

The replicated arena dressing now suppresses legacy static-mesh blockout before building either Iron Sun or Brassfall geometry, rather than only at the Brassfall transition. The player controller no longer quits the application when pause/back is pressed after defeat; on both authoritative and replicated run-over state it stops sprint/fire, unpauses, and exposes the cursor. The beta smoke now requires its game-over pause assertion, and the co-op smoke requires nonzero initial blockout suppression on host and client before their Iron Sun arena-state logs.

Lead verification: UE 5.8.3 `TP_FirstPersonEditor Win64 Development` build succeeded. `Tools/Invoke-BetaSmoke.ps1 -TimeoutSeconds 180` passed with zero failures at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-201331.log`, including the game-over pause/back assertion. `Tools/Invoke-CoopCombatSmoke.ps1 -Port 7846 -TimeoutSeconds 120` passed at `Saved/Logs/RIDGEFIRE-COOP-20260927-201408-{Host,Client}.log`; host and client each logged 98 legacy meshes suppressed before constructing 64 initial-arena components and agreed on the Brassfall transition state. This is NullRHI state evidence only. It does not prove rendered collision parity, late join, four clients, packaged traversal/firing, remote-client defeat UI, or actual menu navigation. Issues #5 and #59 remain open.

## Next beta work

Target-switch safety slice: a sentry now cancels its active melee/ranged warning and queued Foundry/Brute burst when it changes player targets, hides the stale beam, and restarts attack timing so an old warning cannot carry onto the new target. UE 5.8.3 `TP_FirstPersonEditor Win64 Development` built successfully; `Tools/Invoke-BetaSmoke.ps1 -TimeoutSeconds 180` passed with zero failures at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-215349.log`. Lead independently ran `Tools/Invoke-CoopCombatSmoke.ps1 -Port 7850 -TimeoutSeconds 120`, which passed at `Saved/Logs/RIDGEFIRE-COOP-20260927-215551-{Host,Client}.log`. Neither smoke forces rapid target switching or renders the telegraph; issue #25 remains open for targeted two-player and packaged pursuit checks. A separate HUD layout review found no source-level overlap at the measured 960x540, 1280x720, 1920x1080, 640x480, or 720x1280 layouts, made no code change, and leaves issue #30 open pending rendered resolution/navigation tests.

A pre-publish repeat of the beta smoke failed once at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-220055.log`: the observed natural sentry moved 1013 cm and began telegraphing but delivered no hit to the player in 12 seconds. The immediate retry passed with zero failures at `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-220228.log`. Treat this as an intermittent pursuit/attack or test-fixture gap, not a clean signoff; issue #25 remains open.

The 2026-09-27 UE 5.8.3 Editor build and `Tools/Invoke-BetaSmoke.ps1 -TimeoutSeconds 180` passed after adding the natural-pursuit assertion. The log `Saved/Logs/IRON-SUN-Beta-Smoke-20260927-214548.log` reports a wave-one sentry moving 1149 cm toward a reachable player, telegraphing, attacking, and reducing player health over 6.0 seconds; the smoke suite reported zero failures. This remains NullRHI simulation evidence, not rendered proof of animation, silhouette, readable tells, or satisfying combat.

The first Blender MCP blockout was rejected as too stylized and was not imported into Unreal. The player approved the realistic Standard, Sprinter, Brute, and rebuilt Turret visual directions in `SourceArt/Concepts` as the first-arena set, with more roles to be added later. These are 2D concept mockups, not coherent production meshes. The turret game model must have exactly three barrels even where the concept views are ambiguous. Blender MCP was verified live with Blender 5.2.2 LTS; the installed Blender 3.0.0 cannot parse the current add-on.

The optional `Tools/Invoke-CoopCombatSmoke.ps1 -Port 7847 -TimeoutSeconds 120 -VerifyLateJoin` check passed on 2026-09-27. After the first client and host reached Brassfall, a third NullRHI client joined, suppressed 98 legacy meshes, and reconstructed the same revision-1 Brassfall state with 47 components, center, and ground as the host. Logs: `Saved/Logs/RIDGEFIRE-COOP-20260927-204737-{Host,Client,LateClient}.log`. This is headless late-join state parity only; rendered collision, four clients, packaged traversal/firing, and restart parity remain unverified. Issue #59 stays open.

Priority is a PIE/packaged visual test of giant sentry scale, pursuit, hit registration, robot-only death, arena traversal, and the physical arsenal at multiple resolutions. Inspect nav bounds in Editor before enlarging the arena; rebuild the removed Foundry Turret role only after its shape and attack tell are redesigned and visually tested. Then test Brassfall's core interaction and in-place transition in a packaged run, and exercise real two-player weapon selection/damage/revive paths. Run the HUD resolution matrix and a controlled first-damage/Phasma A/B capture. Issue #7 still needs an agreed minimum PC before issue #57 can claim smooth quality-preserving performance. Do not raise the readiness score or close issues merely because a headless build/smoke passes. The broader backlog still includes co-op lifecycle, revives, perks, progression routes, and the remaining distinct arenas.
