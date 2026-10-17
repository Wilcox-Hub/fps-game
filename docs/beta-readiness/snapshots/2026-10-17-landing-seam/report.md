# Entrance stair landing artifact

The live VS68 game shows a broad crosshatched strip at the top stair. A focused production-component regression reproduces one coplanar overlap: the final tread and landing share1800x77.5cm of top surface at the same height. This is depth competition between geometry faces, rather than a missing texture.

The final tread now ends at the existing landing rear boundary, and its accent trim stays within the remaining tread. The landing footprint, twelve steps, elevations and physical component identities are retained. The regression fails before the repair and passes afterward with zero shared face area; all27 downward probes across the left, center and right of the join retain solid upward-facing support. The Editor target builds successfully, and the current1080p rendered test passes. Both saved captures were inspected and the reported broad crosshatched strip is absent.

The initial rendered attempt stalled at Unreal's10-FPS automation-start gate while two player windows were open. That timeout is not a failed geometry result. Independent NullRHI before/after results establish the defect and fix. The successful visual-only run used a process-local1-FPS start threshold; it changed no saved game settings and establishes no performance acceptance.

This repair is **local source/Editor only**. The open game, public launcher, owner installation and Lenovo server remain on the previously published VS68 game20261016-213325-765. No new Shipping package, multiplayer result or human acceptance is claimed. The isolated100-roll audit stays paused. Whole-game status stays NOT_BETA_READY;69% remains the low-confidence October4 planning estimate, with no score increase.
