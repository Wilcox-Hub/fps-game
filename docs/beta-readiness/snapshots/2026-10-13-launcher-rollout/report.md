# Existing-launcher automatic rollout — October 13, 2026

The existing production launcher already receives the published VS61 lobby/game update automatically. Opening an installed launcher triggers its signed launcher-version check, then a game update; an open idle launcher checks the game every ten minutes. Updates wait while the game is running. A closed launcher does not run a background service. Friends do not need to rerun setup for this update.

The public latest release, game/launcher manifests and website setup feed agree. All three production-key signatures and the three exact game/launcher/setup asset digests pass. The current launcher remains 1.0.1, with the exact reviewed hash; no replacement launcher binary or duplicate release was needed or published.

A copy of the actual complete VS60 installation was upgraded by the unchanged native production launcher over live HTTPS to VS61. All 32 new file hashes match, all 32 previous files remain available for rollback, a synthetic root save control is unchanged, and a repeat update reports CURRENT. All 20 original launcher source/tool inputs and all 619 current game inputs match their recorded manifests. Automatic UI startup/idle behavior is verified by inspection of that unchanged compiled source; this backend run does not claim normal GUI acceptance.

The initial harness path was too deeply nested for classic .NET Framework Windows file APIs. The same unchanged launcher passed from a short isolated temporary installation path. This is a harness correction, not a new long-path support feature.

No friend-PC receipt, actual Unreal saved-run upgrade, normal GUI-button, physical two-PC combat or different-Wi-Fi acceptance is claimed. The unsigned publisher/SmartScreen limitation is unchanged. Prior game evidence stays in the VS61 and earlier snapshots at its original scope. The 69% gameplay value remains the October 4 low-confidence estimate; NOT_BETA_READY and the open 75% target are unchanged. This task publishes delivery verification metadata only, preserving all unrelated game work.
