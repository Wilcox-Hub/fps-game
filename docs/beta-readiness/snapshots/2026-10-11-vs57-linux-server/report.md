# VS57 native Linux Server — October 11, 2026

**Native binary compiled; packaging in progress.** Relay game lobby join accepted: False. Wrong-invite rejection verified: False. Different-Wi-Fi/human acceptance remains open.

The actual official 5.8.3 source/v26 Linux compile and link pass. [Build diagnosis](evidence/native-server/linux-build-diagnosis.md) records the original five-second incremental repro and repair: Server-only non-unity compilation isolates file-local helpers; encounter tests requiring editor hooks now require editor support. No runtime bodies, network fields, RPCs or protocol changed. The final native ELF binary is 273,974,200 bytes, SHA256 `459889A0C444AAC6778F8E98D510210C6A995E893E161E5F7A2872C0BC61D5BF`.

The current manifest has 600 inputs. [Source delta](source-delta.json) identifies two changed build/test boundary inputs; the other 598 match VS57. Earlier Windows game tests/build/package/rendered checks retain their historical dates, inputs and validation levels. They were not rerun or relabeled as current exact-input passes. The installed Windows VS57 package remains the compatibility client.

The free Playit one-port UDP relay is created and exact-nonce transport verified. No paid plan or other public service exposure was added. The join key remains private and outside these records.

Completed native deployment and actual packaged lobby controls, if present, appear individually in [results](test-results.json). Smoke clients use production JoinServer with isolated save directories; no UI, combat, full-run, original-squad recovery, external-Wi-Fi or normal performance approval is inferred from lobby joins.

The October 4 gameplay estimate remains **69%**, overall beta unmeasured and **NOT_BETA_READY**. Remaining gates include actual different-Wi-Fi friend firing/recovery, normal 2–4-player profiling alongside Minecraft, owner UI/combat/pacing acceptance and the wider game completion scope. Local source/docs remain unpublished.
