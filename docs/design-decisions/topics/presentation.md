# Visual direction, characters and readable UI

Generated from [current.json](../current.json). Read current rules before historical interviews.

## presentation-ui — Sparse, readable and consistent

**APPROVED** · Recorded 2026-10-18

Use the approved main-screen composition across lobby, selections and arsenal. Reduce information density, keep text crisp and native, give selections stable layouts without one-frame glitches, and put deep stats under Details. Portal 2's sparse menu and the owner's supplied pause/audio references guide hierarchy; copying their artwork is not approved.

Sources: [owner-menu-reference](../../../docs/design-decisions/sources.md#owner-menu-reference), [owner-lobby-direction](../../../docs/design-decisions/sources.md#owner-lobby-direction), [2026-10-14-arena-arrival-q11](../../../docs/design-decisions/interviews/2026-10-14-arena-arrival.md#2026-10-14-arena-arrival-q11).

## presentation-resolution — 1080p minimum;1440p and 4K

**APPROVED** · Recorded 2026-10-18

Target readable native UI and correct scaling at 1080p,1440p and 4K. Prior 720p checks are extra regressions. Validate actual renders instead of treating generated concepts as game screenshots.

Sources: [owner-resolution](../../../docs/design-decisions/sources.md#owner-resolution), [current-evidence](../../../docs/design-decisions/sources.md#current-evidence).

## presentation-characters — Match the character mockups

**APPROVED** · Recorded 2026-10-18

Playable 3D characters should follow their approved individual mockups. Keep the lizard selectable, with human foreground/lizard behind in the approved lobby composition. Portrait/backdrop art and prototype mesh visibility do not establish finished model fidelity.

Sources: [owner-character-fidelity](../../../docs/design-decisions/sources.md#owner-character-fidelity), [owner-lobby-direction](../../../docs/design-decisions/sources.md#owner-lobby-direction).

## presentation-weapon-art — Distinct alien construction

**APPROVED** · Recorded 2026-10-18

Support radically varied alien weapon construction, suggestive of 100+ civilizations, while preserving practical handling, readable muzzles, working ADS and clean rack mounting. Later sheet preference is 1/2/3/4/6; concept 2 uses dark iridescent ceramic and smoked glass. Keep sheet-specific references distinct; a numerical concept choice alone cannot reconstruct a missing image. Upgrades should physically develop the gun.

Sources: [owner-art-sheet](../../../docs/design-decisions/sources.md#owner-art-sheet), [2026-10-16-wildcard-refits-q19](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q19), [2026-10-16-wildcard-refits-q29](../../../docs/design-decisions/interviews/2026-10-16-wildcard-refits.md#2026-10-16-wildcard-refits-q29).

## presentation-lobby-motion — Moving backdrop and sound later

**DEFERRED** · Recorded 2026-10-18

Add appropriate lobby motion, ambience and menu feedback in a future polish pass. Preserve the approved composition; exact animation/audio treatment remains open.

Sources: [owner-moving-backdrop](../../../docs/design-decisions/sources.md#owner-moving-backdrop).
