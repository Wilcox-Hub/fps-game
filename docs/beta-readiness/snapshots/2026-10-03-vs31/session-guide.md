# Party lobby and session flow (VS31)

Use `Play-IRON-SUN.bat` to open the newest complete Windows Shipping prototype at the play menu. Direct Editor map launch still starts the existing solo setup; press **M** to open the session browser. **Menu** on a gamepad opens the browser during play; **Select** retains the existing arsenal interaction.

- **H / X:** host a LAN party. Left/right or D-pad chooses two, three or four slots.
- **F / Y:** refresh the LAN party list. Up/down selects a result; **Enter / A** joins it.
- **S / RB:** start a fresh solo run from the menu. Solo browsing pauses the current solo world.
- In the party lobby, **R / A** readies or unreadies you. Every connected player must ready; at least two players are required. Any membership change clears readiness and invalidates old ready requests.
- The host uses **Enter / Menu** to launch. The party starts the existing 20-second personal-perk, 10-second secondary and 10-second team-vote stages together, followed by the 12-second entry-weapon fallback. Combat stays held while the lobby is waiting.
- **L** leaves a party; a host ends it for everyone. **B / Esc** leaves from the lobby, or closes the browser during play. Each peer returns to a fresh menu world. The host can create another party.

Only matching IRON SUN lobby protocol results appear. Launch closes advertising and rejects new joins; reconnect/reclaim is a separate unfinished feature. Steam invites, friend-private sessions, public queue and distributed Internet/device acceptance remain open. Selecting an Internet session through the subsystem while the configured provider is Null reports that a real online provider is required.

Session operations have deadlines and clear their delegate handles. Successful join means the client has reached replicated lobby state, rather than merely receiving the provider callback. Network/travel failure closes local session state and returns to the menu. An abruptly killed host currently takes the engine's **60-second connection timeout** to be detected; this is not instant recovery or host migration. Provider cleanup failure is reported and local named state removed; provider timeout/cleanup failures have not all been fault-injected.

Controlled checks: `Tools/Invoke-LobbyNetworkChecks.ps1 -Peers 2 -Render`, `-Peers 4`, and `-Peers 4 -HostLoss`. They launch actual local Editor processes and use LAN discovery, owned input/RPCs and the normal setup clocks. The ordinary lifecycle checks launch twice, include unready and stale-request rejection, exercise leave/end/rehost, and require one possessed player pawn in each returned menu. The host-loss check kills only its own test host and requires all three remaining peers to clean up. These checks do not replace normal-input packaged human playtests.

The rendered 1280x720 lobby is inspected. Other resolutions, menu navigation with physical controllers, session-provider stalls, late-join races, live movement after reopening menus and hosted-run wipe/restart need further targeted acceptance. Existing direct-listen/solo routes remain supported; a fresh integrated route regression passes.
