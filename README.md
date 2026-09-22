# Neon Breach

A small, dependency-free first-person survival shooter that runs in a modern browser. Clear the breach, survive escalating waves, and chase your local high score.

## Run

Open `index.html` in a modern browser. For the smoothest mouse-look experience, serve this folder over HTTP and launch it at `http://localhost:8000`:

```powershell
python -m http.server 8000
```

## Controls

- `W A S D` or arrow keys: move and strafe.
- Mouse: aim; click and hold to fire. `Space` also fires.
- `R`: reload. `F` or `Q`: use a medkit. `Shift`: sprint.
- `Escape`: pause or resume. Touch devices have on-screen movement, aim, fire, and medkit controls.

## Run loop

Enemies close in and attack at short range. Clear a wave to earn reserve ammo and a small health boost; every third wave adds a medkit. Drones take two hits, while later-wave brutes take more. Your best score stays in this browser on this device.
