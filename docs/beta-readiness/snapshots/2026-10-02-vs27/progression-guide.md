# Personal progression and run perks

Open personal perks with **K** or **L3** during pre-run setup or after defeat. Move with arrows/D-pad, equip slot one with **1/A**, and slot two with **2/X**. Exactly two distinct unlocked perks remain equipped. The ten perks cover tank, assault, health and movement; thresholds range from 0 to 800 XP.

Every connected player receives 10 XP per enemy death and 30 XP per wave clear, including a downed teammate. The killer receives 3 extra XP for a fatal head/weak-point hit. A successful healing-tube recovery gives its depositor 12 XP. Failed/cancelled deposits and duplicate deaths do not grant bonuses.

The server owns XP and validates selections during setup/defeat. Local profiles save to `IRON_SUN_Progression_v1`; Steam/EOS identities use distinct server-side entries when available. Guest/Null identities remain session-only. Profiles retain XP through shared restart. Real Internet reconnect, cross-machine identity and save migration remain unaccepted. An incompatible existing save is preserved and the game uses a session profile; failed writes retain the dirty ledger for retry. Saves flush at wave clear, defeat, perk changes and subsystem shutdown.

The opening team-perk ballot normally lasts 10 seconds, resolving early when everyone has voted. F1/A votes for Shield (5% less incoming damage), F2/X for Quickload (10% shorter reload), and F3/B skips. Each player contributes one changeable ballot until resolution. Selection uses the displayed ballot proportions; no ballots defaults to Shield, and unanimous skip grants neither perk. Weapon setup follows the vote. This is a small starting vote, not the complete 20/10/10-second character/perk/secondary lobby.

Clearing Iron Sun earns team Momentum (+5% movement); stabilizing the Foundry anchor earns team Amplifier (+10% hit damage). These buffs combine with personal perks, apply to the team and reset on a fresh run. They never enter the permanent profile.

Use `Tools/Invoke-ProgressionChecks.ps1`, `-SkipVote`, or `-Interactions`. Add `-Render` and optional resolution arguments to the interaction fixture for actual engine frames. Unattended fixtures use disposable/session profiles; they do not write the player's real progression ledger. Editor voting fixtures extend only the discovery window, then exercise the same ballot/resolution path. The co-op combat runner additionally checks remote ballots, XP, team perks and restart reset. Consult `docs/beta-readiness/latest.json` before repeating checks.

Current evidence is controlled Editor/local-peer testing. It does not establish game balance, normal hardware input, packaged full-run play, Steam reconnect or human fun. Game work remains local until explicitly published.

## Personal perks: equipment and training

These are passive loadout choices. They add no active power buttons, cooldown abilities, supernatural saves, automatic extra shots or air dashes. Two slots and the existing XP thresholds are unchanged.

| Perk | Benefit | Unlock XP |
| --- | --- | ---: |
| Reinforced Plating | Incoming damage -5% | 0 |
| Braced Stance | Incoming damage -10% while crouched | 200 |
| Trauma Kit | Field-patch healing +15% | 0 |
| Conditioning | Maximum health +10% | 250 |
| Precision Tuning | Head-hit damage +8% | 100 |
| Quick Hands | Reload time -15% | 350 |
| Lightweight Rig | Movement speed +8% | 120 |
| Spring Assist | Jump launch speed +15% | 450 |
| Rescue Harness | Carry at 80% walking speed; baseline 65% | 600 |
| Charge Recovery | Ion-charge gains +15% | 800 |

Braced Stance rewards committing to a position; Precision Tuning rewards aiming. Other choices support handling, survival or rescuing teammates. Plating and crouch armor multiply: 0.95 x 0.90 = 0.855 incoming damage while crouched. Jump launch velocity is not a jump-height percentage. Equipment wording and these values are provisional pending human balance testing. Existing perk IDs and saved unlocks are retained.
