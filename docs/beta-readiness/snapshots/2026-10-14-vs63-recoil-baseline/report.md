# VS63 recoil baseline — October 14, 2026

The owner rejects the current shooting/recoil feel as dull and fake, and selects **Apex Legends as the recoil-style reference**. This assessment reproduces the existing mechanical baseline. The shooting-feel fix is **not implemented or published**. Lean remains removed; the live game/server/launcher continue to deliver VS63.

## Evidence

All 621 published VS63 inputs matched before diagnostics. The actual possessed-owner physical-direction check passes under normal and inverted legacy pitch scales. Its retained manifest/date applies to the pre-fixture build. The new Editor-only recording fixture and guarded friend declaration add one file and modify one header; current input count is 622. Production recoil formulas/firing behavior are unchanged.

The newly compiled 1080p fixture fires through normal `DoStartFiring` and real world timers/ticks, samples camera and camera-owned gun transforms, then waits after trigger release. Its explicit angular-kick criterion reproduces **red in all four cases: 0.000° observed weapon rotation**. The gun translates backward only. The build succeeds; this is an expected design-baseline failure, not a crash.

| Case | Shots / Helix charges | Observed peak aim rise | Observed gun rotation | Observed backward travel | Aim rise remaining after pause |
|---|---:|---:|---:|---:|---:|
| GaleHip | 7 | 2.166° | 0.000° | 0.605 cm | 2.166° |
| GaleADS | 9 | 2.873° | 0.000° | 0.605 cm | 2.873° |
| LanceHip | 3 | 2.817° | 0.000° | 1.972 cm | 2.817° |
| HelixHip | 5 | 5.278° | 0.000° | 1.059 cm | 5.278° |

These are frame-sampled observations during a controlled 1.45-second hold and a pause, not guaranteed peaks, fire-rate benchmarks, balance results or measured fun. AI and fixture damage are controlled. Helix retains three real pulses per charge. Screenshot requests are asynchronous; early stills are not exact shot-onset proof, and the directly equipped test roll need not match the run-state HUD label. The CSVs and unchanged source establish the motion findings.

## Ranked candidates after reproduction

1. **Translation-only viewmodel feedback** — reproduced by all four actual firing cases. Current `Tick` slides the parts back and decays that offset; it adds no angular kick or velocity-driven settling. This is the first motion change to address after direction confirmation.
2. **Weak/common lateral pattern** — source inspection shows one sinusoidal curve with small random noise for every gun, with Auto/Burst/other strength differences. It cannot provide distinct authored family paths by itself. Human impact remains unmeasured.
3. **Delayed joining-player feedback** — source-confirmed server fire request followed by separate unreliable owner-recoil and effect messages. There is no local recoil prediction, shot identity or original-weapon profile in that message. Delay/loss and stale-shot effects require actual network reproduction; none is claimed here.
4. **Sequence inheritance across equipment changes** — source inspection finds only a 0.45-second pause reset; rapid changes can retain the previous sequence index. A dedicated regression is still needed.

An additional correctness risk remains unverified: authoritative ordinary/burst/Ion traces use the remote server camera component's forward vector. Character movement transmits control rotation, but camera rotation correctness still needs an unassisted owned-client aim/trace check. Older combat smoke explicitly writes the server camera and cannot clear this gate.

## Design frontier

The existing approved direction already settles learnable upward family patterns, modest lateral variation, heavier/high-rate guns requiring more control, and slide accuracy/handling penalties. Apex is an approved style reference, with independently tuned IRON SUN weapons. EA's [Season 6 designer notes](https://forums.ea.com/blog/apex-legends-game-info-hub-en/season-6-patch-notes/9462069) discuss the controllability tradeoff of constant horizontal recoil versus a revised pattern; they do not provide an exact implementation to copy.

Three independent feel choices are currently awaiting the owner: opening-burst versus immediate recoil demand; smooth remaining-recoil recovery versus manually returning aim; minimal camera disturbance versus stronger shake/sway. Recommendations are manageable opening bursts with demanding sustained fire, recovery respecting mouse compensation, and clear aim with minimal shake. Silence is not approval. Numeric tuning remains provisional until human playtesting.

## Readiness and delivery

**NOT_BETA_READY.** 69% remains the low-confidence October4 gameplay planning estimate. No score credit, new Windows package, launcher feed or Lenovo deployment. The original VS63 delivery evidence remains dated and scoped; the new diagnostic fixture does not authorize blanket reuse of old input hashes. This public folder retains the numeric baseline and input hashes. Full recordings, logs, source snapshots and screenshots remain local; they are not part of this metadata publication.

Next: settle the three recoil feel choices, implement and compare gun motion/patterns, cover manual compensation/frame timing/weapon changes and unassisted owner/server aim, then test matching packaged/co-op builds and obtain human feel acceptance. Active shield direction, production character fidelity and future menu motion/audio remain open.
