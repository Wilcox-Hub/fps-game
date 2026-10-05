# Economy integration checks

The first rendered run reached the comparison but requested a default screenshot filename instead of the fixture's expected custom path. Correcting `FScreenshotRequest` to include the UI and honor that path removed the fixture timeout.

A stronger real-shot check subsequently reported no Through-shot damage to a second target. Three causes were considered: alignment relative to the actual muzzle ray, upgrade state and trace behavior. A targeted trace probe showed the equipped upgrade was correct, and the actual ray converged from the offset muzzle through the first visible body. The second body had been positioned on the camera axis, outside that extended ray. Aligning the fixture with the actual muzzle ray produced 26.775 HP of second-target damage, matching the model; the front burst dealt 115.668 HP. Production penetration was not changed to redirect shots onto the camera axis.

The next cover check revealed a separate fixture construction error: `SetStaticMesh` was attempted on a static component after spawning it at runtime. The engine rejected that operation. Setting the fixture component movable before assigning its mesh created the intended blocker. The final regression includes an independent ray confirming the cover exists, a positive open-lane damage control and a covered shot that still damages the front body while protecting the rear body. Temporary ray instrumentation was removed.

Older loadout/reliability fixtures acquired unowned weapons through the old free-selection path. They now explicitly receive controlled test funding and buy the weapon at a real arsenal through the production transaction. Their input, ammunition and selection-retention assertions remain. Browsing a wildcard is described as browsing until confirmation, rather than asserting that an unchanged secondary selection is a new purchase.

These are fixture corrections and controlled correctness evidence. They do not establish natural cash balance, normal-play aim feel or human fun.

The first accelerated full-route invocation set `slomo 20` at process startup. The log shows wave 1 began and the unattended player was defeated before the automation test started; its terminal victory assertions correctly failed. Moving acceleration into the opt-in test's Update loop preserves normal setup until automation owns the world. It changes only the development-test fixture and leaves gameplay clocks untouched. The final route verdict comes from its assertions/report, not an unconditional PASS log label.
