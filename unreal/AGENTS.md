# RIDGEFIRE Unreal Game

Work in `unreal/Ridgefire/`; the root HTML/CSS/JavaScript browser prototype is separate. The Unreal tree is still local and untracked, not a published GitHub game build.

## Direction

- Build a fast, polished sci-fi gladiator FPS with modular arenas. First arena: sunset desert, tall armored giant mechanical sentries; later arenas have distinct enemy silhouettes and behavior. Preserve the chrome/plasma visual identity.
- The arsenal offers Gale Repeater, silver-chrome Phasma Rifle, and a generated wildcard. Wildcards may be weak, situational, or overpowered; constrain only for technical safety. Phasma audio should be original, with a descending metallic charge and deep echo rather than copied film audio.
- Favor a playable vertical slice over disconnected content; allow later weapons and arenas without redesigning the core.

## Workflow

- UE 5.8.3 is at `G:\Unreal Engine\UE_5.8`. Check the current checkout and existing changes before editing. Do not commit or push game work without an explicit request.
- Search the relevant heading in `Ridgefire/BETA_PLAYTEST_LOOP.md` for prior evidence; do not reread the full log for routine fixes. Record the exact validation level, especially when only NullRHI/headless checks ran.
