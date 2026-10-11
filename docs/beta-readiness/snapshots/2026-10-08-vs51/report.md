# VS51: pressure combat, weapon feedback and momentum slide

The player-approved pressure combat/movement pass is implemented and delivered to the owner’s fixed IRON SUN launcher. The final Windows Shipping prototype is **20261008-175321-489**. Source and this readiness snapshot remain local/unpublished.

Enemies use burst-led mixed profiles, arena/run-weighted strategic assignments, limited flank routes and aim correction, with smarter tracking and original mechanical footsteps. Heavy/weak-point hits selectively interrupt ordinary enemies with a refractory window; brutes/champions resist. Prototype hit flinches and a short mechanical collapse are replicated/presented.

Weapons have family-based recoil scaled by power/cadence and sustained fire, original spatial shot/impact feedback, viewmodel kick, and actual timed Helix pulses. One burst spends one charge; pending pulses finish after trigger release and cancel on switch/reload. Prototype hold-to-aim zoom/pose tightens normal accuracy and slows movement.

Hold crouch with sprint momentum to slide. Velocity decays with limited steering, native collision, no boost/invulnerability and saved-move/compressed-intent prediction. Hold to remain crouched after stopping; release requests standing, sprint clears crouch. Sliding hipfire is more practical than sliding ADS. Dead/menu/carry states block new movement. Roll/dash remains deferred.

## Evidence

Editor build and final Shipping build/cook/archive pass. Both new pressure production-path tests pass together on final inputs headlessly. An earlier same-pass offscreen D3D12 run passed at 1920×1080 and both frames were inspected before the last controller hold-input fix. Final rendered reruns stalled at 3–4 FPS while another packaged game was open; no final rendered or performance acceptance is claimed. The real timed burst damages a visible solid armor surface over timer callbacks and cancels on weapon switch. Movement/aim/correction replay intent, recoil differences, tactical variation, hit interruption limits and committed enemy traces are checked. Existing input, physical recoil/stat, encounter-selection and collision/cover scopes are separately dated in test-results.json.

The exact-ray geometry test explicitly controls spread on disposable Editor test shooters, keeping the independent visible-mesh scan and full muzzle/fire/damage/Phasma/cover paths deterministic. This DEV_AUTOMATION seam is absent from Shipping and is not enabled by ordinary accuracy tests. It must not be called a normal stochastic shooting acceptance test. Earlier failed iterations are retained.

A real local Editor listen host/client checks remote predicted and authoritative slide/ADS/release, with server displacement, on a replicated test platform with frozen enemies and controlled placement. It does not prove friend LAN discovery, arbitrary latency or production terrain. Final actual Shipping bootstrap initializes a LAN lobby and is discovered/joined by a same-PC Editor client in NullRHI/no-sound mode. Shipping-native logging is disabled; the client/UDP evidence is preserved. Join-time movement-base resolution warnings are recorded; packaged movement/terrain, rendering, full run and human acceptance are not claimed.

The verified complete package is published to the owner’s local update feed and hash-verified into the fixed launcher install, keeping the previous build for rollback. A no-launch recheck confirms CURRENT. No remote PC was installed/updated. The authenticated network share and fixed-install firewall activation remain pending; the older approved timestamped-runtime firewall exception is not claimed for this new path.

## Readiness

Retain **69%** solely as the historical October 4 VS41 low-confidence planning estimate (68.5 weighted). This changed build has no new whole-game assessment or measured fun/difficulty credit. **NOT_BETA_READY; 75% target remains open.** Native hitboxes/cover, source/build checks and local two-process movement are separate from human combat pressure, run pacing and packaged two-PC acceptance.

Next human check: slide into/out of low cover, compare hipfire and ADS while sliding, feel heavy/rapid gun recoil, and check whether mixed attacks/strategic angles create pressure while remaining fair. Further scope: terrain/headroom/packet loss, audio mix/performance, authored sights/animations, polished enemy art/UI, larger maps/gates, full-run balance and friend co-op. No GitHub push was performed.
