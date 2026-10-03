# VS32 — hosted party wipe and restart

Player outcome: a connected 2–4 player LAN party can lose a run and restart together without reopening the lobby, losing members, duplicating pawns, losing personal XP, or trapping movement/weapon controls.

Baseline: VS31, assessed October 3, 2026, gameplay estimate 61% (weighted 60.5), low confidence, NOT_BETA_READY. All 546 input hashes match before work. Preserve existing unrelated edits; game source remains unpublished.

Files: RidgefireGameMode restart/travel lifecycle; ShooterPlayerController transient state reset; RidgefireSessionSubsystem host creation after the listening port is ready and invalid LAN-address guard; controlled lobby fixture and its runner. No art/assets or production save files.

Acceptance: actual Null/LAN discovery and ready launch; normal 20/10/10/12-second setup; first defeat preserves survivors, last defeat ends the run; early and duplicate restart requests are rejected; a client initiates the first restart and host the second; fresh run state and distinct living possessed pawns retain the same party and personal XP; all peers repeat normal setup; gamepad session menu opens/closes; movement, fire and ammo-conserving reload work locally and on authority; host can still end the party cleanly.

User hosting direction: keep active run state on the host. Plan Steam lobbies/Quick Play with player-hosted matches; dedicated hosting is a future option when sales and activity support it. This work adds neither host migration nor shared cloud saves. Saved checkpoints remain a separate open milestone.

Validation limits: local controlled Editor peers and build/package checks. No claim of Steam/Internet, cross-device, physical gamepad, human fun, packaged full-run, minimum-PC or beta release acceptance.
