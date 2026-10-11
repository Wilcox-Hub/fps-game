# Pacing and weapon progression design

**Status:** player-approved direction with the first economy/progression implementation, October 4, 2026 (VS37). Cash, paid known weapons, compare-before-confirm, player-chosen upgrades, private melee-finisher rewards and 20-second regroup breaks are implemented. Final run length, balance and human fun acceptance remain targets.

## Player experience

Keep IRON SUN a sci-fi sentry-arena shooter. Borrow the satisfying arc of starting with dependable, modest gear, discovering a weapon that feels special, and developing it to meet increasingly tough enemies and bosses. Preserve deliberate combat decisions and purposeful enemy behavior. The pacing must not depend on repetitive circular kiting or mindless crowds.

Target a satisfying successful main run of 45â€“60 minutes. Give players roughly 15â€“25 seconds between encounters to reload, regroup, and consider upgrades, with an option for the team to start early. These duration and rest-window targets require human playtesting; they are not measured values.

## Approved two-weapon carry limit (October 5, 2026)

Normal runs carry exactly one primary and one secondary. Both must be useful, with different strengths. Key 1 equips the primary and key 2 equips the secondary. There is no third carried slot, third-slot perk, or capacity unlock in the core design. Opening offer keys remain choices among weapons, separate from combat carry slots.

Known purchases and confirmed wildcard finds replace the selected primary or secondary. The arsenal compares against that destination and names the weapon being replaced before confirmation. Players must commit to a loadout with strengths and weaknesses; upgrades and weapon behavior deepen the chosen guns. Ammunition, reload windows, positioning and complementary teammate loadouts should create pressure. Whether this feels more demanding and fun needs normal playtesting.

This decision supersedes earlier extra-slot and two-primary references below. Existing per-offer upgrade/ammo history remains run-local: replaced guns cannot be equipped or upgraded while uncarried; buying one back still costs cash and does not refill it. Losing upgrades on replacement, weapon stashing and refunds are not newly introduced rules.

## Weapon acquisition and upgrades

- Keep the existing wildcard reroll as the surprise-weapon lane. Known weapon purchases and player comparison provide a reliable, deliberate lane alongside it; do not create a duplicate random-box system.
- Let a player inspect a found weapon's information and compare it with their current weapon before choosing to keep or pass on it.
- Cash should support both known-weapon purchases and upgrading a gun the player has found, with known weapons and meaningful gun development as the focus.
- A major upgrade should clearly raise the gun's power and let the player choose one distinctive firing-behavior change. Preserve each weapon's identity while helping it stay useful against later enemy health and bosses.

### Comparison and stat readout

Show exact values, the difference from the weapon being compared, and a short plain-language explanation of the change. Include at least:

- damage per shot;
- accuracy;
- fire rate;
- burst DPS and sustained DPS;
- magazine size;
- recoil as **Low**, **Medium**, **High**, or **Very High**.

Reload time, reserve ammunition, and effective range are useful supporting stats to evaluate when the display is implemented. Define their measurement conditions consistently; do not present an unverified rating as measured performance.

The comparison should estimate how a weapon performs against relevant standard, armored, and boss enemy types at the current run's party size and enemy-health scaling. Offer solo values as a reference. Include shots-to-defeat where that estimate is meaningful, and explain which enemy trait drives the difference. Keep player count and upgrade state consistent across the comparison.

## Rewards and co-op cash

Encounter and optional-objective cash rewards are shared with the team. A difficult melee-finisher bonus goes to the player who earns it: weaken an enemy, then risk closing in during a brief opening to finish it. The bonus should reward the commitment and timing rather than ordinary melee damage. Exact reward amounts and purchase prices remain to be tuned.

## Decisions to revisit

- **Resolved October 8, 2026:** pressure-inducing mixed attacks, variable arena/run strategies and momentum sliding are player-approved. See [COMBAT_AND_MOVEMENT.md](COMBAT_AND_MOVEMENT.md) for this pass, initial tuning and validation limits.
- **Resolved by the player:** use the existing arsenals during regroup breaks for this first implementation. Purchases also work during live encounters at the actual arsenal; upgrades require an active regroup break.
- Tune cash amounts, weapon prices, upgrade costs, upgrade cadence, and the implementation details of accuracy and DPS values.

## Validation needed

After implementation, test weapon stat calculations and comparisons against actual firing, reloads, enemy armor, bosses, upgrades, and one-to-four-player scaling. Review the rendered comparison screen for readability. Measure run and between-encounter pacing in normal play, then gather human feedback on whether weapon finds feel special and upgrades remain useful. Design approval alone is not implementation or beta-readiness evidence.

## First playable implementation (VS37)

Players enter with a dependable weapon. Known offers are Gale Repeater (300), Phasma Rifle (600), Helix Carbine (650, burst) and Vector Lance (800, piercing). The existing wildcard remains a free surprise find; reroll it and inspect its comparison before confirming it. These are provisional prices and the new weapons reuse prototype presentation.

