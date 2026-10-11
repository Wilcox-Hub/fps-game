## Standards

One actionable correctness finding: **[P2] Phasma splash bypasses the cover actor it hits.** `ShooterCharacter.cpp:1355` adds every direct-hit actor to `IgnoredActors`, including walls and arena geometry. The new visibility query then ignores that entire actor at line 1689. Shooting a divider can therefore damage a sentry behind it within the splash radius; a single actor containing several cover components has all those components bypassed. Limit direct-recipient exclusion to the directly hit sentry and keep world geometry in the obstruction query. Add a regression that shoots the divider itself.

The existing cover control aims at a sentry's shoulder with a separate divider, so its recorded pass cannot catch this case. VS40's cover claim should retain that scope. Its relevant combat log records `-NullRHI`; it is headless collision/damage evidence.

No hard AGENTS violation was found in the six changed source files. The snapshots, latest pointer and overview were updated, and package/human acceptance limitations remain explicit. Six baseline and six current review copies match their declared hashes; current copies match live source.

Optional heuristic suggestion: the newly added camera-to-muzzle probes in `RidgefireCombatReliabilityTests.cpp:403` and line 424 duplicate the calculation. A test-local helper could reduce drift; this is a maintainability suggestion, not a documented standards violation.
