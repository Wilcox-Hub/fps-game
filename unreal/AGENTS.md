# RIDGEFIRE Unreal Game

Work in `unreal/Ridgefire/`; the root HTML/CSS/JavaScript browser prototype is separate. The Unreal tree is still local and untracked, not a published GitHub game build.

## Direction

- Build a fast, polished sci-fi gladiator FPS with modular arenas. First arena: sunset desert, tall armored giant mechanical sentries; later arenas have distinct enemy silhouettes and behavior. Preserve the chrome/plasma visual identity.
- The arsenal offers Gale Repeater, silver-chrome Phasma Rifle, and a generated wildcard. Wildcards may be weak, situational, or overpowered; constrain only for technical safety. Phasma audio should be original, with a descending metallic charge and deep echo rather than copied film audio.
- Favor a playable vertical slice over disconnected content; allow later weapons and arenas without redesigning the core.

## Workflow

- UE 5.8.3 is at `G:\Unreal Engine\UE_5.8`. Check the current checkout and existing changes before editing. Do not commit or push game work without an explicit request.
- Search the relevant heading in `Ridgefire/BETA_PLAYTEST_LOOP.md` for prior evidence; do not reread the full log for routine fixes. Record the exact validation level, especially when only NullRHI/headless checks ran.

## Beta readiness records

For beta status, scoring, milestone planning, or test selection, read `docs/beta-readiness/latest.json` and `docs/beta-readiness/README.md` first. Reuse saved evidence when its source inputs and validation scope match; use the folder's hash checker before applying historical passes to current code. After relevant tests or a readiness assessment, add a dated snapshot and update the latest pointer and overview together. Keep recorded percentages dated and distinguish gameplay estimates from overall beta acceptance.
