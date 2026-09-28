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

- Keep memories limited to this FPS repository. The plugin's default search scope is repository-scoped; use its `search_memories` tool or `/mem0:search` at task start and again before major design changes. Narrow to `dir` when a decision belongs only to a subproject such as `unreal/Ridgefire`.
- Capture only durable user decisions, constraints, project conventions, and verified outcomes. Update or deduplicate stale entries. Do not save secrets, credentials, unrelated personal information, or large transcripts.
- Distinguish user-stated choices from agent proposals and test evidence. Record uncertainty and the validation level (source, build, smoke, rendered, packaged, or human test).
- GitHub files remain canonical. Promote important decisions and evidence into tracked docs or issues rather than relying on memory alone.

## Verification and recovery

To verify memory, save a harmless project fact (for example, “Iron Sun's first-arena sentries should be tall armored giants”), restart Codex or use a fresh Codex session, and retrieve it with a Mem0 search. Confirm the returned memory/tool result contains the fact; do not count an assistant merely repeating prior chat context as verification. Check `/mem0:status` if available. If hooks or tools do not appear, restart Codex and check `codex plugin list`; if authentication fails, confirm `MEM0_API_KEY` is set in the environment of the Codex process without printing its value.

As of the harness setup inspection, the official plugin was installed in Codex, but `MEM0_API_KEY` was absent from the current process and the Windows user environment. No memory round-trip was verified. Complete key setup and the fresh-session test before describing Mem0 as live.

Official setup reference: [Mem0 Codex integration guide](https://github.com/mem0ai/mem0/blob/main/docs/integrations/codex.mdx).
