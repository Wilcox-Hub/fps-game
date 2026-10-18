# Wildcard pool, potential and physical upgrades

Generated from [current.json](../current.json). Read current rules before historical interviews.

## wildcards-quality — Weak through jackpot with real tradeoffs

**APPROVED** · Recorded 2026-10-18

Weak guns remain functional. Mid rolls beat base Phasma in their intended role while allowing recoil, range, ammo or handling costs. Rare jackpots can shred ordinary bots; bosses and late waves still matter. Most strong rolls have tradeoffs, with rare excellent all-round exceptions. Fixed upgrade potential can turn a weak start into an exceptional weapon.

Sources: [2026-10-16-wildcard-refits-q1](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q1), [2026-10-16-wildcard-refits-q2](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q2), [2026-10-16-wildcard-refits-q3](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q3), [2026-10-16-wildcard-refits-q4](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q4), [2026-10-16-wildcard-refits-q9](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q9), [2026-10-16-wildcard-refits-q16](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q16).

## wildcards-pool — Families, parts and firing cycles

**APPROVED** · Recorded 2026-10-18

Six normal families: sidearms, compact automatics, rifles, precision rifles, scatterguns and heavy automatics, plus very rare launchers and dedicated melee. Compatible physical frame/core/emitter/feed/grip/cooling/optic parts determine actual mechanics. Include semi-auto, auto, burst, pump/bolt, charge and supported spin-up. Melee occupies one weapon slot; physical strikes work empty and powered attacks consume charge. Finite cells/ammo remains the normal rule.

Sources: [2026-10-16-wildcard-refits-q18](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q18), [2026-10-16-wildcard-refits-q26](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q26), [2026-10-16-wildcard-refits-q27](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q27), [2026-10-16-wildcard-refits-q35](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q35), [2026-10-16-wildcard-refits-q37](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q37).

## wildcards-mechanisms — Eight selected signature mechanisms

**APPROVED** · Recorded 2026-10-18

Use embedded-hit detonation; narrow/wide pattern switching; real bounded surface ricochets; manually guided projectiles for rare heavy launchers; persistent impact fields; continuous heat-managed beams; a held powered cutter; and broad enemy-penetrating shots. Most special guns get one, rare compatible jackpots two. Electrical chaining and remote planted spikes were not selected. Weaknesses are mechanical; remove random self-damage, damage wheels, health stealing and health/sprint/streak damage multipliers. Specific blue-magma hazard damage is an explicitly approved exception.

Sources: [2026-10-16-wildcard-refits-q21](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q21), [2026-10-16-wildcard-refits-q22](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q22), [2026-10-16-wildcard-refits-q24](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q24).

## wildcards-potential — Potential counter final boundary

**APPROVED** · Recorded 2026-10-18

Draw each player's next marked-offer interval with 5% of intervals on rolls 1–3,80% on 4–8 and 15% on 9–10; guarantee by 10. Count each new free/token/shard wildcard offer, not inspecting the same offer or known purchases. Reset when the marked offer appears, even if passed. Preserve the per-player per-run counter across arenas/Continue; reset for a new run. The mark promises an exceptional fixed upgraded form, not immediate jackpot quality.

Sources: [2026-10-16-wildcard-refits-q9](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q9), [2026-10-16-wildcard-refits-q47](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q47), [2026-10-16-wildcard-refits-q48](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q48), [2026-10-16-wildcard-refits-q49](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q49), [2026-10-16-wildcard-refits-q50](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q50), [2026-10-16-wildcard-refits-q51](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q51).

## wildcards-odds — Initial probability tuning

**PROVISIONAL** · Recorded 2026-10-18

Early:20% weak/50% mid/27% good/3% jackpot; deep arenas approach 10/40/42/8. Improve with arena progress, not reroll spam. Launchers and dedicated melee each around 1% of all rolls. Blue magma around 0.25% early,0.5% deep, inside jackpot odds. These are initial tuning targets, not measured distribution results.

Sources: [2026-10-16-wildcard-refits-q7](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q7), [2026-10-16-wildcard-refits-q10](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q10), [2026-10-16-wildcard-refits-q41](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q41), [2026-10-16-wildcard-refits-q42](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q42).

## wildcards-magazines-optics — Normal/extended magazines and varied sights

**APPROVED** · Recorded 2026-10-18

