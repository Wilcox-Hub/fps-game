# RIDGEFIRE visual and performance targets

Status: working proposal; not yet approved as the beta minimum-PC target.

## Visual identity

Keep the existing dusk-pit broadcast identity: ember/orange arena light, deep ink panels, ice/teal energy, pale warm text, and chrome weapon silhouettes. The root browser prototype is a read-only reference for the broadcast framing, dramatic armory choices, and expressive hit/weapon feedback; Unreal remains the shipped game. Do not copy assets or audio from third parties.

The combat view must prioritize the threat and the equipped weapon. Every weapon needs a recognizable silhouette, a visible barrel/muzzle origin, and mode-specific fire/impact feedback. Sentries need readable silhouettes and attack telegraphs against the sunset. Keep cover and set dressing from hiding the player’s first threat or the main combat lane. HUD panels should use a dark backing rather than relying on text over scenery.

## Resolution and readability checks

Run the same armory, live-combat, pause, and game-over views at 960×540, 1280×720, and 1920×1080, plus one 4:3 viewport. Record the actual game viewport dimensions (not the desktop screenshot dimensions), build, date, and state in each capture name or test card.

| Check | Minimum target |
| --- | --- |
| Safe area | Essential text and controls stay at least 3% inside each viewport edge. |
| Essential HUD | Health, ammunition, wave, active weapon, and pause state remain visible with no overlap or clipping. |
| Text | At 1280×720, essential labels are at least 16 px; at 960×540, at least 13 px. Secondary copy may be smaller only if it is not needed to play. |
| Contrast | Essential text and hit/attack cues remain distinct over both bright sky and dark arena geometry; verify in motion, not only on a still. |
| Armory | All three offers, their defining weapon traits, and their input prompts fit without truncation or requiring a scroll. |
| Action readability | The equipped weapon, sentry silhouette, attack cue, damage response, and shot impact can be distinguished during movement. |
| Aspect ratio | 16:9 and 4:3 keep the combat center and essential HUD intact; no HUD item is positioned solely by an unsafe fixed edge offset. |

### Reproducible capture procedure

Use the packaged build for resolution evidence. Epic documents `r.SetRes` as changing the current game view and explicitly notes it has no effect in the editor. From the project root, launch the packaged executable in a window with `-ResX=960 -ResY=540 -windowed` (repeat with `1280x720`, `1920x1080`, and `1440x1080` for 4:3). In the running game, `r.SetRes 960x540w` can change a packaged game window; use the matching size for each pass. Do not use that command in PIE as proof of render resolution.

At each size, capture the game frame with `Shot` or `HighResShot 1` (not a desktop screenshot). Check the resulting image dimensions, then record the executable/build date, requested and captured dimensions, game state, and graphics settings next to the image. Capture armory, active combat, pause, and game-over states. Stills only support layout checks; attack readability and contrast still require moving-action review. Epic references: [console variables (`r.SetRes`)](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-console-variables-reference) and [taking screenshots](https://dev.epicgames.com/documentation/unreal-engine/taking-screenshots?application_version=4.27).

## Performance proposal

Proposed minimum test PC: four-core desktop CPU, 16 GB RAM, and a GTX 1660 / RX 6600-class GPU or better, running Windows 10/11. Proposed target: 1080p, 60 FPS average (16.7 ms frame time), 1% low at least 45 FPS (22.2 ms), no sustained hitch above 100 ms during ordinary combat, and a 30-minute crash-free run. These are **proposals only**; the project owner must approve or adjust the floor before any performance sign-off. Capture the actual test machine’s CPU, GPU, RAM, resolution, graphics settings, average, 1% low, and frame-time capture.

## Current evidence and gaps

- Existing saved screenshots are dated 2026-09-23; their PNG dimensions often describe the full desktop capture, not the game viewport. Examples: `Saved/IRON-SUN-960x540-Visual.png` is 1920×1080, `Saved/IRON-SUN-Core-Repair-Visual.png` is 1280×720, and `Saved/iron-sun-compact-arena-800x600.png` is 816×639. These filenames/dimensions do not prove controlled viewport sizes. `Saved/dustcrown-phasma-gameplay-4x3-final.png` is 1440×1080, but does not prove how the viewport was set.
- The latest reviewed combat capture shows the current sunset/ember palette and HUD, but the weapon silhouette is pale teal and visually simple; broad central cover also obscures the forward lane. Weapon model/material and first-threat visibility remain below target.
- The browser prototype’s menus, wildcard language, and feedback are reference material only; no claim is made that those visual features have been ported to Unreal.
- Local hardware/build reports show Windows 11 25H2, AMD Ryzen 7 9800X3D, NVIDIA GeForce RTX 3060, and 31.16 GB physical memory. This is a development-machine observation, not the approved minimum specification.
- UE 5.8.3 is installed at `G:/Unreal Engine/UE_5.8`; the packaged build exists under `Saved/Package-Ticket25-20260927`. This confirms tools/build availability, not a visual test. No resolution-controlled captures or frame-time profile have been produced for this target yet.

## Sign-off record

Do not close the related visual-target ticket until the project owner approves the minimum PC and the team attaches fresh, correctly labeled viewport captures at all three 16:9 targets plus a recorded performance pass on the approved minimum PC.
