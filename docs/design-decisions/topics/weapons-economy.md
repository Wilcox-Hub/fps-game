# Two-slot weapons, shards and arsenal

Generated from [current.json](../current.json). Read current rules before historical interviews.

## weapons-two-slots — Exactly two carried weapons

**APPROVED** · Recorded 2026-10-18

Keys 1 and 2 select the two usable carried guns. A third carried slot/unlock is outside the core approved design. Empty slots fill in order; with both full, accepting a replacement changes the currently held slot. Racks, inventory displays, saves and co-op must agree on actual owned usable weapons.

Sources: [owner-two-slots](../../../docs/design-decisions/sources.md#owner-two-slots), [owner-slot-pickup](../../../docs/design-decisions/sources.md#owner-slot-pickup).

## economy-shards — Shards and acquisition choices

**APPROVED** · Recorded 2026-10-18

Use shards. Known weapon purchases are the dependable main purchase lane; preserve the random wildcard reroll instead of a duplicate mystery box. Encounter and optional-objective cash-equivalent shard rewards are shared; difficult melee-finisher bonus rewards belong to the attacker. Preserve clarity about individual cost/ownership.

Sources: [owner-shards](../../../docs/design-decisions/sources.md#owner-shards), [owner-arena-identity](../../../docs/design-decisions/sources.md#owner-arena-identity), [2026-10-11-lean-ion-q29](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q29), [2026-10-11-lean-ion-q30](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q30).

## economy-short-break — Historical short regroup

**SUPERSEDED** · Recorded 2026-10-18

Earlier direction allowed roughly 15–25 seconds between encounters. That was later replaced by the selected 90-second break.

Replaced by: **economy-regroup90**.

Sources: [owner-arena-identity](../../../docs/design-decisions/sources.md#owner-arena-identity).

## economy-regroup90 — 90-second regroup and early team start

**APPROVED** · Recorded 2026-10-18

Use 90-second breaks between encounters with an early team start. Initial run preparation has no forced countdown. Paid known weapons, random rerolls and upgrades remain available mid-combat through the arsenal; interaction supplies no safety pause.

Replaces: economy-short-break.

Sources: [2026-10-08-combat-movement-choice24](../../../docs/design-decisions/interviews/2026-10-08-combat-movement.md#2026-10-08-combat-movement-choice24), [owner-midround-arsenal](../../../docs/design-decisions/sources.md#owner-midround-arsenal), [2026-10-14-arena-arrival-q13](../../../docs/design-decisions/interviews/2026-10-14-arena-arrival.md#2026-10-14-arena-arrival-q13).

## weapons-rack — Crosshair selection and optional detail

**APPROVED** · Recorded 2026-10-18

Approach a physical gun rack, aim at a slightly highlighted weapon, inspect it and select it. Keep a compact readable comparison and make detailed statistics optional. Actual held/displayed models should match and remain clear of walls. Preserve movement and reliable one-action input.

Sources: [owner-rack-focus](../../../docs/design-decisions/sources.md#owner-rack-focus), [2026-10-14-arena-arrival-q11](../../../docs/design-decisions/interviews/2026-10-14-arena-arrival.md#2026-10-14-arena-arrival-q11).

## weapons-optics — Phasma holo and Gale 2x

**APPROVED** · Recorded 2026-10-18

Fit Phasma with a 1x holo and Gale with a 2x optic. ADS must align the reticle and actual camera aim with the shot trace; spread, parallax/near cover and recoil still require explicit scoped validation.

Sources: [owner-optics](../../../docs/design-decisions/sources.md#owner-optics), [current-evidence](../../../docs/design-decisions/sources.md#current-evidence).
