# VS57 server connection preparation — October 11, 2026

The free Playit one-port UDP tunnel is created and a real exact-nonce round trip through it to Lenovo passes. **The game is not live:** the matching native Linux Server compile is in progress. No paid plan was purchased.

Epic/GitHub linking and active Unreal source access are complete. The official 5.8.3 release source commit is `396c9f059903aed5fec78ecd3d437a40c6415368`. GitDependencies completed all 130,143 selected files. The signed Epic v26 / clang 20.1.8 SDK was downloaded, verified and extracted into G: without a global SDK installation.

The Lenovo Playit agent is claimed, connected and running in its pinned non-root container. Actual Compose startup uncovered comma-separated tmpfs options parsed as separate mount entries; both lists are now quoted and config/startup pass. Keys remain mode 600 and are absent from this record. The only tunnel routes UDP to `127.0.0.1:7777`; SSH, admin and Minecraft services were not exposed through it. Existing Minecraft/Crafty and Portainer remain untouched.

The 15.0 ms one-shot nonce round trip is a transport sample from the owner home network, **not** gameplay latency or a different-Wi-Fi acceptance. Its temporary listener exited normally. The transient agent-unsupported UI cleared; final creation initially rejected a long description and succeeded with a concise truthful software description.

The native compile initially used excessive first-build concurrency/cache settings. It was stopped and resumed with its cache on G:, capped storage, loopback-only build-worker listening, and four jobs after memory settled. Completed objects are reused. The packaging helper can use the matching installed editor for cooking while building the proper native Server from source; helper syntax passes, packaging itself is pending.

All 600 VS57 Source/Config/Content hashes match. Nine prior game validation groups are retained by reference with their original scope, date and artifacts. This work receives **no new gameplay credit**: the October 4 estimate remains 69%, overall beta is unmeasured and NOT_BETA_READY. Publication remains local/unpublished.

Next acceptance: native build/cook/package; Linux boot and same-build packaged Windows client join; game firing and original-squad restart/recovery; an actual different-Wi-Fi friend test; normal 2–4-player combat profiling alongside Minecraft. The public endpoint by itself is not a playable server.
