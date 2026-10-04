# VS36 — shooting reliability

Outcome: keep the owner's selected primary through encounter rewards/reinforcements; shots should correspond to visible NPC geometry, cover and intended damage behavior. Preserve current enemy HP progression, wildcards, universal bracing and the existing finite route.

Relevant files: GameMode weapon resupply, ShooterCharacter traces/damage/presentation, ShooterNPC visibility collision, focused runtime tests and runner. Preserve unrelated unpublished work.

Non-goals: pacing redesign, tenfold map expansion, gated entrances, replacement enemy art, leaning, Steam or server ownership changes. Pacing interview remains open.

Evidence: failing engine regressions before production changes; minimized repro and ranked falsifiable causes; green regressions, affected arsenal/piercing/presentation and owned-client checks; rendered inspection where feedback changes; Editor/Shipping builds and package evidence explicitly scoped. Cache unaffected checks only with matching hashes.

Status (2026-10-04): the owner closed Editor. Both defects were reproduced and corrected: normal sprint input fired the template swap action; the movement capsule accepted empty-space shots. Seven new combat/input/PIE tests pass, along with weapon/piercing/arsenal checks, controlled two-peer combat and a fresh Shipping Prototype build/startup. The old loadout fixture now reserves wildcard secondary and checks Phasma identity before its ammo snapshot. This snapshot records readiness evidence separately from unpublished game source. Human fun/pacing and packaged normal-input/co-op acceptance remain open; no new percentage credit is claimed.
