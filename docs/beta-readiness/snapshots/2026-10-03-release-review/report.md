# Developer release review — October 3, 2026

Decision: **approve supervised internal playtesting; do not approve a beta-readiness release claim.** The current implementation is a playable prototype suitable for a controlled alpha playtest. A complete packaged session with real keyboard/controller input is still the first required gate before distributing to a wider closed group.

The saved 50% is a low-confidence gameplay-development planning estimate, with a judgment range of 35–60%. It is not a measurement of overall beta completion or a release qualification. Overall beta readiness remains unmeasured and the verdict remains NOT_BETA_READY. No replacement percentage is invented by this review.

## Evidence reviewed and reused

Reviewed [VS29 results](../2026-10-02-vs29/test-results.json), [report](../2026-10-02-vs29/report.md), [scores](../2026-10-02-vs29/scores.json) and package startup evidence. The input checker on October 3 matched all 534 recorded Source/Config/Content inputs and the project descriptor. No Unreal gameplay tests were repeated for this review. The underlying test date remains October 2; it is not relabeled as fresh testing.

Controls, personal perks, setup timers, weapon selection/reservation, XP, voting, recovery and local replication have useful controlled evidence. Four real local Editor processes exercised setup, ballots and recovery. However, recovery used constructed positioning and disabled enemies; older combat tests use a short setup fixture. These passes cannot establish normal-input playability, rescue difficulty, balance or fun. The Shipping package built and survived about 22 seconds of offscreen startup, with no accepted full human-played packaged run.

## Testing approval

- **Supervised internal Windows playtest:** approved to begin on the development/test PC. Play the actual Shipping package and record problems.
- **Small closed external playtest:** conditional on a full packaged human session passing without crashes, lost controls, progression blocks or unreadable essential feedback. Start with roughly 5–10 invited players and label it an alpha playtest with placeholder art.
- **Two/four-human co-op:** stage through separate-machine packaged LAN checks, then real Internet sessions after the connection/session path is ready. Four local automated processes are not evidence of cross-machine Internet readiness.
- **Broad public beta:** not approved. Ten finished arenas, routes/champions, online session/invite/reconnect handling, production presentation, minimum-PC performance and human fun/readability evidence remain open.

An initial acceptance target is a 30-minute normal-input packaged session covering setup, fighting, weapon switching/reloading, wave transition, defeat and restart, plus save persistence where supported. Then observe at least five first-time players without coaching and run complete two/four-human sessions. Record crashes, frame-time problems, confusion, rescue pacing and whether players voluntarily want another run. These are proposed release gates, not passed results.

## Where to distribute a controlled build

For the first invited group, a restricted itch.io page can grant access using download keys or a password and stay out of public discovery. This is a distribution recommendation, not a claim that a page/account is configured. [Official itch.io access-control documentation](https://itch.io/docs/creators/access-control).

For a later, larger testing group, Steam Playtest supports limited signups, admitting players in batches, and a separate playtest app associated with the main game. It still requires Steamworks setup/build distribution; it does not implement the game's multiplayer session layer. [Official Steam Playtest documentation](https://partner.steamgames.com/doc/features/playtest).

Neither service has been configured or published by this review. Game implementation remains local; only the readiness assessment is published to the existing GitHub readiness folder.
