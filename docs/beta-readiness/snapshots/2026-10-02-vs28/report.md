# VS28 roles, bracing and rescue — October 2, 2026

Retain the previous **44% gameplay planning estimate**, low confidence, NOT_BETA_READY. Overall beta completion remains unmeasured; the 50% target is still open. These focused changes receive no new score credit before human/hardware/network-feel acceptance.

## Player-facing changes

Three perk groups: Combat, Medic and Movement. There is no separate Support group. Two slots allow focused or mixed roles; existing XP thresholds and saved IDs remain valid.

All players can hold C/Left Ctrl or gamepad LT to crouch and brace, reducing weapon recoil by 35% without a perk. Crouch lowers the first-person view and standing restores it. Stable Grip replaces the crouch-only armor perk: post-sprint/hard-landing recoil settling takes 0.65 seconds instead of one second. It adds no aim assist or hidden bullet spread. Owner-only recoil is applied after the authoritative shot.

Rescue Harness is a Medic perk. It preserves the primary weapon at pickup, permits primary firing/reloading and B/Y primary-secondary toggling while carrying, and keeps the normal 65% carry speed and no-sprint restriction. Players without it still need the secondary. Pickup stands the carrier before calculating carry speed; a blocked standing capsule cannot begin a carry. The existing 60 earned-second healing-tube requirement is retained.

See [progression guide](progression-guide.md) for the full ten-perk roster.

## Current evidence

Editor build; three unlock/save/store tests; actual possessed profile/keyboard/gamepad controls; universal crouch/camera restoration and faster recovery; actual downed-player pickup; primary trace damage/ammo/reload completion/toggling; normal secondary restrictions; generated combat; authoritative fire; local two-process co-op/restart; and full healing-tube recovery fixtures passed. Rendered fixtures produced five frames at each of 1280x720 and 800x320, including crouched gameplay. Reload completion inside the dedicated carry fixture calls the real callback directly; separate recovery/co-op fixtures cover real timers.

The Shipping prototype built and survived an offscreen D3D12 startup observation. This does not establish normal-input packaged play. Standing generated-combat/fire/co-op checks were completed for VS28 before the final crouch attachment refresh; their unchanged standing paths are reused, with no claim of network crouch validation. Current rendered crouch and healing-tube checks cover the final fix. The crouch images include a transient editor shader-warmup overlay. [Test results](test-results.json), [excerpts](test-excerpts.txt) and exact Source/Config/Content hashes preserve the scope. The source includes unpublished local work. Earlier VS27 snapshots remain history and do not independently match this source.

## Remaining gates

Physical keyboard/controller and normal-input Shipping full runs; human tuning of recoil and perk balance; crouched third-person animation/production art acceptance; internet/Steam and four-player lifecycle/reclaim; ten finished arenas, routes and champions; complete staged lobby; minimum-PC profiling and longer soaks. Existing connectivity/asset-pipeline evidence is preserved in VS27 and receives no gameplay readiness credit.
