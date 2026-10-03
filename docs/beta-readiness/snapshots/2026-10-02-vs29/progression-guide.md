# Personal progression and run perks

Open personal perks with **K** or **L3** during the opening personal-perk stage or after defeat. Move with arrows/D-pad, equip slot one with **1/A**, and slot two with **2/X**. Exactly two distinct unlocked perks remain equipped. The ten perks cover Combat, Medic and Movement; thresholds range from 0 to 800 XP.

Every connected player receives 10 XP per enemy death and 30 XP per wave clear, including a downed teammate. The killer receives 3 extra XP for a fatal head/weak-point hit. A successful healing-tube recovery gives its depositor 12 XP. Failed/cancelled deposits and duplicate deaths do not grant bonuses.

The server owns XP and validates selections during setup/defeat. Local profiles save to `IRON_SUN_Progression_v1`; Steam/EOS identities use distinct server-side entries when available. Guest/Null identities remain session-only. Profiles retain XP through shared restart. Real Internet reconnect, cross-machine identity and save migration remain unaccepted. An incompatible existing save is preserved and the game uses a session profile; failed writes retain the dirty ledger for retry. Saves flush at wave clear, defeat, perk changes and subsystem shutdown.

The opening team-perk ballot normally lasts 10 seconds, resolving early when everyone has voted. F1/A votes for Shield (5% less incoming damage), F2/X for Quickload (10% shorter reload), and F3/B skips. Each player contributes one changeable ballot until resolution. Selection uses the displayed ballot proportions; no ballots defaults to Shield, and unanimous skip grants neither perk. Weapon setup follows the vote. Normal setup now runs 20 seconds for personal perks, 10 for a secondary, and 10 for the shared vote, followed by up to 12 seconds for an entry primary. Saved valid perks remain on timeout; an unselected secondary defaults to Phasma, and the entry primary defaults to Gale or Phasma when Gale is reserved as secondary. Each stage locks when it ends. Secondary selection uses the existing three offers, reserving that offer from both primary slots. Character preselection, matchmaking and a shared reveal animation remain unfinished.

Clearing Iron Sun earns team Momentum (+5% movement); stabilizing the Foundry anchor earns team Amplifier (+10% hit damage). These buffs combine with personal perks, apply to the team and reset on a fresh run. They never enter the permanent profile.

Use `Tools/Invoke-SetupChecks.ps1` for production timed stages (`-Defaults`, `-GaleSecondary`, `-Render`), and `Tools/Invoke-SetupNetworkChecks.ps1` for four local peers (`-Skip`, `-Disconnect`, `-VerifyRecovery -TimeoutSeconds 180`). The recovery fixture checks actual owned players, downed XP and a real sixty-second tube timer, with client observation of the same restored pawn. These constructed scenarios are not full four-human runs. Older progression/combat fixtures isolate their scenarios through an Editor-only short setup path. Use `Tools/Invoke-ProgressionChecks.ps1`, `-SkipVote`, or `-Interactions`. Add `-Render` and optional resolution arguments to the interaction fixture for actual engine frames. Unattended fixtures use disposable/session profiles; they do not write the player's real progression ledger. Editor voting fixtures extend only the discovery window, then exercise the same ballot/resolution path. The co-op combat runner additionally checks remote ballots, XP, team perks and restart reset. Consult `docs/beta-readiness/latest.json` before repeating checks.

Current evidence is controlled Editor/local-peer testing. It does not establish game balance, normal hardware input, packaged full-run play, Steam reconnect or human fun. Game work remains local until explicitly published.

## Personal perks and roles

Two equipped perks create flexible Combat, Medic or Movement builds. There is no separate Support category. Existing IDs, unlock thresholds and saved XP are retained. Conditioning is Combat; rescue belongs to Medic.

| Perk | Role | Benefit | Unlock XP |
| --- | --- | --- | ---: |
| Reinforced Plating | Combat | Incoming damage -5% | 0 |
| Stable Grip | Movement | Post-sprint/landing aim settles 35% sooner | 200 |
| Trauma Kit | Medic | Field-patch healing +15% | 0 |
| Conditioning | Combat | Maximum health +10% | 250 |
| Precision Tuning | Combat | Head-hit damage +8% | 100 |
| Quick Hands | Combat | Reload time -15% | 350 |
| Lightweight Rig | Movement | Movement speed +8% | 120 |
| Spring Assist | Movement | Jump launch speed +15% | 450 |
| Rescue Harness | Medic | Primary fire and B/Y weapon toggling while carrying | 600 |
| Charge Recovery | Combat | Ion-charge gains +15% | 800 |

Hold C/Left Ctrl or controller LT to crouch. All players gain 35% lower firing recoil when crouched, without spending a perk slot. Normal aim settling takes one second after stopping a moving sprint or a hard landing; Stable Grip reduces it to 0.65 seconds. Settling adds up to 50% extra camera recoil and widens the reticle as feedback; it does not add hidden bullet spread, damage or aim assist. Recoil is delivered to the weapon owner's camera; traces and damage remain server-authoritative. Physical-input/network-feel balance remains unaccepted.

Rescue Harness preserves the weapon held at pickup, allows primary fire and normal B/Y primary-secondary toggling, and retains baseline 65% carry speed/no sprint. Without it, carrying requires the secondary. The 60 earned-second healing-tube gate is unchanged. Jump launch speed is not a jump-height percentage. All handling and percentage values remain provisional pending human play.
