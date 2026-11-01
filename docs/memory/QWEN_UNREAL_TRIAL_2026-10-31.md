# Local Qwen Unreal coding trial — October 31, 2026

Owner requested real Unreal coding tasks, comparison with existing work, and an
approve/deny review. Image-generation installation is deferred. This is a blind
reimplementation trial of two existing fixes associated with open issues
[#60](https://github.com/Wilcox-Hub/fps-game/issues/60) and
[#23](https://github.com/Wilcox-Hub/fps-game/issues/23), not delivery of a new
feature or closure of those tickets.

Workspace: `G:/AI/Benchmarks/unreal-qwen-20261031/`. The current unpublished
Source/Config/Content/Tools were copied and hashed; all trial edits/builds run
in `Candidate/`. The live checkout's game files, launcher, server and VS73
release are unchanged. The reference implementations are withheld from Qwen.

Model: existing ISTA IQ3_XXS-MTP, fast 32K profile at localhost:18081. No profile
or model change. Temperature 0.2, top-p 0.95, seed 311060, maximum 3072 output
tokens, reasoning unchanged/off. Requests, raw replies, metrics and generated
functions are retained. First attempts: team-wipe 22.719s total/2.428s first
token/34.893 tokens/sec; cached-controller cargo recovery 17.452s/8.987s/32.963.
These are short-context task measurements (965/4711 prompt tokens), not 32K
prefill measurements.

Initial review rejected the team-wipe repair: it omitted the immediate client
run-state refresh in the surviving-teammate branch. The first combined attempt
passed146/148 independent native C++ contract assertions. The current reference
passed148/148; the deliberately broken controls failed8. Qwen received this
specific failure feedback and produced its own repair; the reviewed repair now
passes148/148. The cargo function needed no repair.

These contracts execute unchanged function bodies against instrumented APIs.
Actual UE5.8.3 Editor compilation also passes. The cold isolated build took
approximately399s/166actions; both submitted functions compile/link in8.6s.
Existing checkpoint-world checks pass21.32s; the focused two-local-process
team-wipe fixture passes34.59s, including client game-over replication and
living-player rewards. The engine negative control fails the expected cached
cargo recovery assertions. All853 original input hashes remain unchanged.

**Scoped verdicts:** initial team-wipe DENY; its one Qwen feedback repair APPROVE;
initial cargo logout APPROVE. The repairs retain the reference behavior, rather
than improving that already-implemented feature. No source merge, game commit,
release, beta-readiness score change or human play acceptance is claimed.

**Unresolved baseline failure:** broader `-VerifyRestart` fails in both the
Qwen candidate83.49s and freshly compiled current reference83.97s. Both reach
correct survivor/final-wipe/client-over states, then time out after travel with
`fresh=0 movement=0 fire=0 living=2 local=1 remote=1 armory=0 wave=1 score=0`.
This is an observed test/integration failure, not a proven normal-play defect;
root cause remains open. Full restart/ticket acceptance is withheld. Failed
runs and reference control logs are preserved alongside the focused passes.

Exact configurations, requests, responses, code, diffs, build/gameplay logs,
controls and verdicts: `G:/AI/Benchmarks/unreal-qwen-20261031/REPORT.md` and
`RESULTS.json`. Checks use actual actors/local processes with NullRHI, not
rendered play, Shipping or physical-device acceptance. Qwen remains the existing
fast33792-token allocation; these task prompts were short, not full32K tests.

Next useful project task: isolate the restart fixture/fresh-start mismatch
against normal gameplay before changing production logic. Preserve the working
Qwen profile, live VS73 release and all trial evidence. Small local-model repairs
benefit from independent tests and a review/correction loop.
