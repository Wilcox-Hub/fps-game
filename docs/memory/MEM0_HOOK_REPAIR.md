# Mem0 hook repair — October 20, 2026

Status: October25 verified all8 Mem0 hooks enabled and trusted, including the7 changed definitions; fresh automatic capture and connection checks pass. Owner approvals were preserved; no trust bypass.

The owner's pasted log contains six explicit three-second timeout messages while game preparation continued. The game chat was left running throughout diagnosis.

## Changes

- `core/hook_runner.py`: read hook JSON from the binary stdin buffer, allowing Python's JSON decoder to interpret its UTF-8 bytes independently of Windows' legacy text encoding. Preserve text-stream fallback for existing callers. Hook commands and trust settings were unchanged by this code fix.
- `hooks/hooks.json`: increase the seven ordinary lifecycle hook budgets from 3 to 15 seconds. Run capture-only PostToolUse, Stop and SubagentStop handlers in the background. Keep retrieval/context-producing hooks synchronous. SessionEnd stays at 3 seconds, Codex's documented maximum. All command strings remain identical, and no hooks or memory capture setting were disabled.
- Existing hook trust permissions were preserved. At initial October20 installation, Codex's `hooks/list` reported the seven changed definitions as `modified`; SessionEnd remained trusted. New or changed hooks are skipped until the owner reviews them using `/hooks`. The account's current game turn may keep its earlier hook snapshot until settings reload or the next turn.

## Evidence

- `probe_hooks.py --powershell --legacy-encoding` reproduced UnicodeEncodeError with `\udc9d` on curly quotes and failed to preserve the captured text. This matches the class of errors in the real plugin log. Initial tests that checked only exit code were insufficient: the plugin catches failures and exits zero. The final check asserts that the intended text was actually saved.
- Following the input fix, Unicode text and tool-result capture passed, including with legacy Windows pipe encoding.
- Controlled four-second SQLite write contention reproduced the three-second cutoff in all three cases (`fixtures/9a885115/results.json`). The same three cases passed with the 15-second budget in 4.086–4.094 seconds, preserving Unicode and recording events without errors (`fixtures/bb42cd15/results.json`). This validates headroom for transient contention; it does not establish that every pasted timeout had that exact cause.
- A separate installed-Codex diagnostic turn under the managed account completed with its baseline SessionStart/UserPromptSubmit/Stop hooks in about 0.2–0.4 seconds. This demonstrates the timeout is intermittent. No-tools Luna/xhigh diagnostic work was kept separate from the ongoing game chat.
- Installed definitions were parsed through Codex 0.163.0's real protocol. Changing timing changes the hook hash, confirmed first in an isolated home. No trust hashes were edited or approval checks bypassed.
- Mem0 status/doctor after installation: capture configuration unpaused, key configured, user YaYa, repository `https://github.com/Wilcox-Hub/fps-game`, 1,266 events, 89 completed flushes, 25 retrievals. All doctor checks passed, including authentication in 361 ms. These counters and connectivity checks do not prove execution of the newly modified, untrusted hooks.

Backups: `before-hook_runner.py`, `before-hook-definitions.json`. Receipts: `encoding-patch.json`, `timing-patch.json`, `installed-pending-review-hooks.json`. The local plugin patch can be replaced by a future plugin update; check the installed version/behavior after updating.

October20 follow-up: the owner approved the concrete comparison: unchanged commands, seven timeout increases to15seconds and background capture for PostToolUse/Stop/SubagentStop. The main agent opened Review hooks in the blocked helper's TUI and used its advertised trust-all action. The review warning disappeared, listed installed/active counts matched, and fresh worker startup no longer stopped at the hook prompt. The unused blocked worker was released; no trust hash was edited or review bypassed.

Next: continue normal work; recheck installed behavior after a plugin update because the local patch can be replaced. October25 verification below closes the pending automatic-capture check.

Primary source: [OpenAI Hooks documentation](https://learn.chatgpt.com/docs/hooks), especially hook trust, background execution, and timeout limits. The original account/chat recovery remains successful; its memory-hook limitation is tracked here.


## October25 live verification

Codex `hooks/list` confirms all8 Mem0 hooks are enabled and trusted, including the7 changed definitions. The7 ordinary budgets remain15seconds; SessionEnd stays3seconds; background PostToolUse/Stop/SubagentStop preserved. Fresh `tool_result` events were saved by the active Orca session at20:53Pacific (October26 03:53UTC), independently confirming automatic capture after trust. No additional model prompt or task content was needed for verification. The plugin error log has no newer entries since October20 19:15Pacific.

Mem0 is unpaused, key configured, userYaYa; IRON SUN repository scope remains `https://github.com/Wilcox-Hub/fps-game`. Status snapshot:7125events,181completed flushes,145retrievals. All doctor checks pass, including authentication436ms. These prove current capture/connectivity; each lifecycle event was not separately triggered. Receipt: `C:/Users/YaYa/Documents/ChatGPT/FPS-1/output/orca-setup/2026-10-20/hook-repair/approved-verification-2026-10-25.json`.
