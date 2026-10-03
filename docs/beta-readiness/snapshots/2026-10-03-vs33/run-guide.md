# Prototype progression run

Ordinary play follows a finite route. Opening setup uses the real perk, secondary and team-vote stages, followed by the entry weapon choice. Older opt-in Editor combat fixtures retain their isolated endless-wave scenario.

Clear **six reinforced encounters per regular arena**. Each encounter has four groups with a bounded live-enemy limit. More enemies enter every fourteen seconds while space is available; clearing the current group brings the next group in after two seconds. The HUD shows `FIGHT current/total / LIVE active +pending`. Clearing the visible enemies cannot grant the encounter-clear reward or advance the route while reinforcements remain. There is no compulsory arena-duration timer.

After Iron Sun, everyone votes for Brassfall Foundry or Mirror Delta with **1/A** or **2/X**. Majority wins; equal votes or abstention use the run seed. Fifteen seconds is the deadline. Disconnects remove departing voters. After the next arena, choose the remaining branch or go to Last Star. A run visits three or four unique prototypes and ends after **15 or 21 encounters**. Roughly 4–6 minutes per regular arena is the current tuning proposal, pending fresh human feedback. This is not measured 45–60-minute final-run pacing.

Mirror Delta has walkable side terraces and three beam relays. Approach within 280 cm and press **H/D-pad left** to rotate a dial to the displayed target. Secured relays latch. Complete six encounters and all three relays to proceed. Hostile beam hazards and the finished glass environment remain open.

Brassfall retains the optional furnace anchor resupply and damage reward. Its prototype entrance refill is granted once. Foundry encounters also mix in turret enemies.

Last Star has two approach encounters, then a champion encounter with reinforcements. Its three lens controls can be shut down with **H/D-pad left**. Active lenses reduce champion damage by 75%; shutdown removes the guard. The champion accelerates below half health and uses existing telegraphed brute attacks. The route changes its movement profile. Clear every encounter, the champion and the lenses for one terminal victory. The existing brute silhouette remains prototype content.

Difficulty snapshots **connected players, including downed teammates**, at each encounter start. Losing health or being carried does not lower the difficulty. A departed player is removed from the following encounter's calculation.

| Players | Early group size | Live cap | Enemy HP multiplier | Early damage multiplier | Champion HP |
| --- | ---: | ---: | ---: | ---: | ---: |
| 1 | 6 | 12 | 1.100 | 2.0 | 5,000 |
| 2 | 9 | 16 | 1.298 | 2.2 | 8,750 |
| 3 | 11 | 20 | 1.496 | 2.4 | 12,500 |
| 4 | 14 | 24 | 1.694 | 2.6 | 16,250 |

The multipliers apply to role/progression base values. Group size rises modestly in later encounters. Damage also rises 10% per subsequent visited arena, reaching 2.6× solo or 3.38× for four players on the fourth arena. Four groups give budgets of 24–32 enemies per solo encounter and 56–72 for four players. Intermediate encounters grant 20 health and 72 reserve ammo; clearing an arena grants the separate recovery reward. These are tuning values, not human difficulty acceptance.

Physical weapon recoil raises the owner's aim independently of inverted mouse/controller look scales. Stable Grip and universal bracing still reduce recoil. Normal camera limits and menu input suppression remain in effect.

Arena clearance and XP are granted once. Cleared arenas cannot be revisited for farming. A route locks after voting, but travel waits for the recovery tube to empty and carried teammates to be set down. Arrival preserves pawns/loadouts, uses checked walkable ground and resets aim pitch. Team wipe cancels reinforcement scheduling; hosted restart retains the party and personal XP. Unattended fixtures do not write persistent profiles.

Ten finished arenas, survival, saved host checkpoints, Steam/Internet, disconnect AI/reclaim, production art/audio, minimum-PC performance and packaged human co-op acceptance remain separate work.

## Reproduce evidence

```powershell
powershell -NoProfile -File Tools/Invoke-RunChecks.ps1 -Rules
powershell -NoProfile -File Tools/Invoke-RunChecks.ps1 -PacingRules
powershell -NoProfile -File Tools/Invoke-RunChecks.ps1
powershell -NoProfile -File Tools/Invoke-RunNetworkChecks.ps1
powershell -NoProfile -File Tools/Invoke-RunNetworkChecks.ps1 -Disconnect -Port 7823
powershell -NoProfile -File Tools/Invoke-LobbyNetworkChecks.ps1 -Peers 2 -HostedRestart -VerifyLateJoin
```

The route property check covers 5,000 seeds and four vote patterns. Encounter rules check all one-to-four-player plans, finite values, monotonic scaling and bounds. Integrated fixtures freeze and defeat actual NPCs, exercise normal setup, real trace damage, reinforcement caps, objectives and rewards, and teleport only fixture players near controls. Network checks use actual local Editor processes. Hosted restart checks actual owned firing recoil, movement, reload, retained XP and fresh run state. Controlled combat and synthetic key events are structural checks, not normal-input packaged human playtests, measured pacing, Internet checks or performance results.
