# VS42 — Review of recent combat and arsenal work

**Verdict: changes requested before treating the work as complete.** The Phasma diagnosis and targeted tests are useful, and the arsenal no-change state is a valid small improvement. The new splash path introduces a cover-occlusion regression that the saved tests do not exercise.

Reviewed October 5, 2026. The fixed comparison is the VS39 saved input hashes against the VS41 saved hashes and matching live source: six changed files, 260 diff lines. This is a scoped working-change review, not a whole-game audit. No gameplay code, engine tests or builds were changed/run during this review. Current input checking passes 560/560, and 26 referenced logs, images, package metadata and archive artifacts match their saved hashes. Recorded passes remain valid only at their original date and scope.

## Standards

One actionable correctness finding: **[P2] Phasma splash bypasses the cover actor it hits.** `ShooterCharacter.cpp:1355` adds every direct-hit actor to `IgnoredActors`, including walls and arena geometry. The new visibility query then ignores that entire actor at line 1689. Shooting a divider can therefore damage a sentry behind it within the splash radius; a single actor containing several cover components has all those components bypassed. Limit direct-recipient exclusion to the directly hit sentry and keep world geometry in the obstruction query. Add a regression that shoots the divider itself.

The existing cover control aims at a sentry's shoulder with a separate divider, so its recorded pass cannot catch this case. VS40's cover claim should retain that scope. Its relevant combat log records `-NullRHI`; it is headless collision/damage evidence.

No hard AGENTS violation was found in the six changed source files. The snapshots, latest pointer and overview were updated, and package/human acceptance limitations remain explicit. Six baseline and six current review copies match their declared hashes; current copies match live source.

Optional heuristic suggestion: the newly added camera-to-muzzle probes in `RidgefireCombatReliabilityTests.cpp:403` and line 424 duplicate the calculation. A test-local helper could reduce drift; this is a maintainability suggestion, not a documented standards violation.

## Spec

One functional regression: **[P1] A Phasma impact on cover disables that cover's splash occlusion.** `ShooterCharacter.cpp:1355` adds every directly struck actor to `IgnoredActors`, including walls. The falloff pass puts those actors in its visibility-query exclusions at lines 1685-1694. Shooting solid cover can damage sentries behind it within 460 units. Production cover meshes are components of `ARidgefireArenaDressing` (`RidgefireArenaDressing.cpp:577`, `BlockAll` at line 590), so striking one mesh excludes every blocking mesh owned by that actor. This contradicts VS40's stated behavior, "with a visibility trace preserving cover," and the approved direction, "Preserve deliberate combat decisions."

Restrict the direct-recipient exclusion to the struck sentry; keep struck environment geometry eligible to block splash. Add an impact-on-cover negative control. The existing cover test aims at a sentry and does not exercise this case.

No other functional defect or scope creep was found in the changed hunks. Phasma's direct-only comparison matches the revised direct damage; upgrades still increase direct damage, cadence and magazine size. Exact comparisons remain accessible through details and upgrade preview.

Human readability and combat acceptance remain partial. The spec requires rendered comparison review and human feedback; VS41 correctly keeps those gates open. Its 69% estimate does not establish beta acceptance.

## Evidence and completion audit

The VS41 loadout scope lists "purchase/keep" alongside the bound keyboard interactions. The fixture actually buys Phasma through a direct `Mode->PurchaseWeapon` call (`RidgefireLoadoutInteractionTests.cpp:138`) and exercises slot/menu/fire key delegates; it never presses Enter to purchase or confirm keeping a wildcard. Narrow that saved claim or add the missing input-path validation before using it to skip a purchase-confirmation check. This does not invalidate the genuine slot-selection/fire/secondary-return pass.

The wider arsenal readability request remains open. VS41 hides repeated stats for a weapon already in its selected slot; font scale, real different-weapon comparisons and upgrade-view layout are unchanged. The current screenshot also contains an engine shader notice and transient receipt, limitations already recorded by VS41.

The 69% figure is preserved as VS41's October 4 low-confidence planning judgment, not independently validated completion. This review awards no points and does not approve the one-point increment as a measured milestone. Beta approval remains false; the 75% target is still open.

The relevant game work and beta-readiness folder remain local/unpublished, with new files untracked. The earlier GitHub upload block remains unresolved; this review makes no publishing attempt.

## Required follow-up

1. Keep struck world geometry in Phasma's splash obstruction query, exclude only the directly hit sentry from splash damage, and test an explosion that impacts the cover itself.
2. Narrow the cached loadout purchase/keep scope or verify those actual keyboard confirmation paths.
3. Review real purchase/upgrade comparison states and obtain owner usability acceptance.

See [review results](review-results.json), [pinned source scope](scope.json), [diff](changes.diff), [input check](input-check.json) and [saved-evidence verification](evidence-verification.json).

Standards: one correctness finding and one optional heuristic; worst issue is cover bypass. Spec: one functional finding; worst issue is cover bypass. Separate evidence audit: one overbroad input-validation claim.
