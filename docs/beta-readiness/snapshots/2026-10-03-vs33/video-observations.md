# Recorded gameplay observations — October 3, 2026

The owner supplied `Ridgefire - Unreal Editor 2026-10-03 10-33-18.mp4`, then clarified that it was the requested gameplay recording. Explicit feedback: levels end too quickly, recoil pulls in the wrong direction, boss health is too low, the game is too easy and difficulty must scale with more players. The sampled observations below concern that pre-fix Editor recording. VS33 subsequently addresses this feedback and records separate source-bound tests.

Recording: 430,250,922 bytes; container duration 7:42.27; H.264 video, 1920 x 1032; stereo AAC audio. The image shows an Unreal Editor PIE viewport. The source revision, loaded binary, input method and recording frame pacing are not established by the file. Container frame rate is not a performance benchmark.

Method: inspected 31 frames at 15-second intervals, seven frames at four-second intervals from 3:42 through 4:06, and twelve frames at four-second intervals from 6:54 through 7:38. Reviewed seven contact sheets and four larger individual frames. This is sampled visual review, not continuous playback, an audio review or a new live test. Brief failures can fall between samples. The final audio/container timestamp did not produce a video frame; extraction was completed using earlier decodable frames.

## Direct observations

- Opening personal-perk and weapon-choice screens are visible, followed by combat. Selected/highlighted perk rows change during the second setup sequence. The footage alone does not verify every selection or its gameplay effect.
- Arena appearance, wave counts, ammo, enemy counts and score change during play. Weapon switching, projectile/beam effects and fallen mechanical enemies are visible in representative frames. Precise input response and ammo conservation were not measured.
- At 3:46, the overlay reads `CHAMPION DEFEATED / RUN COMPLETE`, with wave 7 and score 13,200. At 3:54 a new personal-perk setup appears. This supplies visual evidence of a recorded victory-to-new-setup sequence, without proving the compiled source revision or multiplayer behavior.
- The second run reaches the champion. At 7:30 the HUD shows 1% vitals and a champion with 465 HP / guard down. At 7:34 the defeat overlay reads `THE PIT CLAIMS ANOTHER`, wave 5 and score 7,040. The Editor leaves play shortly afterward. The second run therefore ends in defeat, not victory.

## Presentation concerns to discuss

1. Large top status panels, a wide bottom controls panel and a central champion banner occupy substantial viewport space. A compact player HUD and optional debug overlay would improve combat readability. Text size should be assessed in a standalone window before changing scale based solely on the smaller PIE viewport.
2. Enemies and the champion use similar assembled primitive shapes and stiff silhouettes. Around 3:00, 6:30 and 6:45, close enemies occupy much of the aiming view. Distinct silhouettes and readable approach/attack/recovery motion would help. These images do not establish capsule penetration or an AI spacing root cause.
3. Much of the environment remains primitive geometry, bright pillars and broad reflective surfaces. The footage still lacks the finished first-arena art direction. Glare and landmark readability warrant inspection in both bright and dark views.
4. World-space station labels and the rescue-tube prompt can compete with the central combat view. Their visibility and proximity rules need a live reproduction before classifying them as bugs.

Several presentation concerns were already recorded in the September 29 video review. Do not treat their reappearance as newly diagnosed source defects or rerun unrelated tests solely because of this recording.

The sampled frames alone do not diagnose the recoil cause. A separate VS33 owner-controller regression reproduced downward camera aim with an inverted legacy pitch scale, then passed after physical recoil was separated from look scaling. VS33 also adds reinforced encounters, larger boss HP and connected-party scaling. Exact source/binary correspondence for this recording remains unknown; post-fix human balance, audio, physical-controller response and packaged co-op remain unverified. The owner recording supersedes the old statement that the owner cannot currently playtest. See the VS33 snapshot for the dated assessment and fresh test scopes.
