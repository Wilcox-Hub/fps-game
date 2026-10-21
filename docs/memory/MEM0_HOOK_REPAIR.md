# Mem0 hook repair — October 20, 2026

Status: code and timing fix installed; seven changed hook definitions await the owner's required Codex review. Do not report live automatic capture as verified until the updated definitions are trusted and a fresh callback succeeds.

The owner's pasted log contains six explicit three-second timeout messages while game preparation continued. The game chat was left running throughout diagnosis.

## Changes

- `core/hook_runner.py`: read hook JSON from the binary stdin buffer, allowing Python's JSON decoder to interpret its UTF-8 bytes independently of Windows' legacy text encoding. Preserve text-stream fallback for existing callers. Hook commands and trust settings were unchanged by this code fix.
- `hooks/hooks.json`: increase the seven ordinary lifecycle hook budgets from 3 to 15 seconds. Run capture-only PostToolUse, Stop and SubagentStop handlers in the background. Keep retrieval/context-producing hooks synchronous. SessionEnd stays at 3 seconds, Codex's documented maximum. All command strings remain identical, and no hooks or memory capture setting were disabled.
- Existing hook trust permissions were preserved. Codex's actual `hooks/list` response reports the seven changed definitions as `modified`; SessionEnd remains trusted. New or changed hooks are skipped until the owner reviews them using `/hooks`. The account's current game turn may keep its earlier hook snapshot until settings reload or the next turn.

## Evidence

- `probe_hooks.py --powershell --legacy-encoding` reproduced UnicodeEncodeError with `\udc9d` on curly quotes and failed to preserve the captured text. This matches the class of errors in the real plugin log. Initial tests that checked only exit code were insufficient: the plugin catches failures and exits zero. The final check asserts that the intended text was actually saved.
- Following the input fix, Unicode text and tool-result capture passed, including with legacy Windows pipe encoding.
- Controlled four-second SQLite write contention reproduced the three-second cutoff in all three cases (`fixtures/9a885115/results.json`). The same three cases passed with the 15-second budget in 4.086–4.094 seconds, preserving Unicode and recording events without errors (`fixtures/bb42cd15/results.json`). This validates headroom for transient contention; it does not establish that every pasted timeout had that exact cause.
- A separate installed-Codex diagnostic turn under the managed account completed with its baseline SessionStart/UserPromptSubmit/Stop hooks in about 0.2–0.4 seconds. This demonstrates the timeout is intermittent. No-tools Luna/xhigh diagnostic work was kept separate from the ongoing game chat.
- Installed definitions were parsed through Codex 0.163.0's real protocol. Changing timing changes the hook hash, confirmed first in an isolated home. No trust hashes were edited or approval checks bypassed.
- Mem0 status/doctor after installation: capture configuration unpaused, key configured, user YaYa, repository `https://github.com/Wilcox-Hub/fps-game`, 1,266 events, 89 completed flushes, 25 retrievals. All doctor checks passed, including authentication in 361 ms. These counters and connectivity checks do not prove execution of the newly modified, untrusted hooks.

Backups: `before-hook_runner.py`, `before-hook-definitions.json`. Receipts: `encoding-patch.json`, `timing-patch.json`, `installed-pending-review-hooks.json`. The local plugin patch can be replaced by a future plugin update; check the installed version/behavior after updating.

Next: owner opens `/hooks` in Orca and reviews/trusts the seven modified Mem0 definitions; verify trust through `hooks/list`, then run fresh callbacks and inspect saved capture plus failures. If the current game turn still uses its old snapshot, wait for it to finish before refreshing the same saved session. Account authentication, game code and builds were not changed by this repair.

Primary source: [OpenAI Hooks documentation](https://learn.chatgpt.com/docs/hooks), especially hook trust, background execution, and timeout limits. The original account/chat recovery remains successful; its memory-hook limitation is tracked here.
