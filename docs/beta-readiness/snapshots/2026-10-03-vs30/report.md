# VS30 run foundation — October 3, 2026

Gameplay planning estimate: **58%**, weighted **57.5**, low confidence. **75% remains the target and has not been reached. NOT_BETA_READY.** Overall beta completion is unmeasured; release approval remains false. This adds 7.5 subjective planning points to VS29 using the same weighted areas. The estimate is not a test pass rate or a measured production completion percentage.

## Player-facing change

Normal play now has a beginning, branching middle and finish. Clear two Iron Sun waves; vote for Brassfall or Mirror Delta; optionally visit the other branch; reach Last Star and finish a lens-protected champion. Each run visits three or four unique prototypes and ends after five or seven waves. Route ballots use owner RPCs, majority resolution, seeded ties and a real fifteen-second timeout. Late/stale/invalid ballots are rejected. A disconnected voter is removed. Clearance rewards and terminal victory are granted once.

Mirror Delta adds stepped side paths and three numbered relay controls. Last Star adds lens controls and an existing brute-based prototype champion, whose guard reduces incoming damage by 75% until lens shutdown and whose movement changes below half health. Arena transitions check walkable arrival positions, preserve possessed pawns/loadouts, reset the old aim pitch and wait for occupied recovery tubes or carried teammates. The latter wait is implemented but its specific full carry/tube-at-transition scenario has not yet been independently exercised. Existing normal recovery is freshly checked.

The added material is a real Unreal asset with working color, roughness, metallic and emission parameters. Four actual 1280×720 offscreen D3D12 frames were inspected. Geometry, character, weapon and enemy art remain primitive. Lens pylons are too bright for production acceptance; the initial arena capture has a shader preparation overlay. Constructed fixture cameras and frozen NPCs do not establish normal combat readability or frame pacing.

## Evidence

- Editor build succeeds on UE 5.8.3. Added tests are included through a build without stale UBT makefiles.
- 5,000 seeds × four vote patterns verify 20,000 bounded unique routes, majority choices, deterministic ties and duplicate-clear rejection. RunPlan inputs did not change after that pass.
- Final actual-world integrated automation uses normal setup timeouts, production input bindings/relay proximity/line-of-sight checks, real spawned NPC death handling and one victory. It checks guard damage, half-health phase, route countdown updates, aim reset and no duplicate terminal rewards. Enemies are deliberately frozen/killed and the fixture player is teleported near controls. The final NullRHI report succeeds and process exit is 0.
- The rendered final-source run exported an authoritative Success report with no errors/warnings and four inspected captures. Its original wrapper rejected missing buffered final log text; process exit was not recorded for that attempt. The wrapper now reads reports. The separate final NullRHI run verifies clean exit.
- Four real local Editor peers finish a controlled route and observe the same victory. This earlier no-disconnect gate is scoped to its recorded iteration; final camera/material/HUD and logout/countdown changes are explicitly not hidden by a broad saved pass.
- A final-source run starts four peers, terminates one abstaining voter, resolves with three eligible ballots and requires all remaining peers to observe the four-arena/seven-wave victory. This passes. It does not test AI takeover or reclaim.
- A final-source four-peer normal setup/recovery fixture passes: one remote player is downed, carried and deposited; actual sixty-second recovery restores the same pawn and replicates to all peers. Positions are constructed and enemies frozen.
- Two-peer authoritative firing, reload, single rewards, full wipe, shared restart, actual remote movement and post-restart firing pass on the unchanged legacy path. Scoped reuse is recorded in test-results.json.
- A fresh Windows Shipping package built/cooked/staged/archived successfully. Its runtime survives a 22.44-second offscreen D3D12 startup check. This is not a full packaged gameplay session.

See scores.json, test-results.json, input-manifest.json, test-excerpts.txt, rendered-evidence.json and packaged-startup.json. All historical snapshots are retained. Game code remains unpublished local work; this GitHub snapshot is evidence, not a source-code release or downloadable beta.

## Remaining release gates

The owner cannot human-test now. Human balance, normal-input packaged full runs and audio acceptance remain pending. Ten finished arenas, intended 45–60-minute progression pacing, twenty-minute survival, saved party checkpoints, character preselection/reveal, integrated lobby/Steam, disconnect AI/reclaim, production assets and agreed minimum-PC profiling/soaks remain unfinished. The prototype relay puzzle does not implement Mirror Delta's full hostile-beam hazards. The champion is not an original finished broadcast boss.

Supervised internal alpha is still the current recommendation. Broader closed distribution remains conditional on a normal-input packaged session. The [75% target plan](target-75.md) remains open; this assessment does not approve a 75% claim.
