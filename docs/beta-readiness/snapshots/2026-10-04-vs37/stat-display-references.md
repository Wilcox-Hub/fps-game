# Weapon-stat presentation

Player feedback on October 4: the first numerical comparison is confusing; compare it with popular titles.

- [Black Ops 6 official weapon guide](https://www.callofduty.com/guides/blackops6/pre-game/call-of-duty-guides-black-ops-6-multiplayer-pre-game-weapons-and-loadouts) describes Firepower, Accuracy, Handling and Mobility categories with general comparison bars. Its underlying categories include firing rate, recoil and reload timing.
- [Modern Warfare III Season 2 official patch notes](https://www.callofduty.com/au/en/patchnotes/2024/02/call-of-duty-modern-warfare-iii-season-2-patch-notes) explicitly describe detailed Gunsmith stat comparisons and corrected detailed spread values. This supports keeping advanced numerical inspection available.
- [Arena Breakout: Infinite official helper](https://www.arenabreakoutinfinite.com/act/a20251202store/index.html?lang=es) exposes separate vertical/horizontal recoil, ergonomics, accuracy, hip-fire stability, range and weight entries, with change indicators. Some labels in the retrieved page are Chinese; this is not an acceptance test of its current in-game UI.
- [Bungie's stat-display update](https://www.bungie.net/7/en-us/News/article/51250) discusses both numerical stats and visual bars, and distinguishes display changes from changes to actual weapon behavior.

Design inference: IRON SUN's short regroup window benefits from a compact first comparison and an optional exact-details view. The main view uses six familiar rows: damage, fire rate, accuracy, recoil, magazine and reload. Values are rounded and changes use plain terms/percentages; accuracy describes spread rather than inventing a hit-probability score. Details keeps exact deltas, loaded-magazine/sustained DPS, angles and party/solo enemy estimates. No external game's opaque composite score is copied into the game.

This adapts the presentation principle, not another game's weapon balance, content or branding. Player approval of the redesigned screen still requires normal use.
