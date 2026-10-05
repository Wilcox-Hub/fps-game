# Blast capture review

Initial actual-shot integration, collision/cover, local co-op, input, fire, and loadout checks passed. The first variable-time screenshot showed only a fragment of the transient pulse, despite readable recipient counts.

Ranked hypotheses: (1) impact plane was edge-on/buried; (2) screenshot occurred after the 0.38-second pulse; (3) material was too faint. One-variable timing probe kept the production pulse geometry/material/orientation unchanged and made only the capture portion use fixed 1/60 world steps. Logs measured normal (-1,0,0), radius 460, capture age 0.100, and all 24 visible parts. The resulting actual rendered frame showed the whole bright thin pulse and hit/down count. This supports capture timing as the cause in this scene and rejects the orientation/material hypotheses for the sampled front-facing shot; it does not validate every grazing angle or minimum-PC performance.

Removed all temporary probes. Retained an actual-shot visual regression requiring 24 visible parts at capture, world anchoring under pawn movement, expiration and repeat reuse. Fixed-step state restores on fixture destruction, and begins only after checking the real 20-second normal-world regroup. Added a dark backing behind the blast count for bright scenery. Production damage, radius, prices, pacing and effect orientation remain unchanged.
