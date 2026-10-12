# Play IRON SUN on Windows

In the source checkout, double-click `Play-IRON-SUN.bat` in this folder to run the newest completed local Windows Shipping prototype. It does not build the game or install software. If no complete archive exists, follow the developer build step below. Rebuild after source/content edits; an older complete package may not contain them.

1. Download or copy the **entire** packaged `Windows` folder, then unzip it before playing. Do not run the executable from inside a ZIP.
2. Open `Windows/TP_FirstPerson.exe`. Keep the adjacent `Ridgefire/` and `Engine/` folders beside it; do not copy just the executable.
3. If Windows reports a missing Visual C++ runtime, run the included `UEPrereqSetup_x64.exe` if it is present in `Windows/Engine/Extras/Redist/en-us/`, then retry. If the installer was not staged, install the current Microsoft Visual C++ x64 redistributable from Microsoft. No Unreal Editor or Visual Studio is needed to play a correctly packaged build.

The current package is a Shipping **prototype**, not a signed-off beta or performance-certified release. The agreed minimum PC and frame-time target are pending in GitHub issue #7. If it fails to start, record the Windows version, CPU, GPU, RAM, graphics driver version, and any error text. Do not lower resolution or visual settings to hide a performance problem without an explicit player choice and visual review.

For developers, run `& .\unreal\Ridgefire\Tools\Build-WindowsShipping.ps1` from the repository root after installing UE 5.8.3 and the Windows C++ toolchain. Each run writes a timestamped, self-contained archive under `unreal/Ridgefire/Saved/Packages/`. The script builds, cooks, stages, packages, checks the executables and cooked containers, then writes a complete-package marker; it does not publish or sign a release. Use `Tools/Play-Latest-WindowsPackage.ps1 -ListOnly` to inspect the selected archive without launching it.

## Prototype controls and rescue

Use WASD/mouse to move/look, left mouse to fire, Shift to sprint, R to reload, and P/Escape to pause. Opening setup chooses your secondary first and then your primary. You carry two guns: 1 equips the primary and 2 equips the secondary. New purchases and confirmed finds replace the chosen gun; J switches the arsenal destination between primary and secondary. T opens a nearby arsenal; Left/Right browse, Enter confirms purchases/upgrades, Tab shows exact stats. B also toggles the secondary. F uses a patch; V is melee (M fallback). Hold Q/E to lean left/right; Settings offers toggle lean. Z fires the equipped Ion Blaster's charged attack. Ion is bought at the arsenal and charges from direct normal-shot hits.

In connected co-op, select a secondary before carrying. Contextual T/gamepad VIEW picks up a nearby downed teammate, sets them down away from a tube, or deposits them at a nearby empty team tube, prioritizing rescue before arsenal actions. Carrying is slower and uses the secondary by default; Rescue Harness permits primary fire and weapon switching. Defend the tube for 60 unpaused seconds; an enemy hit pauses it for five seconds without deleting progress. Clear a blocked exit when prompted. Recovery keeps the original pawn, weapons and ammo, with provisionally configured 50% health. The current tube, poses, sentries and arena are still placeholder art; online matchmaking and full beta acceptance are unfinished.

`BETA_PLAYTEST_LOOP.md` records which exact builds and paths were checked. A successful package/launcher check does not mean this Shipping archive was played; current rescue visuals were inspected in controlled offscreen Editor fixtures, not a hardware-input or Shipping playtest.


## LAN discovery troubleshooting

See SESSION_FLOW.md for host/find/ready controls. If a party never appears between PCs, use Tools/Configure-LANFirewall.ps1 to preview a game-only Private-network exception; -Apply needs administrator permission. The actual network executable is inside Windows/Ridgefire/Binaries/Win64. No Windows prompt may appear when listen notifications are off. The original October 5 VS47 package is unchanged; two-PC LAN still needs acceptance.


## Automatic update launcher (VS50)

Use the new IRON SUN desktop shortcut for the per-user fixed installation and approved-feed checks. See [LAN_UPDATES.md](LAN_UPDATES.md) for one-time second-PC setup, authenticated home share, future tested-build publishing and rollback. The old full game ZIP remains valid; no game-code update was made. Network activation/second-PC acceptance are recorded separately in latest.json.


## Pressure combat and movement (VS51, October 8, 2026)

Use the IRON SUN desktop shortcut for the fixed updater install. The new package/feed version is recorded in repository-root docs/beta-readiness/latest.json; old packages remain historical builds.

Hold RMB to aim (modest zoom, tighter accuracy and slower movement). Hold C or Ctrl with sprint momentum to slide; hold to finish crouched, release to stand when clear. Shift stands and sprints. Sliding permits both firing styles with reduced accuracy; hipfire is more practical than aiming during a slide. Build momentum again before repeating. There is no dodge/roll in this pass.

Gamepad: LT aims; hold R3 to crouch/slide; D-pad up reloads. L3 sprints. This replaces the old LT-crouch/R3-reload mapping; hardware/controller acceptance remains pending.

This pass adds pressure profiles/strategic variation, stronger gun-family recoil, timed Helix pulses and prototype shot/impact/footstep/hit/death feedback. Read [COMBAT_AND_MOVEMENT.md](COMBAT_AND_MOVEMENT.md). Rendered inspection and automated tests are separate from normal feel/fun, authored optics/animations and two-PC co-op acceptance. Network share and fixed-path game firewall activation remain pending; no remote PC was updated by this work.