Normal and extended magazines are separate rolls. Good/jackpot quality plus a high or very low family-relative fire rate increases extended-mag likelihood; it is not guaranteed. Usually pay a modest reload/handling cost. Provide multiple fitted optics at 1x/2x/3x/4x where appropriate, including a red upside-down-V chevron with its apex as aim reference. Shared alternate fire keeps RMB for ADS.

Sources: [2026-10-16-wildcard-refits-q28](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q28), [2026-10-16-wildcard-refits-q35](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q35), [2026-10-16-wildcard-refits-q36](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q36), [2026-10-16-wildcard-refits-q40](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q40).

## wildcards-magma — Solar blue-magma jackpot

**APPROVED** · Recorded 2026-10-18

Very rare, very strong short-range blue-glowing magma gun. Hold fire for irregular heavy arcing blobs, roughly 6m useful reach, with left/right variation. Ground deposits last 10 seconds; owner can be hurt. Base protects teammates; the major upgraded form greatly increases damage and also harms teammates. Player hazard damage is reduced but dangerous. Overheat readily. Recharge slowly only in direct sunlight, held or stowed; shade/roofs/sunless arenas block recharge. Ordinary ammo drops do not refill it; paid upgrades retain the refill rule.

Sources: [owner-magma-request](../../../docs/design-decisions/sources.md#owner-magma-request), [2026-10-16-wildcard-refits-q30](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q30), [2026-10-16-wildcard-refits-q31](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q31), [2026-10-16-wildcard-refits-q32](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q32), [2026-10-16-wildcard-refits-q33](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q33), [2026-10-16-wildcard-refits-q34](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q34), [2026-10-16-wildcard-refits-q38](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q38), [2026-10-16-wildcard-refits-q39](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q39).

## upgrades-old-choice — Historical exact-preview upgrade choice

**SUPERSEDED** · Recorded 2026-10-18

Earlier progression proposed exact upgrade previews and choosing a behavior at purchase. Later wildcard/refit answers replace that with a path fixed when rolled and only general hints beforehand.

Replaced by: **upgrades-fixed-path**.

Sources: [owner-arena-identity](../../../docs/design-decisions/sources.md#owner-arena-identity).

## upgrades-fixed-path — Fixed gun-specific transformations

**APPROVED** · Recorded 2026-10-18

All guns can receive a major transformation and only meaningful gun-specific follow-ups. Roll the path with the weapon; preserve its identity. A potential-marked weapon guarantees an exceptional upgraded form. Before paying, show general hints about direction, preserving discovery rather than disclosing exact future stats.

Replaces: upgrades-old-choice.

Sources: [2026-10-16-wildcard-refits-q4](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q4), [2026-10-16-wildcard-refits-q5](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q5), [2026-10-16-wildcard-refits-q6](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q6), [2026-10-16-wildcard-refits-q9](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q9), [2026-10-16-wildcard-refits-q15](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q15).

## upgrades-physical — Walk-up shared physical refit

**APPROVED** · Recorded 2026-10-18

Aim at the machine, see cost and a general upgrade hint, then E inserts the held gun and pays once. Refit about 5 seconds while the other gun remains usable. One machine for the whole squad stays occupied until the owner collects; no deletion timer. Reserve the original slot without creating a third weapon. Refill the magazine and top reserves up to at least 50%, preserving surplus.

Sources: [owner-refit-interaction](../../../docs/design-decisions/sources.md#owner-refit-interaction), [owner-e-interact](../../../docs/design-decisions/sources.md#owner-e-interact), [2026-10-16-wildcard-refits-q11](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q11), [2026-10-16-wildcard-refits-q12](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q12), [2026-10-16-wildcard-refits-q13](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q13), [2026-10-16-wildcard-refits-q23](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q23), [2026-10-16-wildcard-refits-q44](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q44).

## upgrades-cost — Initial refit prices

**PROVISIONAL** · Recorded 2026-10-18

Start testing at 700 shards for the first transformation, then 1,200/1,800 only for meaningful further stages. Saving the first arena's six 120-shard clear rewards yields 720, putting a first upgrade near arena one's end. Costs and earnings remain tuning values, not guaranteed fun or economy acceptance.

Sources: [2026-10-16-wildcard-refits-q17](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q17), [2026-10-16-wildcard-refits-q45](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q45).
