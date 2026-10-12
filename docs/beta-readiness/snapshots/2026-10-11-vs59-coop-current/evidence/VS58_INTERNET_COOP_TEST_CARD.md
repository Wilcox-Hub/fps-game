# VS58 private internet co-op check

The Lenovo server is running at `cairo-ferry.tun.ply.gg:42527`. Everyone needs **PLAYTEST / VS58**. Owner launcher is updated; install/extract the full new ZIP on the other PC. Use the private invite card at `%LOCALAPPDATA%/IRON SUN/Private-Server.txt`; share it privately with your squad.

1. Use two PCs on different internet connections for this check, such as one home Wi-Fi and one phone hotspot. Choose **JOIN SERVER**, enter address/invite, ready both players, then have the leader start.
2. Finish loadout selection. Fire both weapons, switch with **1** and **2**, and confirm shooting keeps the selected gun equipped. Check sprint, momentum slide, aiming and hits on moving bots. Lean is not implemented in VS58 and should not be scored as a working function.
3. Clear an encounter. At regroup, note each player's guns, ammo, shards and upgrades. Both leave normally. Close/reopen both games, preserving their saved player identity. Reconnect the same squad and continue; compare resources/loadouts.
4. Record join failures, ping/visible lag, missing geometry, automatic gun changes or differences in saved resources. Confirm which PC/weapon and the menu build label; a short clip helps reproduce failures.

Server restart recovery is a separate check: once the squad has saved at regroup and left, restart only the managed IRON SUN game, then reconnect the original squad and continue. Automated cold lobby boot/join is already checked; saved-run recovery on Linux is not yet accepted.

This is supervised internal alpha. Current automation covers native boot and two packaged lobby clients through Playit on the owner home network, plus wrong-invite rejection. It does not substitute for this different-Wi-Fi human combat/recovery session.
