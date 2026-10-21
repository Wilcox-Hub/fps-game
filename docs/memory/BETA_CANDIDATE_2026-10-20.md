# Local VS69 candidate — October 20, 2026

The local technical preparation block is complete. Full beta acceptance is
still open, and the original host ammo failure is unreproduced. Public delivery
remains VS68 / game20261016-213325-765 / launcher1.0.1.

Candidate: `VS69-20261020-1d98618a77db`, protocol69. Both packages share 763
source/config/content inputs, fingerprint
`1d98618a77dba82e8b6ac7873901a7c64437463995c98677d8a639169a8de21d`.

| Package | Local folder |
| --- | --- |
| Windows Shipping | `G:/IRON-SUN-Work/BetaPreparation/VS69-Windows/20261020-185431-222/Windows` |
| Linux Development Server | `G:/IRON-SUN-Work/BetaPreparation/VS69-Linux-Direct/LinuxServer` |

All source inputs, 36 Windows / 35 Linux artifacts and both artifact manifests
were verified unchanged after testing. Windows includes an isolated diagnostic
launcher and a standalone test card. No installed public game was replaced.

Recorded technical passes:

- 24 real two-process ammo cases, including 100ms outbound delay per peer and
  2% packet loss. Trace receipts check actual useful gains and both carried pools.
- Six uninterrupted first-arena encounters with the same connected pawns and
  conserved resources. Enemy AI/deaths/approaches are controlled; this does not
  reproduce ordinary combat pressure or the reported failure.
- Actual no-gain rejection/retry, shared refit ownership and two-way rescue
  including real hit pauses and normal recovery timers.
- Exact Windows Shipping 1080p rack/E refit/both-gun fire and 74 optics assertions;
  four scoped frames inspected. Human feel and wider-resolution checks remain open.
- Two exact Shipping clients accepted by both Windows Editor dedicated and
  native Linux servers; exact VS68 rejected. These are isolated same-PC
  lobby/auth/version checks, not packaged multiplayer combat acceptance.

The native test uses the existing documented UE5.8.3 source-server launch option
`-networkversionoverride=55116800`. An earlier probe omitted it and was correctly
rejected with NetCL0; no game compatibility bypass was added. Earlier harness,
Docker and build-memory failures remain in the local evidence.

Next normal co-op test: copy the entire Windows folder to both PCs, start
`Play-With-Ammo-Diagnostics.cmd`, then host a fresh LAN party. Carry unupgraded
Gale/Phasma through the first arena; alternate pickups, reloads and slot swaps.
Also alternate rescue/refit and test safe-boundary close/reopen/Continue. Keep
both traces from `%LOCALAPPDATA%/IRON-SUN-Local-VS69/Saved/Diagnostics/`, host roles,
encounter/time and any video if an orb disappears without useful ammo.

Local detailed evidence: `docs/beta-readiness/snapshots/2026-10-20-vs69-candidate/`.
Local human checklist: `unreal/Ridgefire/LOCAL_BETA_TEST_CARD.md`. These detailed
readiness/source files remain unpublished; this concise memory note is synced.

Normal two-PC/controller/different-network combat, minimum-PC profiling/soaks,
human fun/readability, original ammo closure and remaining approved content/mode/
online work remain beta gates. The 100-roll audit stays paused. No new score,
game/source commit, launcher feed change or public deployment. 69% remains the
old low-confidence October4 gameplay planning estimate; NOT_BETA_READY.