An encounter pays every possessed teammate 120 cash, including downed players. A completed optional objective pays 180 each, once. Cash is personal, run-only, and server-authoritative; spending it does not reduce teammates' balances. A weakened ordinary/sprinter/brute sentry at or below 25% health exposes a 2.5-second melee-finisher opening. Closing within 1.9 metres and landing the finish pays the attacker 90 once. Ordinary melee, turrets and champions do not award that bonus.

At the existing arsenal, each upgrade adds 50% base damage, reduces refire delay by 8% and increases magazine capacity by approximately 25%. Levels cost 450, 800 and 1,150 cash. The first upgrade chooses and locks one behavior: **Echo pulse**, a 35% delayed follow-up hit, or **Through shot**, a 50% direct hit on one next sentry along the actual muzzle ray, stopped by solid cover. Later levels preserve that choice and the original fire mode/payload/condition. Upgrading preserves ammunition; repeated purchases of an owned weapon do not charge or refill it.

Each encounter ends with a 20-second break. Every living teammate can mark ready to start early; downed teammates do not hold that vote open. The next encounter closes arsenal panels. Required relay/lens interactions cannot bypass an active regroup timer. The main run remains the existing three/four-arena prototype route; the 45â€“60-minute goal requires additional content and normal-play tuning.

Keyboard: **T** opens/closes a nearby arsenal; **Left/Right** browse; **Enter** confirms the purchase or upgrade; **U/I** preview Echo/Through; **J** selects primary/secondary; **Tab** toggles exact details; **G** rerolls the wildcard; **N** marks ready during regroup; **M/middle mouse** attempts a melee finisher. Numbers **1/2** equip the carried primary and secondary respectively, including inside the arsenal. Purchases replace the primary by default; **J** switches the destination to the secondary. Left/Right browse offers. Gamepad: **X/B** browse, **A** confirms, **LB/RB** preview, **Y** toggles primary/secondary, **View** closes; **L3** toggles exact details. **B** outside menus marks ready during regroup; **D-pad down** outside menus attempts the finisher.

The default comparison uses four familiar rows with rounded values and plain changes: damage, fire rate, magazine and recoil. Accuracy and reload remain in the exact view. **Tab/L3** toggles an exact-details view. That view displays modeled damage per trigger, accuracy cone in degrees, trigger rate, loaded-magazine average DPS, sustained DPS including reload, magazine size and labeled numerical recoil. It includes current-party and solo estimates for ordinary sentries, armored brutes (extra HP) and the champion's normal/75%-guarded states. Reference conditions are 7.5 metres, full player health, standing, no streak/headshot and all pellets hitting, with modeled expected damage. Phasma direct-impact comparisons exclude its reduced, distance-scaled splash; Mortar includes its modeled blast. Damage-per-trigger assumes the first shot after a reload; magazine/DPS estimates include subsequent shots. Chance-based payloads use expected damage. These are estimates with explicit assumptions, not measured hit probability or human effectiveness ratings.

Automated checks and rendered-screen inspection are recorded separately in `docs/beta-readiness` at repository root. Controlled funding and frozen/repositioned targets in those checks verify transaction and shot correctness; they do not establish natural cash balance, a satisfying difficulty curve or a 45â€“60-minute successful run.


## Arsenal usability update (VS44, October 5, 2026)

The main arsenal separates the carried loadout, selected weapon, four comparison rows and price/action. Empty primary slots fill before replacement choices; the display reflects that rule. Owned weapons show their actual carried slot and equipped state. Exact details occupy a separate full-page view. U/I (LB/RB) cancel their own active preview as well as choosing it; accepted upgrades return to the ordinary view. Changing the action clears old receipts. Purchase/keep/upgrade confirmation uses Enter/A; browse/preview does not spend money. Keyboard Enter paths and rendered views at 1280x720 and 800x600 are verified by VS44; this is not human readability, gamepad hardware or natural balance acceptance.


## Primary UI validation resolution (October 5, 2026)

The owner plays at 1920x1080. Validate arsenal and future HUD/UI work at that resolution first; 1280x720 and 800x600 remain fallback layout coverage. VS45 verifies all eight arsenal confirmation states at actual 1080p with unchanged VS44 source. Rendered inspection does not substitute for owner comprehension/fun acceptance.


## Opening clarity and player delivery (VS46, October 5, 2026)

Opening personal perks and secondary/primary selection now use larger landscape text and separate equipped/selected information. VS46 introduced one primary, the selected secondary and an initially empty extra primary. VS47 supersedes that capacity: one primary plus one secondary, with new guns replacing the selected carried weapon. All three starting secondary choices are verified through normal timed setup and both-gun switch/fire automation. Build owner packages into default Saved/Packages and verify the batch launcher selects that exact archive before handing off.
