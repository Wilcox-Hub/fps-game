# RIDGEFIRE

A first-person sci-fi gladiator shooter. Fight mechanical sentries in a sunset-soaked arena, build kill streaks, scavenge field drops, and save up an ion surge to clear the sky.

The Unreal Engine 5.8.3 C++ project is in [`unreal/Ridgefire`](unreal/Ridgefire). This Unreal project and the browser prototype are maintained in the same GitHub repository: https://github.com/Wilcox-Hub/fps-game.

## Play in Unreal Engine

The Unreal Engine 5.8.3 project is the main game: run [`unreal/Open-Ridgefire.bat`](unreal/Open-Ridgefire.bat) or open [`unreal/Ridgefire/Ridgefire.uproject`](unreal/Ridgefire/Ridgefire.uproject) in Unreal Editor. In the editor, press **Play** to enter the arena. Setup and controls are in [`unreal/README.md`](unreal/README.md).

## Browser prototype

The root-level browser build remains as a separate playable prototype and design reference. Open `index.html` in a modern browser. For the smoothest mouse-look experience, serve this folder over HTTP and open `http://localhost:8000`:

```powershell
python -m http.server 8000
```

## Controls

- `W A S D` or arrow keys: move and strafe.
- Mouse: aim; click and hold to fire. `Space` also fires.
- `Shift`: sprint. `R`: reload. `F` or `Q`: use a medkit.
- `E`: discharge a fully charged ion surge against nearby visible sentinels.
- `Escape`: pause or resume. Touch devices have on-screen movement, aim, fire, medkit, and surge controls.

## Field notes

Destroy sentinels quickly to keep a score multiplier alive. Scouts push in, skimmers harry you from range, and brutes absorb extra hits. Sentinels can drop health patches or charge cells. Clear a wave for a health and ammo resupply; each kill charges the ion surge. Your high score is saved locally in the browser.

## The armory

Each run offers the Gale Repeater, the chrome Phasma Rifle, and a freshly generated wildcard. The Phasma fires heavy plasma bursts with a synthetic falling whine, deep impact, and metallic echo. The wildcard rolls a firing mode, a payload, and sometimes a firing condition, with independently random damage, tempo, and magazine size. Its combinations are intentionally not balanced: some rolls are ridiculous, some are awful, and some are just right. It only rolls again when a new run enters the armory.
