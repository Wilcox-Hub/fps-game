# October 22 local VS72 test candidate

`VS72-20261022-dcdb600b9032` is a local technical test candidate, **not an accepted
beta**. It adds the Garden seed-to-arsenal supply prototype and keeps the ammo
diagnostics. Source/game/packages are unpublished; public VS68 and launcher1.0.1
are unchanged. All three packages share775 source/config/content inputs.

| Package | Local folder under `G:/IRON-SUN-Work/AutonomousBeta/20261020/Garden72/` | Files |
| --- | --- | --- |
| Windows Shipping | `Shipping72-Source96-20261022-1636/Windows` |34 |
| Windows Development | `Development72-Source96-20261022-1621/Windows` |52 |
| Native Linux server | `Linux72-Source96-20261022-1630` |35 |

All recorded file sets and hashes remained unchanged after testing. The combined
local record is `Garden72/Candidate72-result.json`. Previous [Core70](BETA_CANDIDATE_2026-10-21.md)
remains separately preserved. The local readiness report is in
`docs/beta-readiness/snapshots/2026-10-22-vs72-supply-delivery/`; those game/evidence
files have not been published with this memory-only record.

## Use it when normal co-op is available

1. On both PCs, use the identical VS72 Shipping folder. Start
   `Play-With-Ammo-Diagnostics.cmd`; host a new LAN party from this candidate and
   join with the same candidate. The public VS68 server is incompatible.
2. Use unupgraded Gale and Phasma through the first six fights. Alternate both
   players collecting teal ammo, empty magazines/reserves, reloads and rapid B
   swaps. If an orb disappears without useful ammo, retain both traces, time,
   host role and video. The original failure remains unreproduced/open.
3. Try held-E rescue, fresh-press tube deposit, interruption and alternating
   rescuers. At refit, verify the700-shard/general hint, E insert, five-second
   work, other-gun firing and E collection into the reserved slot.
4. In Garden, E carry/plant the required seed and defend the first fight. During
   the open second-fight side path, E harvest the defended seed: no immediate
   ammo/XP. Bring it to the arsenal and E deliver. If the held gun cannot gain
   ammo, cargo stays; B switch then retry. The prototype awards up to18 stock
   reserve and10XP per currently connected player once; tuning is provisional.
5. Confirm cargo does not block carrier aim and the other player sees it. Try
   downed-carrier recovery, logout/rejoin and patient pickup. Missing optional
   delivery must not prevent normal progression. Keep recordings/logs.

This launcher uses the isolated profile `%LOCALAPPDATA%/IRON-SUN-Local-VS72/`.
Ammo traces are in its `Saved/Diagnostics/`. Rollback means closing this separate
candidate and reopening the existing public launcher; no installed game was replaced.

## What passed

Cooked Development world/lifecycle/checkpoints8.36s and full21 rendered delivery
155.71s pass; five actual1080p cargo images inspected. Shipping rack/refit22.11s
passes independent retained-evidence review; its original missing-log wrapper
failure remains recorded because Shipping logging is compiled out. Optics19.39s,
74checks and6aimed swaps pass; six selected real refit/optics frames inspected.
Both packaged client configurations pass two matching joins and old68 rejection
on isolated nativeLinux and WindowsEditor backends. Those are lobby/auth/version
checks. Source96 separately passes four actual Editor peers/full21 at0delay and
100ms/2%, including ownership, one reward, no-gain/replay and required route guards.

Normal two-PC/Internet, actual cargo transport recovery/remote visuals, final
arena content/art/audio, pacing, performance and blind human fun remain open.
Only five arena prototypes are integrated; the larger content target is unfinished.
Safe-boundary Continue passes; immediate mid-wave crash persistence is not guaranteed.
The100-roll wildcard audit stays paused.69% is only the October4 low-confidence
planning estimate; no new score or beta approval.
