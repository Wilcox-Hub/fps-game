# Prototype progression run

The normal game now has a finite route. Opening setup still uses the real 20-second perk, 10-second secondary and 10-second team-vote stages, followed by the entry weapon choice. This replaces endless waves in ordinary play. Older opt-in Editor combat fixtures retain their isolated endless-wave scenario.

Clear two waves in Iron Sun. Everyone can vote for Brassfall Foundry or Mirror Delta with **1/A** or **2/X**. The majority wins; equal votes or complete abstention use the run seed. Fifteen seconds is the deadline. Disconnects remove departing voters. After the next arena, choose the remaining branch or go to Last Star. A run visits three or four unique prototypes and ends with five or seven waves; this is not yet the intended 45–60-minute progression pacing.

Mirror Delta has walkable side terraces and three beam relays. Approach a relay within 280 cm and press **H/D-pad left** to rotate its numbered dial to the displayed target. Secured relays latch, so another player cannot undo them. Clear both waves and all three relays to proceed. These are prototype relay puzzles; hostile beam hazards and the finished glass environment remain open.

Brassfall retains the optional furnace anchor resupply and damage reward. Skipping it does not stop the run. Its prototype entrance ammo refill is granted once.

Last Star has three lens controls. Approach each and press **H/D-pad left** to shut it down. Active lenses reduce champion damage by 75%; shutdown removes that guard. The champion accelerates below half health, with existing telegraphed brute attacks. The route changes its movement profile. Clear the champion, escorts and lenses to receive one terminal team victory. This is a prototype encounter using the existing brute silhouette, rather than a finished broadcast champion with original animation and equipment.

Arena clearance and XP rewards are granted once. Cleared arenas stay recorded in the current run and cannot be revisited for farming. A route locks immediately after the vote, but travel waits for the recovery tube to empty and for carried teammates to be set down. Arrival preserves player pawns and loadouts, places players on checked walkable ground and resets the previous aim pitch. Unattended runs do not write persistent profiles.

The ten finished arenas, 20-minute survival mode, saved party checkpoints, Steam lobby, disconnect AI/reclaim, production assets/audio and human balance acceptance remain separate work. A prototype success does not close those issues.

## Reproduce evidence

From this project:

```powershell
powershell -NoProfile -File Tools/Invoke-RunChecks.ps1 -Rules
powershell -NoProfile -File Tools/Invoke-RunChecks.ps1
powershell -NoProfile -File Tools/Invoke-RunChecks.ps1 -Render
powershell -NoProfile -File Tools/Invoke-RunNetworkChecks.ps1
powershell -NoProfile -File Tools/Invoke-RunNetworkChecks.ps1 -Disconnect -Port 7823
```

The rules test explores 5,000 seeds with four vote patterns. The integrated test uses actual game worlds, normal setup timeouts, input bindings, objectives and NPC death/reward handling. It deliberately freezes and kills enemies and teleports the fixture player near controls to verify the run structure quickly. Render captures use constructed camera positions. Network checks use four local Editor processes and reliable owner RPCs; the disconnect variant removes one peer and requires the remaining three to finish. These are not normal-input packaged human playtests, Internet checks, balance measurements or minimum-PC performance results.
