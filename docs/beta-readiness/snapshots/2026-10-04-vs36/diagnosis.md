# Reproductions and diagnosis

The active diagnosing-bugs workflow used actual-game feedback loops before production theories and changes. Commands run from `unreal/Ridgefire` were `Tools/Invoke-CombatReliabilityChecks.ps1`, then the focused `-Filter Ridgefire.Regressions.Input.SelectedPrimaryThroughInputProcessing` and `-Filter Ridgefire.Regressions.Input.SprintRetainsSelectedPrimary`. The `-PIE` variant loads the actual map and starts a controlled Editor play world. Engine startup takes roughly 10–18 seconds; normal setup is accelerated only in these new fixtures, then time dilation returns to 1 for the tested actions and encounter boundaries.

## Empty-space hits

The constructed scene minimized the bad shot to one actual Blueprint NPC, one real firing character and one normal Gale shot. Independent visible-mesh intersections were compared with world visibility traces. Two baseline runs returned 35 empty-space hits and identical 42 damage at the chosen point. Ranked, falsifiable candidates were the movement capsule, hidden legacy mesh collision and shot-specific trace/damage widening. Ignoring only the movement capsule reduced invisible trace hits to zero while retaining visible controls; hidden meshes and weapon-specific widening were not needed to explain the symptom.

The production change sets the capsule's visibility response to Ignore when visible armor query parts supply the hit geometry. Physical collision and other channel responses are preserved. The original world/shot loop goes green, including a fixed empty-space control inside the movement capsule, four role/pose scans, and a cover/no-cover differential Phasma check. The old capsule alone cannot fabricate an impact to trigger a blast anymore.

## Unrequested primary switching

Initial bound-key selection plus reinforcement/reward/encounter, firing/reload, pickup overlap and direct combat-action tests passed. A real-shot encounter clear also passed in standalone and PIE. Those results narrowed the missing seam: direct delegate execution bypassed ordinary input processing.

`FInputKeyEventArgs::CreateSimulated` events submitted through the possessed controller's real `InputKey` path produced the exact Phasma-to-Gale symptom after Shift. Removing firing, reload, C and other actions still produced the switch with only a sprint press after real processed slot-2 selection. This minimized loop reproduced again with a tagged log showing the inherited swap callback firing while Phasma was active.

Ranked predictions considered an inherited swap binding sharing sprint, sprint itself resetting the loadout and repeated selection-key events. Direct sprint handlers had retained the weapon; the inherited callback probe identified the first. `DoSwitchWeapon` now returns while the custom arsenal is active, so that template action cannot cycle the IRON SUN primary. The original full input loop and the minimal held-sprint loop pass; actual sprint speed still rises. Explicit numbered primary controls and secondary toggling also pass. Temporary `[DEBUG-vs36]` production logs were removed and checked absent; their historical probe excerpts remain diagnostic evidence.

## Fixture corrections and evidence limits

The first prepared selection fixture indexed an unassigned slot 2 and crashed. The second assumed Phasma was available as primary even though ordinary setup reserved it as the default secondary. The corrected fixture explicitly selects wildcard secondary in the proper setup stage, acquires Phasma through the actual arsenal, presses slot 2 and asserts identity before testing retention. These were fixture errors; they receive no game-fix credit.

A second firing actor initially overlapped the first constructed shooter's lane. Destroying the first actor before the Phasma scene restored a real damage positive control. The final blast layout puts an exposed and a covered neighbour on the shooter-facing side, then removes only the divider to prove that cover caused the zero-damage result. Nearby exposed splash is intended existing behavior, not evidence that every clustered kill was a collision defect.

The pre-existing arsenal fixture had the same reserved-secondary assumption: it recorded Gale's 24/144 ammo, then expected those numbers when Phasma correctly showed 6/30. It now reserves wildcard secondary and asserts Phasma identity before sampling ammo. The corrected rendered test passes. Its changed body is inside `WITH_DEV_AUTOMATION_TESTS`, excluded from Shipping, and the prior package input hash is retained separately.

No changes to enemy health, damage, party scaling, encounter count, map size, lean behavior, art or Steam/server architecture were made. No new human recording, fun rating, minimum-PC performance, Internet, packaged full run or four-human session is claimed. Prior route/perk/recovery evidence remains dated and restricted to relevant unchanged inputs.
