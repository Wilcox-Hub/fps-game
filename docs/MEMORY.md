# Mem0 Project Memory

Git-tracked files and GitHub issues are the canonical project record. Mem0 is an optional searchable aid for durable decisions and verified outcomes; it must not overrule the current request or repository.

## Codex installation

Use the official Mem0 Codex plugin (currently installed for this Windows user as version `0.3.3`). Its plugin manifest registers its own `mem0` MCP server and lifecycle hooks. Do **not** add a second, direct `[mcp_servers.mem0]` server while this plugin is installed.

To install or recover it in Codex:

```powershell
codex plugin marketplace add mem0ai/mem0
codex plugin add mem0@mem0-plugins
```

Restart Codex after install or configuration changes. `codex plugin marketplace upgrade` updates the marketplace; `codex plugin list` shows installation state. The plugin uses Python 3.10+ and a Mem0 Platform API key (`m0-…`). Set `MEM0_API_KEY` in the Windows user environment using the system's Environment Variables UI, then restart Codex. Never put the key in Git, project config, command examples, logs, or chat. If the plugin exposes an interactive authorization flow in a future version, use that rather than adding a duplicate MCP server.

## Scope and use

- Keep memories limited to this FPS repository. The plugin's default search scope is repository-scoped; use its `search_memories` tool or `/mem0:search` at task start and again before major design changes. Narrow to `dir` when a decision belongs only to a subproject such as `unreal/Ridgefire`. Check that the Codex task's workspace is this Git checkout: the tool scopes to the task workspace, not to a different directory selected by a shell command.
- Capture only durable user decisions, constraints, project conventions, and verified outcomes. Update or deduplicate stale entries. Do not save secrets, credentials, unrelated personal information, or large transcripts.
- Distinguish user-stated choices from agent proposals and test evidence. Record uncertainty and the validation level (source, build, smoke, rendered, packaged, or human test).
- GitHub files remain canonical. Promote important decisions and evidence into tracked docs or issues rather than relying on memory alone.

## Verification and recovery

To verify memory, save a harmless project fact (for example, “Iron Sun's first-arena sentries should be tall armored giants”), restart Codex or use a fresh Codex session, and retrieve it with a Mem0 search. Confirm the returned memory/tool result contains the fact; do not count an assistant merely repeating prior chat context as verification. Check `/mem0:status` if available. If hooks or tools do not appear, restart Codex and check `codex plugin list`; if authentication fails, confirm `MEM0_API_KEY` is set in the environment of the Codex process without printing its value.

Verification on 2026-09-27: the official plugin's status/doctor checks identified `https://github.com/Wilcox-Hub/fps-game` and authenticated successfully. From this checkout, its core capture/flush saved the harmless user-confirmed first-arena sentry constraint (`semantic-succeeded`, one memory). A separate fresh Python process using the same plugin core and repository scope retrieved that fact. This verifies repository-scoped save/retrieval, not automatic hook behavior in a newly opened Codex game task. The verification chat's workspace was a different folder, so its MCP `search_memories` results were not proof of FPS-repo scope; future game tasks should open this checkout directly and confirm the scope there.

Official setup reference: [Mem0 Codex integration guide](https://github.com/mem0ai/mem0/blob/main/docs/integrations/codex.mdx).
