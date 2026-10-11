# VS43 — Phasma cover fix and cached-evidence correction

Both required VS42 findings are resolved locally. Phasma excludes only its directly hit sentry from splash; struck environment actors and all their cover components remain visibility blockers.

The regression fires actual Phasma shots through production Blueprint actors. Before the fix, a protected sentry 215 units from a wall impact took 39.946 damage despite two wall panels sharing the struck owner. After the fix it takes zero; the exposed same-side control still takes 35.188, and feedback counts one recipient. Existing visible-body, direct-hit, full-health cluster (one death), falloff, range and other cover controls also pass. Red and green logs are retained in [regression history](regression-history.json).

Final Editor build and a fresh Windows Shipping Prototype build/cook/stage/archive pass. The package was not launched. The combat check is standalone Editor with NullRHI, frozen/repositioned production sentries and actual firing APIs; it establishes collision/damage correctness, not rendered, human, Internet or packaged multiplayer acceptance. See [test results](test-results.json) and [package artifacts](package-artifacts.json).

VS41's cached loadout record now describes only the bound-key interactions it actually checked. Phasma was bought by a direct fixture API call; Enter-key purchasing and wildcard keep confirmation were not tested. The original scope is preserved in the dated correction annotation, and original test dates, logs, images and pass status remain unchanged. No loadout rerun or current-source blanket pass is claimed. [Correction audit](scope-correction.json).

Two game inputs changed, with all 560 current Source/Config/Content hashes saved. The October 4 VS41 69% estimate (68.5 weighted, low confidence) remains dated planning history; this fix awards no new points. The 75% goal, wider UI readability, human fun/balance/pacing, finished content and friend co-op acceptance remain open. NOT_BETA_READY; overall beta completion remains unmeasured.

Game source and readiness records are local/unpublished. No commit, push or upload was attempted.
