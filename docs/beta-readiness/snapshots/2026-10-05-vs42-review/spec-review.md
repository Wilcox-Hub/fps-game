## Spec

One functional regression: **[P1] A Phasma impact on cover disables that cover's splash occlusion.** `ShooterCharacter.cpp:1355` adds every directly struck actor to `IgnoredActors`, including walls. The falloff pass puts those actors in its visibility-query exclusions at lines 1685-1694. Shooting solid cover can damage sentries behind it within 460 units. Production cover meshes are components of `ARidgefireArenaDressing` (`RidgefireArenaDressing.cpp:577`, `BlockAll` at line 590), so striking one mesh excludes every blocking mesh owned by that actor. This contradicts VS40's stated behavior, "with a visibility trace preserving cover," and the approved direction, "Preserve deliberate combat decisions."

Restrict the direct-recipient exclusion to the struck sentry; keep struck environment geometry eligible to block splash. Add an impact-on-cover negative control. The existing cover test aims at a sentry and does not exercise this case.

No other functional defect or scope creep was found in the changed hunks. Phasma's direct-only comparison matches the revised direct damage; upgrades still increase direct damage, cadence and magazine size. Exact comparisons remain accessible through details and upgrade preview.

Human readability and combat acceptance remain partial. The spec requires rendered comparison review and human feedback; VS41 correctly keeps those gates open. Its 69% estimate does not establish beta acceptance.
