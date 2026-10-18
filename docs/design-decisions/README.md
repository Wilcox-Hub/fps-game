# IRON SUN — Design Decisions

[Project memory tree](../memory/README.md) · [Machine-readable rules](current.json) · [Recovery coverage](coverage.json)

Current rules below are the design baseline. Historical interviews preserve what was asked and answered; later user corrections take precedence. Approved direction is separate from implemented, tested and published work.

## Current direction

- [Identity, modes and naming](topics/identity-modes.md)
- [Combat, movement and controls](topics/combat-movement.md)
- [Waves, arena layout and pressure](topics/waves-arenas.md)
- [Two-slot weapons, shards and arsenal](topics/weapons-economy.md)
- [Wildcard pool, potential and physical upgrades](topics/wildcards-refits.md)
- [Squad lobby, perks and arena arrival](topics/squad-arrival.md)
- [Visual direction, characters and readable UI](topics/presentation.md)
- [Co-op, launcher and delivery](topics/network-delivery.md)
- [How development, memory and validation work](topics/work-policy.md)

## Original interview answers

| Interview | Questions | Answers saved |
| --- | ---: | ---: |
| [2026-10-02-perks](interviews/2026-10-02-perks.md) | 1 | 1 |
| [2026-10-04-pacing-economy](interviews/2026-10-04-pacing-economy.md) | 5 | 1 |
| [2026-10-08-combat-movement](interviews/2026-10-08-combat-movement.md) | 24 | 24 |
| [2026-10-11-lean-ion](interviews/2026-10-11-lean-ion.md) | 35 | 35 |
| [2026-10-13-naming-perks](interviews/2026-10-13-naming-perks.md) | 18 | 18 |
| [2026-10-14-arena-arrival](interviews/2026-10-14-arena-arrival.md) | 15 | 15 |
| [2026-10-14-recoil](interviews/2026-10-14-recoil.md) | 5 | 5 |
| [2026-10-14-shield](interviews/2026-10-14-shield.md) | 1 | 0 |
| [2026-10-14-wave-pressure](interviews/2026-10-14-wave-pressure.md) | 3 | 3 |
| [2026-10-16-wildcard-refits](interviews/2026-10-16-wildcard-refits.md) | 52 | 52 |
| [2026-10-17-coop-diagnostics](interviews/2026-10-17-coop-diagnostics.md) | 3 | 3 |

## Source notes

[Supporting owner statements and evidence](sources.md) preserve direct choices and explicit approvals outside numbered interviews, with original wording where recovered. Initial numerical balance choices are provisional. A named concept image without its original sheet is a visual-reference gap, not permission to invent its appearance.

## Maintenance

Follow [the memory workflow](../memory/WORKFLOW.md). Edit current.json and interview JSON, then run `python tools/project_memory.py render` and `python tools/project_memory.py check`. Generated Markdown is a readable view, not a separate source of truth.
