# October 4 owner gameplay review

The longer encounter loop is functioning in this run, but length has increased more than variety or tension. The owner explicitly likes later enemies needing two shots and reports that the game remains boring. Unexpected weapon changes and poorly explained multiple kills undermine the shooting experience before further balance tuning.

## Review scope

Source recording: `C:/Users/YaYa/Videos/Captures/Ridgefire - Unreal Editor 2026-10-04 13-23-26.mp4`, 556,471,624 bytes, 508.34 seconds, 1920×1032 H.264, stereo AAC. Solo Play in Editor, with the Editor UI visible. One arena is cleared; the next arena is played through part of its third encounter. There is no full-run victory, champion fight, packaged build, or multiplayer acceptance here.

Two visual passes cover the recording using different timestamp samples: 85 overview frames at six-second intervals, then 85 offset frames at 3, 9, 15…507 seconds. Close sequences add 16 frames at 73–80.5 seconds, 24 at 45–50.75 seconds, and 20 at 471–480.5 seconds. These are frame-based visual reviews, not continuous playback, audio evaluation, frame-time profiling, or hardware-input telemetry. Brief events between samples may be missed. The matching October 4 Editor gameplay log adds event counts and encounter timings; its UTC timestamps are not assumed to align exactly with the video's presentation timestamps.

The video and raw Editor log remain local. Published evidence includes observations and narrow derived metrics, not the raw recording or full log. The recording title and matched session markers do not establish an executable/source/package hash; current source hashes independently match the saved VS34 inputs.

## First pass: mechanics and defects

| Approximate video time | Observation | Evidence strength / implication |
| --- | --- | --- |
| 0:00–0:29 | Personal perks, weapon choices, team perk and secondary setup precede the arena. | Setup is visible; the clip starts partway through the perk stage. Do not treat 29 seconds as total setup duration. |
| 0:39–1:12 | Bots cluster around the central stations; much of the shooting is at close range in open space. | Position changes are visible, but repeated nearby targets often do not require a new tactic. |
| 0:47–0:49 | Several adjacent bots collapse during a short sequence of Phasma shots. | The close frames corroborate confusing grouped deaths. A quarter-second sampling interval cannot establish an exact causal hit for every collapse. |
| 1:15–1:16 | Equipped model/HUD changes from Phasma to Gale. | The owner explicitly states switches back to slot 1 were automatic. Video alone cannot reveal the physical key history. Preserve that testimony; do not blame user input. |
| Around 3:24, 3:36, 4:06 and 4:24 | Gale appears again during the continued fighting; Phasma is repeatedly restored. | Multiple additional examples warrant a selected-slot persistence regression check. No exact trigger or source cause is established. |
| Around 5:04–5:12 | First arena clears, route selection appears, then the brighter second arena loads. | Progression and a route choice are visible. First-arena duration is measured more precisely from the log below. |
| 5:18–6:06 | Player health falls more visibly and the player travels along stairs/walls and past cover. | More apparent pressure and varied geometry than the opening stretch; motivation for each movement is not directly observable. |
| 6:12–6:24 | Wildcard is used briefly before returning to a primary. | Arsenal variety is exercised, but this clip cannot compare weapon appeal fairly because unwanted switches distort usage. |
| 7:51–8:00 | Later combat includes larger/purple-accented enemies, close threats and clustered collapses. | Owner specifically likes some later enemies surviving a first shot. Preserve this preference; exact shots-to-kill by role/location is not measured by these samples. |
| Around 8:18–8:27 | Arsenal opens and Play in Editor ends. | The run is stopped, rather than completed. |

Two defects need independent checks:

1. **Unrequested return to primary slot 1.** Select slot 2 through its real binding, fire/reload, take pickups and kill enemies, cross pending reinforcement groups and the actual encounter-clear/reward/next-encounter boundary, and assert the same selected slot, equipped identity and HUD identity persist without selection input. Repeat for a possessed network peer. Existing normal-interaction evidence did not establish this whole boundary sequence.
2. **Hit/kill attribution.** Exercise actual visible NPC parts and gaps, cover and occlusion with current normal weapon presets, not just isolated deterministic test rolls. Record the real trace hit component plus all damage recipients. Compare isolated, aligned and adjacent bots, then moving/role variants. This must distinguish collision accuracy from intended multi-target weapon behavior. Any intentional splash/pierce must be visually explained and match its damage area; unintentional damage must be removed. No cause or fix has been validated in this review.

The diagnosing-bugs workflow is still at feedback-loop construction for these newly reported cases. Its instruction is “No red-capable command, no Phase 2.” Existing test files were inspected for seams; no new engine repro was run and no source hypotheses are presented as conclusions. The owner's Editor is still open, so a normal replacement-DLL rebuild will require closing it when bug implementation begins.

## Recorded measurements

Derived from the matching Editor log, with the extraction rules retained in `analyze-log.py` and full derived data in `log-metrics.json`:

- First arena: **271.786 seconds / 4:31.8**, from its first `Starting wave` event to its arena-clear event. Includes inter-encounter waits and final clear delay.
- Six first-arena encounters: **40.466, 36.013, 37.077, 33.169, 45.726 and 49.310 seconds**, measured from start to last recorded NPC death. They contained **24, 24, 28, 28, 28 and 32** recorded deaths.
- Last death to next start is approximately **5 seconds** for the five regular first-arena transitions. That is the observed recovery window, not a decision to keep it.
- Nine encounters start in the captured session. The second arena's first two start-to-last-death spans are **74.037 and 40.933 seconds**. Its third is incomplete at recording end; do not report its 53.173-second observed span as a completed-round duration.
- **239 mechanical NPC deaths** and **337 fire-input log events** appear in the bounded session. Inputs include cadence-rejected and empty-magazine attempts; they are not successful shots or a hit rate.
- **13 groups** contain a Phasma fire-input event and **two NPC deaths in the same engine frame**. This corroborates two-bot clusters. It does not confirm the reported three-bot case, identify a collision bug, or prove damage causality by itself.
- Phasma accounts for **289/337 fire-input events (85.8%)**, Gale for 45, and the wildcard for 3. Treat this as observed use, not a preference ranking or weapon-strength measurement.
- **18 empty-magazine fire-input events** are logged. Frequent six-round cycling is visible, but the log does not measure reload satisfaction or frustration.

## Second pass: fun scorecard

This is a developer judgment about this solo clip, anchored by the owner's “still kind of boring” feedback. It is not an automated psychological measurement, a population result, a percentage of fun, or a beta-readiness score. Scale: **1 = absent or actively undermined; 2 = weak/repetitive; 3 = useful but uneven; 4 = strong and recurring; 5 = consistently compelling.** No composite total is calculated.

| Area | Rating | What works | What weakens it / why |
| --- | ---: | --- | --- |
| Progression | 3/5 | Later tougher targets, larger silhouettes and the arena change make progress perceptible. Owner likes some enemies needing two shots. | More HP increases commitment, but often leaves the same shoot/reload response. Preserve that step while adding decisions, rather than multiplying health everywhere. |
| Pressure and release | 2/5 | More visible health loss and repositioning in the second arena; late close threats create brief pressure. | Opening rounds frequently permit open-space shooting near clustered bots. Similar small waves and roughly five-second transitions blur into continuous routine. |
| Tactical decisions | 2/5 | A route choice, loadout options and cover geometry offer beginnings of agency. | Most visible targets invite the same solution. Few clear moments demand choosing between escape, priority target, supplies or an objective. Intent and unused options cannot be inferred from video alone. |
| Encounter variety | 2/5 | Sprinter/brute silhouettes and the bright second arena provide contrast. | Six opening encounters repeat around the same central area. Enemy count changes more clearly than the behavior demanded from the player. |
| Visual shooting feedback | 3/5 | Beams, hit cues and collapsing bots make actions visibly responsive. | Nearby unintended/unclear kills and ambiguous exposure make precision hard to trust. Audio was not evaluated. |
| Control trust | 1/5 | Basic movement, shooting, reload and selection are exercised. | Owner-reported automatic switching directly interrupts the chosen weapon and breaks continuity. This is a defect, not a difficulty feature. |
| Combat readability | 2/5 | Distinct colored cores and some larger enemy shapes can signal roles. | Persistent center-screen rescue-tube text, small HUD text, bright glare and overlapping bots obscure what matters. The rescue prompt often appears without an actual rescue being shown. |

The most promising moments are **later surviving targets, short periods of increased pressure, and moving into a different arena**. They introduce commitment, uncertainty and contrast. The weakest stretches repeat **aim at nearby bots → shoot → reload → repeat**, with little reason to rethink position or target order. Making those stretches longer repeats the boredom.

## What the next slice should prove

Fix control trust and hit attribution first. Keep the liked late two-shot progression as a tuning reference, not a requirement that every enemy become durable. Pacing direction remains under the owner's requested interview: there is no approved choice yet between horde looping, defensive holds, power growth, rescue pressure, finite routes or endless rounds.

Once those preferences are settled, a small revised arena should create readable pressure that makes staying still costly, at least one enemy priority that changes the player's response, useful recovery decisions, and a visible reward/next-round buildup. Leaning can support cover decisions, but this footage suggests it also needs situations that make cover and movement worthwhile.

For the next human comparison, record forced switches, damage outside intended hit/splash geometry, encounter time, health loss, emergency repositioning, resource choices and the owner's enjoyment/tension/frustration rating after each round. Use telemetry for events and player ratings for enjoyment; do not infer enjoyment from kill counts. Proposed design outcomes await the interview and are not implemented here.

## Readiness impact

No game source changed and no engine checks/build/package runs were repeated for this review. All 550 current source inputs match VS34; older passes retain their dated scopes. This clip adds owner Editor feedback and one measured arena duration, while exposing open control/combat issues and rejecting fun/pacing acceptance. It does not justify raising readiness. Retain the **dated 62% low-confidence gameplay planning estimate**, unmeasured overall beta completion and **NOT_BETA_READY**. The 75% target remains open.
