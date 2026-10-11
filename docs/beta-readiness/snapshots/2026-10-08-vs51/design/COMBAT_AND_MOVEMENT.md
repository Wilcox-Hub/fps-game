# Pressure combat and momentum movement

Player approved this direction October 8, 2026. This supersedes the deferred attack-pattern decision in PACING_AND_WEAPON_PROGRESSION.md. Initial numeric tuning remains provisional until normal solo and co-op playtesting.

## Experience

IRON SUN creates pressure-inducing, dangerous fights through exposed positions, complementary threats and coordinated angles. Speed alone is not the goal. Players establish temporary control and deliberately move between cover. A single ordinary enemy should allow several seconds to react; concentrated fire makes exposure dangerous. These are human balance targets, not measured survival guarantees.

Mostly short bursts, longer pressure sequences and slower charged attacks provide variety. Roles inform firing habits without rigidly scripting every fight. Limited aim correction lets well-timed movement escape committed fire. Smarter bots track better and use more strategic routes. Arena and run variation change which types receive that behavior. Flanking remains limited and should become readable through visible movement and mechanical footsteps and existing enemy warning sounds, not arbitrary center spawns.

## Shooting

Gun families have learnable upward recoil with modest lateral variation. Individual gun power and cadence affect kick; heavy shots and especially high fire rates require more control. Sustained fire builds recoil. Shot and target-impact audio, muzzle/viewmodel movement, visible enemy hit reactions and mechanical collapse improve feedback. Ordinary hits cause small visual reactions. Heavy or weak-point hits may interrupt ordinary enemies, with a refractory window; brutes and champions resist interruption.

The Helix's three energy pulses are timed separately, trace from the current aim and produce separate feedback. One burst consumes one charge, preserving the existing per-trigger economy/stat model. Switching/reloading cancels pending pulses. It is not three ordinary magazine rounds.

Aim down sights uses hold RMB (gamepad LT), modest zoom and a centered prototype viewmodel pose. Normal aiming tightens accuracy and slows movement. Authored optics/weapon animations remain production-art work.

## Slide input and movement

Hold C or Ctrl (gamepad right-stick press) while moving with sprint momentum to slide. The slide spends existing velocity; it supplies no launch impulse, invulnerability, new stamina meter or fixed ability cooldown. Small steering adjustments are possible. After momentum is spent, continuing to hold keeps crouch. Releasing at any time requests standing, subject to capsule clearance. Sprint stands the player when room permits and clears crouch intent. Normal crouch is hold-based, never a toggle.

Both hipfire and sight aiming work during slides with reduced accuracy. Hipfire is more accurate and practical than sliding sight aim. Stationary crouch bracing is not applied to a moving slide. Repeating requires rebuilding speed and a brief handling recovery. Dead, downed/carrying, menu and run-over states reject the new movement. Airborne crouch does not start a slide.

Sprint/crouch/aim intent travels with Unreal character movement packets and saved moves. Slide state and recovery are restored during client correction replay. Native walking handles floor and wall sweeps; standing still respects headroom. Keyboard R reloads; gamepad D-pad up reloads, replacing the old right-stick reload binding to make room for hold crouch.

Roll/dash is deliberately deferred until sliding is tested. Lean, larger maps, entrance gates and a full HUD redesign are outside this pass.

## Initial tuning and validation limits

Slide entry requires ground contact and at least 1.28 times configured walk speed. Initial deceleration is 650 cm/s², steering at most 35°/s, end below 0.48 times walk speed, and short recovery 0.22 s. ADS zoom reduces FOV by 12° and normal speed to 75%. Slide adds a 2.6° hipfire cone penalty or 3.8° sight-aim penalty. These values need human feel, slope/steps/ceiling and network-latency testing.

Ordinary pressure profiles start with a 70% burst preference; burst lengths and rest periods vary. Charged and longer sequences make up the remainder. Heavy/weak-point ordinary interruptions last 0.3 s and cannot repeat within 2 s. Existing one-to-four-player encounter/health/damage scaling remains; this pass does not claim new measured balance or 45–60-minute run pacing.

Consult docs/beta-readiness/latest.json at repository root for the actual build, test scopes and pending acceptance. Automated checks do not establish fun or beta readiness.
