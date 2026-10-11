# First-arena cover forms — VS55

Two original faceted rock forms refine the scaled-plan cover footprints. Blender 5.2.2 LTS source and FBX exports are retained here; the recipe runs only in a separate background Blender process. These are metric blockout forms, not finished textured production art.

- `IronSun_FirstArenaCover_v55.blend`: editable source with both rock forms and generated UCX hull objects. Objects are hidden for export isolation; unhide the desired form when inspecting.
- `SM_IronSunCoverRock_A.fbx` and `SM_IronSunCoverRock_B.fbx`: 200×200×100cm base geometry. In-game instances scale to the individual footprint/height in `RidgefireFirstArenaLayout.h`.
- `build-cover.py`: isolated generation/export recipe; emits a fresh `assets` directory beside itself and refuses to overwrite an existing source blend.

Engine assets live at `/Game/IRON_SUN/Environment/FirstArenaVS55`. Interchange did not retain the exported UCX hull in the first probe. The accepted normal Editor import generates a single convex hull from each convex render form, verifies measured bounds and saves the assets. Both are explicitly included in cooking. Capsule clearance and native bot/player-route checks operate on the imported hulls; collision is an engine convex approximation, not an exhaustive exact-surface oracle. Rough nonmetal material overrides replace the previous shiny ellipsoid massing.

The source uses no external textures or downloaded models. Rock surfaces, richer sculpting, LODs, moving-game human acceptance and minimum-PC profiling remain work. [Current evidence](../../../../../docs/beta-readiness/snapshots/2026-10-09-vs55/report.md).
