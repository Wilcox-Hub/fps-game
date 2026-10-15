# VS65 opening pressure and arena scaling

Published internal alpha **20261014-142836-108**, protocol 65, through the existing launcher 1.0.1. Matching Lenovo server and owner install are updated. [Release](https://github.com/Wilcox-Hub/iron-sun-releases/releases/tag/playtest-vs65-20261014-142836-108-launcher-1.0.1-setup-1). Source remains local and unpublished; this is not beta approval.

The owner confirmed all three grilling recommendations: easy rounds 1–2 apply only at the start of the whole run; opening relief includes fewer enemies and lower damage; later pressure is led by enemy numbers/composition while health/damage grow more gradually.

| Solo first arena | Total enemies | Arrival group | Maximum alive | Enemy damage multiplier |
| --- | ---: | ---: | ---: | ---: |
| Round 1 | 6 | 2 | 3 | 1.00 |
| Round 2 | 10 | 3 | 4 | 1.12 |

Previously round 1 budgeted 24 solo sentries with up to 7 live and a 2.00 damage multiplier. Pressure grows from round 3 through role budgets, overlapping arrivals, health and damage. Existing AI/tactics, safe multi-gate emergence, warning cues, distress relief and 90-second regroup/early readiness remain.

Later arena entries reset group/live limits and use fewer enemies while retaining the previous final-round damage. For the same role and party size, ordinary enemy health becomes exactly 90% of the previous arena final round, then grows from that baseline. A solo sentry goes 198 → 178.2 at the first boundary. The calculation is stateless across reload/resume. Party changes also apply normal encounter-start party modifiers. The three-round finale uses opening/middle/peak curve steps; unique champion health is unchanged.

The arsenal solo reference preserves current run progression instead of recreating arena one. New curve tests fail before the change and pass afterward. Current rules/comparison checks, a completed accelerated 21-encounter/four-arena route, exact Shipping two-client/native 1080p setup/firing, native first-wave population/health/damage observations, production LAN/relay lobby/auth and 32 current / 32 rollback owner hashes pass. Earlier actual-world spawn checks cover four layouts/parties at their retained source state. See test-results.json for source boundaries, failed/incomplete attempts and explicit scopes; 63 numeric curve samples are retained.

Keep the October 4 low-confidence **69%** planning estimate (weighted 68.5), **NOT_BETA_READY**, with target 75% open. Numeric correctness does not establish human ease, pressure, balance, pacing or fun. Normal physical two-PC/different-Wi-Fi combat, native saved-run restart, performance and production art remain open. Existing recoil stays implemented; lean stays removed; active shield/character work is separate.
