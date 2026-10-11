# VS39 — carried-weapon selection and arsenal readability

**Result:** the arsenal input issue now has a focused live-game regression. While the keyboard-controlled arsenal is open, `1` selects primary slot 1, `2` selects primary slot 2, and `3` selects the carried secondary. Left/right still browse purchase offers. The menu remains open while switching.

The root cause was that keyboard `1–3` were routed to the shop-offer cursor whenever the arsenal was open; the displayed owned count therefore did not correspond to the keys’ action. The screen now lists the exact carried slots, highlights the equipped slot, presents one selected offer at a time, and limits the default comparison to damage per shot, fire rate, magazine and recoil. Detailed party-scaled comparisons stay behind Tab. Keyboard and gamepad hints now follow their separate bindings.

The final `TP_FirstPersonEditor` Development build passes on Unreal Engine 5.8.3. `Ridgefire.Weapons.Loadout.NormalInteractions` passes against the standalone possessed Blueprint and real bound-key delegates. Three rendered 1280×720 frames were captured; the arsenal view was visually inspected and the summary/control rows no longer overlap. See [test results](test-results.json), [visual check](visual-check.json), and [screenshots](visuals/ArsenalOpen.png).

These checks cover an Editor build and a controlled, frozen-sentry interaction fixture. They do not verify physical keyboard/gamepad hardware, Shipping, a packaged run, readability comprehension across users, natural economy balance, human fun, co-op or Internet.

The **65% low-confidence gameplay planning estimate remains the last whole-game assessment from VS38**. VS39 is a narrow UI/input check and earns no whole-game score increase. Overall beta readiness remains unmeasured and the verdict remains **NOT_BETA_READY**.
