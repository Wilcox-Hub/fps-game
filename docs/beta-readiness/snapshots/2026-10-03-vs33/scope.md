# VS33 — pacing, recoil and party difficulty

Player report: requested Editor recording shows complete levels ending too quickly; recoil pulls in the wrong direction; the boss has too little health; overall difficulty is too easy and must scale with player count.

Baseline: all 546 VS32 input hashes match before edits. The first recorded solo run shows champion victory around 3:46 and the second ends in defeat around 7:34. The capture is Editor PIE; exact binary/source correspondence and audio/input latency are not proven. This is now owner-provided playtest feedback, so the earlier statement that the owner cannot human-test is stale.

Scope: fix upward physical recoil independently of inverted look-input scales; replace two-small-group arenas with multiple encounters and bounded reinforcements; raise boss health; scale enemy pressure, HP and damage for one through four connected players, counting downed members. Preserve legacy isolated combat fixtures, route objectives, owned RPCs, grounded perks and intentionally variable wildcard strength.

Working pacing proposal: roughly 4–6 minutes per arena pending the owner's preference. Current routes visit three or four prototypes; this is roughly 12–24 minutes, not proof of the longer 45–60-minute final-run target. Favor actual encounters over forced empty waits. No ten-arena art/content, saved checkpoints, Steam or migration work under this ticket.

Acceptance: red-capable owned-controller recoil regression before the fix; pure monotonic/bounded scaling and finite encounter-plan checks; actual normal-setup solo route with every reinforcement drained, objective and reward guards; local network route with replicated encounter counters and actual scaled NPC health/damage; party restart cancellation of pending reinforcements; Editor/Shipping build and a bounded package startup check. Scripted combat is structural evidence, not measured human difficulty or final pacing acceptance. Keep the readiness estimate conservative and preserve all unrelated edits. Publish only readiness records; source stays local unless explicitly requested.
