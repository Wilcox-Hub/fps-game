# Private internet co-op on Lenovo — VS60

October 12, 2026: **LIVE FOR INTERNAL ALPHA**. The native Linux Server and approved free Playit UDP tunnel are running. Two actual VS60 Windows Shipping clients connected over LAN and Playit, held for 12 seconds with no unresolved arena references, and a wrong invite was rejected. The clients used the owner PC/home network. Different-Wi-Fi combat, original-squad recovery, normal capacity and human acceptance remain open.

## Join the private server

1. Install the full matching VS60 game on both PCs. The owner desktop launcher/local update feed is already refreshed. The wife/friend PC still needs the update; an older EXE alone will not work.
2. Choose **JOIN SERVER**. Paste `cairo-ferry.tun.ply.gg:42527` and the private squad invite from the owner connection card at `%LOCALAPPDATA%/IRON SUN/Private-Server.txt`.
3. Everyone readies up. The first connected player leads and starts the run after at least two players are ready.

Only Lenovo runs Playit; friends need the matching game and invite, with no VPN app. Keep the invite card out of GitHub/public galleries. Full ZIP: the local feed's `iron-sun-20261012-034847-088.zip`, SHA256 `386D00B90847AB5E6542516203DBFA070FB66FC762F217C363DA2723EEEA182F`. The current full game ZIP is also available at https://iron-sun-playtest.wilcoxwarrior4577.chatgpt.site/; launcher security review remains separate. See [test card](VS60_COOP_TEST_CARD.md).

## Deployment

SSH alias `sidequest` reaches Ubuntu on Lenovo at `192.168.4.36`. Added services live under `/home/caiden/iron-sun-server`; existing Minecraft/Crafty and Portainer were preserved. `current` selects native release `2a634f02ad9bbb7b`; VS58 remains available for rollback. Game source remains unpublished; readiness metadata is published separately.

The runtime uses Ubuntu 24.04, UID 1000, a read-only root, dropped capabilities, separate saves/logs and initial limits of 3 CPU/6 GiB. The relay is the official Playit v1.0.12 image pinned by digest. Both services have Docker restart policies. These limits are configuration, not measured four-player capacity.

The approved free endpoint forwards exactly one public UDP port to `127.0.0.1:7777`. Game UDP 7777 binds only loopback and the current Lenovo LAN address. SSH, Portainer and Minecraft were not exposed through Playit. The LAN binding in `.env` must follow any future DHCP address change. No paid plan, Steam/matchmaking, recurring automation or automatic untested server deployment was added.

Private keys are stored separately in `config/join_key` and `config/playit_secret`, mode 0600. The shared squad invite generates protocol/time-bound HMAC proofs; the long-lived key stays out of travel URLs. Client/server clocks must agree. This is shared squad access, not authenticated online identities or anti-cheat.

## Build and update

The official Epic 5.8.3 source is commit `396c9f059903aed5fec78ecd3d437a40c6415368`, with v26/clang 20.1.8 SDK. Epic/GitHub link, source access and Playit claim/policy acknowledgments are complete. The SDK signature/hash were verified and it was extracted without a global SDK installation.

`Tools/Build-LinuxServer.ps1 -SourceEngineRoot <source-engine> -CookEditorRoot <matching-editor> -ArchiveDirectory <new-directory> -MaxParallelActions 2` compiles the native Server from source, reuses the same 5.8.3 editor for cooking, and writes 607-input/protocol metadata. Source-only Server non-unity compilation isolates file-local helpers; editor-hook tests require editor support. Shipping Windows uses `Tools/Build-WindowsShipping.ps1`.

Source builds report CL 0; the verified 5.8.3 compatible changelist is supplied by `-networkversionoverride=55116800` in Compose. Engine/game protocol validation and private invite validation remain active. Actual successful/rejected client joins validate compatibility. VS60 fixes generated collision/movement-base names to match Windows/Linux, independent of cosmetic/material availability. Older packages must update.

Zip the entire directory containing `TP_FirstPersonServer.sh`. Upload to `incoming/`; `publish-release-vs60.py <absolute-zip> <sha256>` validates checksum, archive paths/sizes, native ELF and VS60/5.8.3 metadata before atomic activation. Secrets/saves stay outside archives, and the previous release is retained. Then restart only IRON SUN. The deployed VS60 ZIP SHA256 is `2a634f02ad9bbb7b616736a0b2f1b74de546f46625a5563a5f19310631d11ac7`.

From Lenovo's managed directory:

```sh
python3 manage.py status
python3 manage.py start --relay
python3 manage.py backup
python3 manage.py stop
python3 manage.py rollback
```

`start` recreates only the game container and checks listening boot; `--relay` ensures the relay is running. `stop` stops both IRON SUN services and backs up saves. `rollback` selects the previous release and leaves services stopped; clients must match that release before starting. The publisher's `process_restarted` flag describes its file-activation action; live boot is recorded separately.

## Runs and recovery

The first connected player leads; leaving transfers leadership. The server owns `IronSun_ServerRun0/1`, independent of ordinary hosted player saves. Cleared-encounter/regroup/arena-entry boundaries save progress; a first-encounter exit before any checkpoint starts fresh. Wipes/victory retire the run. On squad disconnect, the server returns to the lobby; an existing checkpoint requires the original squad to reconnect and continue. Preserve client player-identity saves. No mid-encounter joining/reclaim or host migration was added.

These behavior checks passed in earlier controlled Windows Editor two-client fixtures, with invulnerability/frozen AI/fixture-completed damage. They retain their original validation level. Native Linux restart/checkpoint behavior has not yet been accepted by this deployment.

## Evidence

Start with `docs/beta-readiness/latest.json`, [VS60 report](../../docs/beta-readiness/snapshots/2026-10-12-vs60-lean-ion/report.md), and [scoped results](../../docs/beta-readiness/snapshots/2026-10-12-vs60-lean-ion/test-results.json). Native package and current readiness manifest have the same 607 hashes. Current checks include lean/Ion and grounded progression, owned local Editor input/state, Windows/Linux builds, 32 installed file hashes, boot, wrong-invite control and two-client LAN/Playit lobby holds. Earlier checks keep their recorded dates and limits. NoSound/NullRHI smoke does not establish sound quality, rendered Shipping UI, normal firing/balance, human feel or performance. A fresh empty server lobby is left running.

Before external beta: perform real different-Wi-Fi primary/secondary combat, cleared-wave server restart/original-squad recovery, normal 2–4-player profiling alongside Minecraft, and owner UI/combat/pacing/full-run acceptance. The dated October 4 gameplay estimate remains 69%, overall beta unmeasured and NOT_BETA_READY.


## Corrected authority recovery

A remote client timeout is handled by authoritative Logout; it must never send the dedicated or listen host through a client menu recovery path. Final role/auth/save regression tests pass. Two actual corrected Shipping clients hold native LAN/Playit lobbies with explicit driver/connection-close guards, and forcibly terminating one LAN peer leaves the other connected after the real native timeout/logout. No new combat or different-Wi-Fi acceptance is implied. The earlier VS60 candidate was removed from latest and marked prerelease after raw-log review exposed a stale-roster false pass; it is superseded by this build.
