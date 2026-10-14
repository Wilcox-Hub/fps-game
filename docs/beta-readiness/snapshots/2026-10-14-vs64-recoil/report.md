# VS64 recoil and tracking

Published internal alpha **20261014-134814-678**, protocol 64, through the existing launcher 1.0.1. The matching native Lenovo server is live. [Packaged release](https://github.com/Wilcox-Hub/iron-sun-releases/releases/tag/playtest-vs64-20261014-134814-678-launcher-1.0.1-setup-1). Source remains local and unpublished. This is not beta approval.

The approved direction is implemented: manageable opening bursts across gun families, stronger heavy-weapon kick, original learnable patterns, backward/upward gun articulation, modest ADS reduction and smooth release recovery that respects manual compensation. Bounded horizontal-tracking relief is the only Apex-inspired characteristic. Tracking relief caps at 25%; camera/weapon tuning remains provisional until the owner plays.

The aim correction uses current controller rotation for server shots and immediate local traces. Owner-only recoil packets reject duplicates and stale weapon revisions. Feedback remains server-confirmed; local prediction is not implemented. Gun beams retain their impact endpoint and follow the animated barrel.

## Evidence

- Final Windows Shipping and Linux Development Server packages use the same 628 Source/Config/Content hashes. Complete 32-file Windows inventory is in package-artifacts.json.
- Two exact final Shipping clients complete normal party/setup actions and fire both slots against an isolated native Linux server at 1080p. Real authoritative ammo decreases, selected slots remain stable, gun/aim articulation and release recovery pass. AI/damage are controlled.
- Final 4K native Slate menu actions and clean Shipping exit pass. Production LAN and relay two-client lobby holds and wrong-invite rejection pass.
- The production signed HTTPS updater installs this version on the owner PC; all 32 current and 32 previous files match. Existing launcher/setup binaries are unchanged. Friend-PC receipt is unverified.
- Focused dynamics, physical-input compensation, tracking bounds, packet ordering, camera limits, immediate aim/trace, Ion/cover and unassisted two-owned-client aim checks retain their recorded dates and narrow source scope. See test-results.json for red/green history and manifest boundaries.
- A delivery-harness timing failure was retained, then corrected by waiting for session completion and replicated readiness. No production input gate was bypassed.

## Readiness

Keep **69%**, the low-confidence October 4 planning estimate; weighted 68.5, **NOT_BETA_READY**, target 75% still open. Tests do not measure fun. Human recoil feel, normal two-PC and different-Wi-Fi combat, natural economy/45–60-minute pacing, native saved-run restart, performance and production art remain open. Production held zero real save files at deployment; preserving that empty inventory proves no human recovery scenario.

Lean stays removed. Active shield needs its separate behavior decision. Character prototypes still fail mockup fidelity. Full local evidence is retained; only metadata, numeric summaries and hash inventories are published here.
