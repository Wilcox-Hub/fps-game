# VS57 — Lenovo private-server preparation

October 11, 2026. **PREPARED, NOT LIVE. NOT_BETA_READY.** The gameplay estimate remains **69%**, a low-confidence October 4 planning estimate. This networking/deployment slice earns no new gameplay or overall-beta percentage.

The owner approved a private server on the Lenovo with free software, with friends joining through the game and no VPN app. VS57 adds Join Server with an address and masked invite, a dedicated Server target, server-selected leadership, separate server checkpoint slots, full original-squad save preservation and empty-server lobby recovery. LAN and solo remain available.

The server stores its invite privately and validates a short-lived HMAC proof before accepting players. This is shared-invite access for trusted playtesters, not authenticated online identity, encrypted gameplay or anti-cheat. The normal engine compatibility check remains enabled, with game protocol 57. Old VS56 clients must update.

## Current evidence

| Check | Result and limit |
| --- | --- |
| Editor build | Pass, UE 5.8.3. Final fixture-only deadline update compiled afterward. |
| Windows Shipping package | Complete Prototype `20261011-085109-773`; 32 payload files fingerprinted. |
| Private helper automation | 3 pass: address validation, invite/protocol/time proof, isolated server checkpoint slots. |
| Checkpoint regression | 2 pass: actual standalone Editor disk/world tests; no packaged recovery claim. |
| Dedicated lifecycle | Actual Windows Editor dedicated process plus 2 local clients: wrong invite/retry, leader-only launch, timed setup, primary/secondary LMB ammo, leader departure, boundary save, both peers leaving, original-squad continuation. A remaining player waits across several autosave ticks so the saved roster cannot shrink. |
| Client exit | Requested ordinary exit reached world and net-driver teardown; final OS exit was not asserted. |
| LAN regression | Two actual local Editor peers pass discovery/join, ready/unready, timed setup, leave/end, clean menu, rehost and second launch. |
| Join Server form | Three inspected 1920×1080 Editor frames; actual Slate pointer routing, full-width masked fields, invalid address/invite rejection and Back. Original October 11 capture time retained; final server-only changes do not alter the menu implementation. No human UI acceptance. |
| Packaged private join | Actual Windows Shipping runtime accepted by a local Editor dedicated process through production JoinServer, using the opt-in session-smoke harness. Server logs establish acceptance; Shipping client logging is unavailable. Lobby startup only, no packaged firing/UI acceptance. |
| Lenovo deployment | Live SSH/Docker checks pass: Compose and shell syntax, non-root runtime image, private key permissions, missing-native-package refusal, unsafe-archive-path rejection and no IRON SUN service start. Minecraft/Crafty and Portainer remain running. |

The dedicated fixture freezes AI, makes players invulnerable and completes a controlled first encounter to reach a real checkpoint boundary. These results do not measure combat balance, fun, normal arrival pressure, latency or four-player capacity. Existing broader gameplay evidence keeps its original dates and limitations.

## Deployment and blockers

Own deployment directory: `/home/caiden/iron-sun-server`. The Ubuntu runtime image is built; the official Playit relay image is pinned by digest. Game files, saves, logs and secrets are separated. A native release selects atomically, keeps the previous release, and starts only through explicit management commands. Status does not print secrets. No game or public relay is currently running.

The Launcher engine cannot build the dedicated Server target. Both inspected GitHub accounts lacked Epic repository access; the owner must link the appropriate GitHub account to Epic and accept the organization invitation. A matching Unreal 5.8.3 source build and Linux SDK/native executable have not been downloaded/built/validated.

The Playit login page is waiting for owner sign-in. The agent is not claimed and no public UDP tunnel/address exists. No paid service was purchased. Friends will need only the matching game once the server is live; that outcome is not yet delivered.

See [PRIVATE_SERVER.md](../../../../unreal/Ridgefire/PRIVATE_SERVER.md) for account steps and the prepared build/deployment commands. No source commit/push was requested or performed. The local launcher/feed delivery result is recorded separately in this snapshot.

## Test repairs and remaining gates

Early fixtures exposed outdated assumptions: the LAN test used the retired M menu key and readied before local inputs existed on rehost; those controls now use F10 and wait for readiness. The private recovery test initially expected a save before the first encounter cleared; checkpoints begin at actual safe boundaries. Its deadline was extended to include normal arrivals and a longer disconnected-player hold. A first packaged join attempt used a command-line travel URL that Shipping ignores; the passing check calls production JoinServer through the existing smoke-harness pattern. These earlier failed attempts are not acceptance evidence.

Remaining: Epic access and native Linux build/run; Playit claim/tunnel; actual external-Wi-Fi matching packaged clients with both guns; cold server restart/original-squad recovery; normal 2–4-player profiling alongside Minecraft; owner UI/combat/pacing acceptance and existing whole-game beta gates. Public matchmaking, Steam invites, player account authentication, mid-encounter reconnect/reclaim and host migration remain separate work.


## Local delivery

The VS57 local feed is published. The owner fixed launcher reports UPDATED then CURRENT with NoLaunch, and all 32 installed files match the built package. VS56 remains as rollback. The fixed install was not played; the packaged loopback join used the archived runtime. The wife PC, authenticated LAN share and internet distribution are not verified by this delivery.
