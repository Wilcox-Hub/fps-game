# Combat, movement and controls

Generated from [current.json](../current.json). Read current rules before historical interviews.

## combat-pressure — Pressure-inducing fights

**APPROVED** · Recorded 2026-10-18

Favor temporary holds and deliberate movement between cover. Mostly short enemy bursts mix with longer pressure and telegraphed heavy attacks. Limited aim correction, complementary roles and bounded coordinated flanks let movement matter. Strategic roles vary by arena and between runs; avoid a rigid predictable script.

Sources: [owner-pressure-wording](../../../docs/design-decisions/sources.md#owner-pressure-wording), [2026-10-08-combat-movement-q2](../../../docs/design-decisions/interviews/2026-10-08-combat-movement.md#2026-10-08-combat-movement-q2), [2026-10-08-combat-movement-q8](../../../docs/design-decisions/interviews/2026-10-08-combat-movement.md#2026-10-08-combat-movement-q8), [2026-10-08-combat-movement-q12](../../../docs/design-decisions/interviews/2026-10-08-combat-movement.md#2026-10-08-combat-movement-q12), [2026-10-08-combat-movement-q13](../../../docs/design-decisions/interviews/2026-10-08-combat-movement.md#2026-10-08-combat-movement-q13), [2026-10-08-combat-movement-q16](../../../docs/design-decisions/interviews/2026-10-08-combat-movement.md#2026-10-08-combat-movement-q16), [2026-10-08-combat-movement-q17](../../../docs/design-decisions/interviews/2026-10-08-combat-movement.md#2026-10-08-combat-movement-q17).

## combat-slide — Hold crouch and spend momentum

**APPROVED** · Recorded 2026-10-18

Slide from sprint momentum, with small steering adjustments and reduced shooting accuracy. Permit hipfire and ADS, with hipfire more practical. Hold to remain crouched; release stands when clear, and sprint stands when clear. Rebuild momentum before repeating. A roll/dash remains deferred until sliding is tested.

Sources: [2026-10-08-combat-movement-q6](../../../docs/design-decisions/interviews/2026-10-08-combat-movement.md#2026-10-08-combat-movement-q6), [2026-10-08-combat-movement-q9](../../../docs/design-decisions/interviews/2026-10-08-combat-movement.md#2026-10-08-combat-movement-q9), [2026-10-08-combat-movement-q10](../../../docs/design-decisions/interviews/2026-10-08-combat-movement.md#2026-10-08-combat-movement-q10), [2026-10-08-combat-movement-q11](../../../docs/design-decisions/interviews/2026-10-08-combat-movement.md#2026-10-08-combat-movement-q11), [2026-10-08-combat-movement-q14](../../../docs/design-decisions/interviews/2026-10-08-combat-movement.md#2026-10-08-combat-movement-q14), [2026-10-08-combat-movement-q21](../../../docs/design-decisions/interviews/2026-10-08-combat-movement.md#2026-10-08-combat-movement-q21).

## combat-recoil — Learnable recoil with bounded tracking relief

**APPROVED** · Recorded 2026-10-18

Manageable opening shots grow into more demanding climb/drift; stronger heavy-gun and sustained rapid-fire recoil. Related gun families share learnable patterns without becoming identical. Use visible backward/upward gun kick with little extra camera shake; recovery respects manual compensation; ADS slightly calms recoil. Only modest capped tracking relief borrows the Apex characteristic; build the rest as IRON SUN's own foundation.

Sources: [2026-10-08-combat-movement-q22](../../../docs/design-decisions/interviews/2026-10-08-combat-movement.md#2026-10-08-combat-movement-q22), [2026-10-14-recoil-q1](../../../docs/design-decisions/interviews/2026-10-14-recoil.md#2026-10-14-recoil-q1), [2026-10-14-recoil-q2](../../../docs/design-decisions/interviews/2026-10-14-recoil.md#2026-10-14-recoil-q2), [2026-10-14-recoil-q3](../../../docs/design-decisions/interviews/2026-10-14-recoil.md#2026-10-14-recoil-q3), [2026-10-14-recoil-q4](../../../docs/design-decisions/interviews/2026-10-14-recoil.md#2026-10-14-recoil-q4), [2026-10-14-recoil-q5](../../../docs/design-decisions/interviews/2026-10-14-recoil.md#2026-10-14-recoil-q5).

## controls-lean-enabled — Historical lean implementation

**SUPERSEDED** · Recorded 2026-10-11

The October 11 interview approved manual Q/E lean, hold default and a settings toggle. This is historical, replaced by removal.

Replaced by: **controls-lean-removed**.

Sources: [2026-10-11-lean-ion-q1](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q1), [2026-10-11-lean-ion-q4](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q4), [2026-10-11-lean-ion-q5](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q5).

## controls-lean-removed — Lean is removed for now

**APPROVED** · Recorded 2026-10-18

Remove lean behavior and its settings entry. Historical lean answers do not authorize reintroducing it.

Replaces: controls-lean-enabled.

Sources: [owner-lean-removal](../../../docs/design-decisions/sources.md#owner-lean-removal).

## controls-old-rescue — Historical T rescue binding

**SUPERSEDED** · Recorded 2026-10-11

The lean/Ion interview chose V melee, F healing and T contextual rescue/arsenal. World interaction has since moved to E.

Replaced by: **controls-e-interact**.

Sources: [2026-10-11-lean-ion-q28](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q28).

## controls-e-interact — E standard interaction and held rescue pickup

**APPROVED** · Recorded 2026-10-18

E is standard world interaction. Hold E near a downed teammate to pick them up; a short cancelled hold must do nothing. Releasing after pickup keeps carrying; a fresh E press sets down or deposits at medical equipment. F heals; V is melee, with M a current fallback. Keep prompts and input context consistent across racks, arsenals, upgrades and objectives.

Replaces: controls-old-rescue.

Sources: [owner-e-interact](../../../docs/design-decisions/sources.md#owner-e-interact), [current-work-record](../../../docs/design-decisions/sources.md#current-work-record).

## controls-hold-duration — Initial rescue hold duration

**PROVISIONAL** · Recorded 2026-10-18

The current local implementation uses 0.65 seconds with progress feedback and range/target cancellation. This is an engineering tuning choice, not a user-selected or human-accepted duration.

Sources: [current-work-record](../../../docs/design-decisions/sources.md#current-work-record).

## combat-ion — Ion is a weapon identity

**APPROVED** · Recorded 2026-10-18

Ion is a semi-automatic arsenal gun, not a universal player power. Normal direct successful hits build one stored charge based on roughly 3–4 original magazines; misses do not. A deliberate alternate attack has a short cancellable windup, one-round cost and small aimed impact disruption pulse. Preserve gun charge across reloads, holstering, upgrades and Continue. Signal ready charge audibly and visually; heavier bots resist interruption.

Sources: [2026-10-11-lean-ion-q9](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q9), [2026-10-11-lean-ion-q12](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q12), [2026-10-11-lean-ion-q13](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q13), [2026-10-11-lean-ion-q14](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q14), [2026-10-11-lean-ion-q15](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q15), [2026-10-11-lean-ion-q16](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q16), [2026-10-11-lean-ion-q18](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q18), [2026-10-11-lean-ion-q22](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q22), [2026-10-11-lean-ion-q23](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q23), [2026-10-11-lean-ion-q34](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q34), [2026-10-11-lean-ion-q35](../../../docs/design-decisions/interviews/2026-10-11-lean-ion.md#2026-10-11-lean-ion-q35).

## combat-shield — Active shield remains unresolved

**OPEN** · Recorded 2026-10-18

The owner requests an active shield tactical. Its form and exact behavior are not yet approved; a passive team Shield bonus is not proof of an active tactical. Resolve the shield interview frontier before implementation.

Sources: [owner-lean-removal](../../../docs/design-decisions/sources.md#owner-lean-removal), [2026-10-14-shield-q1](../../../docs/design-decisions/interviews/2026-10-14-shield.md#2026-10-14-shield-q1).
