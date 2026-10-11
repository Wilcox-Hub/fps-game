# VS56 — host checkpoints and safer lobby launch

**Verdict: NOT_BETA_READY.** The October 4 gameplay estimate remains **69%**, low confidence. No new whole-game percentage or 75% acceptance is inferred from these technical checks. Existing full-beta scope is retained; the private-friends-versus-full-beta clarification has not been answered.

Safe regroup checkpoints and a clear **Continue run** option now preserve a host's run. Original squad identity matching restores each player's two guns, spent ammo, upgrades, shards, health/resources and progression together. Route history, team perks and objectives persist. Continuation gives 90 seconds to regroup, with early start; a team wipe or victory retires the run. Corrupt/unsupported state is preserved and Continue blocked. Normal combat resumes from the last safe boundary, rather than serializing live bots or timers.

An early cold-client click exposed an input-registration timing gap. Ready/launch inputs and their buttons now wait for the local pawn's controls. After the fix, actual local host and guest processes restart from disk and both fire/switch correctly. Native Continue UI was inspected at 1080p. [Frame](evidence/checkpoint-disk-and-world/20261009-230745-991-continue-run-1080p.png).

Current evidence includes two disk/world tests across four real arena layouts, cold two-peer disk recovery and rejection controls, four-layout/party wave regression, normal two-peer entrance/body/firing checks, both legacy handshake directions, Editor and Shipping builds, and actual Shipping lobby startup. These are controlled fixtures, not natural human or physical-device runs. [Test results](test-results.json) and [input hashes](input-manifest.json) define the scope.

Prototype **20261009-230836-509** is verified into the owner fixed launcher; all 32 payloads and the ZIP match. The install was updated without launching it. The other PC needs this same VS56 build. LAN protocol is 56. Source, dated records and test runners are local/unpublished; no GitHub push, network-share/firewall activation or other-PC update is claimed.

Technical checkpoint support is implemented, but packaged physical two-PC recovery, normal full-run combat/economy/fun, final menu/art acceptance, minimum-PC performance and the broader planned arena/progression/online scope remain open. The first map is still a refined blockout. Beta sign-off cannot be replaced by automated passes.

[Two-PC test card](../../../../unreal/Ridgefire/VS56_COOP_TEST_CARD.md) · [Recovery contract](../../../../unreal/Ridgefire/RUN_RECOVERY.md) · [Changed inputs](changed-inputs.json) · [Artifact hashes](artifact-manifest.json) · [Scoped reused evidence](reused-evidence.json)
