# Orca worker workflow

Owner preference, October 20, 2026: use smaller independent helpers when useful, with **`gpt-6-luna` at `xhigh`**, in separate Orca conversations. The main conversation owns the plan, integration and review. Keep its model unchanged unless the owner requests a change.

## Dispatch and review

1. Keep quick, tightly coupled work in the main conversation. Delegate bounded searches, document checks, small isolated fixes or reviews when a focused brief will save main-context work. Start with one helper; use at most two simultaneously. Workers return their result to the coordinator and do not launch children.
2. Read the installed `orca-cli` and `orchestration` skills. Use native Orca workers so messages, completion and cleanup are tracked in Orca. A fresh conversation gets only its brief, required file pointers and acceptance criteria; copying the entire main transcript spends extra context.
3. Give each helper one observable result, exact writable paths (or read-only scope), relevant constraints and a concise report format. Read-only work can share the current checkout. Coordinate writes by exclusive file ownership. An isolated worktree starts from committed files; explicitly account for unpublished inputs before using one. Serialize Unreal builds, shared editor/Blender operations and packaging in the same project.
4. Create a Run, then launch each independent worker before waiting. The launch must confirm `launch.effective.model = gpt-6-luna` and `launch.effective.effort = xhigh`. An error or unknown result requires the skill's exact recovery procedure; inspect the accepted attempt before retrying.
5. Process all delivered messages and answer worker questions. Review the actual diff/report and acceptance evidence. A helper's claim is evidence to review; the coordinator owns acceptance. Preserve test dates and scopes and reuse matching recorded passes instead of repeating unchanged checks.
6. After accepted `worker_done`, immediately reuse the worker for bounded follow-up or release it through Orca. Acknowledge the processed delivery. Before finishing, account for every task and check that the Run has no reclaimable workers. Archived output stays readable after release.

Canonical CLI pattern (run from the correct workspace; copy returned IDs):

```powershell
orca orchestration run-create --objective '<outcome>' --json
orca orchestration worker-start --run <run-id> --spec '<bounded brief>' --task-title '<short title>' --worktree current --agent codex --model gpt-6-luna --effort xhigh --json
orca orchestration check --run <run-id> --wait --types 'worker_done,escalation,question' --timeout-ms 45000 --json
# After reviewing the returned result:
orca orchestration worker-release --dispatch <dispatch-id> --json
orca orchestration check --run <run-id> --ack <delivery-id> --json
orca orchestration worker-list --run <run-id> --terminal-state reclaimable --json
```

Worker lifecycle commands and IDs come from its live injected preamble. Orca's new-agent setting selects a structured Chat UI or a terminal chat; both are separate conversations. The October 20 check used the existing terminal-chat setting. Only change presentation when requested; worker control uses the same orchestration commands for either surface.

## Tools and limits

Use the shell/files, GitHub CLI, installed skills and configured MCP services already available to the agent. Use Orca CLI for its workspaces, agent chats, orchestration, browser and conversation search; load only the relevant skill reference. Check actual tool exposure and live connection separately from installation. Follow [environment guidance](ENVIRONMENT.md) for engine and asset dependencies.

Codex-app panels, voice, connectors, image generation and app-managed browser services can depend on that app/runtime. A cached skill does not establish that its service works in Orca. Report a missing capability and use a verified equivalent where available; preserve service credentials and paid-service approval requirements.

Luna is intended for lower-cost bounded work. Extra-high reasoning uses more reasoning work, and parallel helpers still consume the selected subscription's allowance. This workflow does not guarantee a particular subscription saving. Use the existing signed-in subscription; keep credit purchases, paid generation and account switching as separate owner decisions.

## Verification record

October 20: Orca 1.4.232 / Codex CLI 0.163.0. Installed Orca's bundled `orca-cli` and `orchestration` skills at upstream commit `49109f362f057cd2dc3d81cd378cb84d140c3dc5` through its pinned installer. `grill-me` and `grilling` remain installed.

Live check: Run `run_3601d60de709`, Task `task_c3106053edee`, Dispatch `ctx_eccc0c29735a`. Orca confirmed the requested and effective model/effort, a separate terminal-chat tab and a started turn. The worker completed its bounded read-only audit, queried GitHub successfully and delivered accepted `worker_done`. The coordinator reviewed the report and corrected its catalog interpretation: `skills get` lists Orca's bundled guides, while `skills installed` confirms Codex's `grill-me` and `grilling`. The worker was released and its transcript archived; the initial close timeout recovered using Orca's exact retry instruction. Final fleet accounting: one succeeded/completed worker, released, no action required.

Supporting evidence: `C:/Users/YaYa/Documents/ChatGPT/FPS-1/output/orca-setup/2026-10-20/`. Primary references: installed Orca orchestration kernel and `references/coordinator-loop.md`; [OpenAI subagents documentation](https://learn.chatgpt.com/docs/agent-configuration/subagents).
