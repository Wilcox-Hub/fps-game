# RIDGEFIRE in Unreal Engine

RIDGEFIRE's Unreal Engine 5.8.3 project lives in `Ridgefire/`. The browser version at the repository root is a separate playable prototype and design reference; it is not the Unreal game.

## Open and play

On the configured PC, run `Open-Ridgefire.bat` or open `Ridgefire/Ridgefire.uproject` from Unreal Editor. The editor opens the shooter arena. Use **Play** in the editor toolbar to start a run. The batch file and project use the existing installation at `G:\Unreal Engine\UE_5.8`.

For a standalone development launch, use the editor's **Platforms > Windows > Package Project** workflow after opening and saving the project. Unreal compiles the C++ module when opening the project; Visual Studio Build Tools with the C++ desktop workload and the .NET Framework 4.8.1 developer pack are installed on the configured PC.

## Controls

- `W A S D`: move; mouse: look; left mouse: fire.
- `Shift`: sprint; `R`: reload; `F`: use a field patch.
- `E`: use a fully charged ion surge.
- `1`, `2`, `3`: choose one of the three opening armory offers.
- `T`: interact with the in-world arsenal when nearby; `B`: toggle the selected secondary.
- `P` or `Escape`: pause/resume. After defeat, `R` starts a fresh run; `Escape` keeps the game open.

## Current game loop

Choose between the Gale Repeater, chrome Phasma Rifle, or a newly rolled wildcard. Fight escalating waves of mechanical sentries, build kill streaks, charge an arena-clearing ion surge, and earn healing and ammunition between waves. Wildcards combine firing modes, payloads, conditions, and volatile random stats; their outcomes intentionally range from rough to wildly overpowered. Phasma's falling charge tone and deep echo are synthesized in-game. This is still a development build, not a signed-off beta; see `Ridgefire/BETA_PLAYTEST_LOOP.md` for verified tests and open gaps.

The arena/game loop lives in the `TP_FirstPerson` C++ module, layered onto the First Person Shooter template content. Keep maps and arena-specific behavior modular as new pits are added. Build output and editor caches are ignored by Git; commit the `.uproject`, source, config, maps, and required content assets.

For an automated editor-build gameplay smoke test, add `-RidgefireSmokeTest` to the Unreal Editor launch arguments. It chooses the wildcard, checks pause/resume and firing, clears wave one, verifies wave two, ammo resupply and a collectible drop, then checks game over and the absence of an automatic respawn.

## Validate the C++ editor target

```powershell
$project = (Resolve-Path 'unreal\Ridgefire\Ridgefire.uproject').Path
& 'G:\Unreal Engine\UE_5.8\Engine\Build\BatchFiles\Build.bat' TP_FirstPersonEditor Win64 Development "-Project=$project" -WaitMutex -NoHotReloadFromIDE
```
