# Lean and Ion Blaster — approved implementation direction

Approved by the owner on October 11, 2026 following the grill-me interview.

## Lean and controls

Manual Q/E left/right lean. Hold is the default; local Settings offers a persisted toggle option. Grounded standing/crouched slow peeks support hip fire and aiming. Sprint, slide, jump, carrying, downing and menus cancel lean. Reloading retains lean. Lean supplies no additional handling bonus; ordinary stationary crouch bracing still applies.

Heavy weapon families lean slightly slower; upgrading damage or magazine capacity does not change handling class. Prototype transitions take about 0.14 seconds for ordinary families and 0.18 seconds for heavy families. Movement intent travels in saved character moves and correction replays. Cover clamps the viewpoint and shoulder, muzzle-to-view traces block thin-wall shooting, and the exposed lean torso remains damageable.

V is melee (M remains a keyboard fallback). Remove middle-mouse melee. F remains healing; T prioritizes rescue when applicable, otherwise the arsenal. Z requests the equipped Ion Blaster's charged attack. Controller lean mapping is deferred.

## Arsenal and Ion Blaster

Keep the arsenal's known purchases, upgrades and existing token-based wildcard reroll. No separate mystery box, area weapon unlocks, additional slot or payment system. Ion is a known arsenal purchase during the run, not a new starting selection.

Ion fires semi-automatic normal shots. Only the first living enemy directly hit by a normal shot earns charge; eligibility is captured before damage, so lethal direct hits count. Misses, other players' kills, encounter rewards, splash, Echo/Through recipients and the charged attack earn no charge. One full charge can be stored; normal fire remains available while full.

Charge requires roughly three–four original magazines of successful hits. Prototype: 12-round base magazine, 42 successful hits, 72 reserve, 0.4-second normal refire, damage scale 2 (84 reference damage before target modifiers), 900 shards. Magazine upgrades keep the same 42-hit requirement. These are provisional tuning values, not approved balance or measured fun.

Full charge produces a local original two-tone ready sound and a compact equipped-gun HUD cue. Z starts a 0.4-second windup. Sprint, weapon changes, reloading, melee, menus or incapacitation cancel it. Spend one round and the full charge only when the shot actually fires. An empty magazine requires reloading.

The charged shot deals aimed direct damage (prototype 420 before modifiers) and a small 2.8m cover-checked disruption pulse. Susceptible bots stagger briefly and lose aim accuracy briefly. Brutes/champions resist the stagger and receive shorter aim disruption. A refractory interval prevents chained interruption. No body-centered universal Ion attack remains.

Charge belongs to the gun: preserve it through reloads, holstering, upgrades, regroup and checkpoint continuation. Ordinary damage upgrades improve both attacks; chosen Echo/Through behavior applies only to normal shots.

## Perks, saves and multiplayer

Replace perk node 9, Charge Recovery, with Heavy Target Training: prototype +8% damage against brutes/champions. Preserve the node identity and unlock requirement so existing perk selections survive. Apply the bonus at the actual enemy recipient, including relevant weapon damage paths.

Checkpoint schema 2 stores per-offer gun charge. Valid schema-1 five-offer saves migrate in memory to the six-offer catalog with empty Ion charge; retired universal charge cannot become weapon charge. Preserve original files until a normal new safe-boundary save. Unsupported/malformed files still block Continue and writes.

Network protocol advances to VS60. Both players and the private server need matching builds. Prior VS58 package/server checks are dated history, not passes for changed inputs.

## Acceptance

Required: current Editor compilation and focused control/cover/charge/save checks, affected movement/progression/checkpoint regressions, two-client owned input/state checks, rendered 1080p review, Windows Shipping build and matching package delivery. Native Linux server and any live deployment need matching protocol and independent verification. Human combat feel, actual different-WiFi/two-PC play and balance remain open.
