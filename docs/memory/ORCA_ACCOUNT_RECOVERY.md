# Orca account-switch recovery — October 20, 2026

The owner reported that IRON SUN — Talk Here stopped working after switching to the other ChatGPT account. Orca 1.4.232 was healthy, but the tab contained a plain PowerShell prompt with no Codex agent. The selected managed account was recognized as ChatGPT Plus and its usage query succeeded. This was an exited/unresumed assistant, not evidence that the account itself was unusable. The exact reason the earlier restart failed was not established.

## Verified repair

In the existing idle shell, with the intended account already selected in Orca, ran:

```powershell
codex resume 01a14bab-ce64-7530-bb8b-0eae10c928bd
```

Codex reopened the same session, previous game history and GPT-6.1 Sol/xhigh settings. `/status` showed the selected managed Plus account and the original session ID. A fresh no-tools verification prompt returned **IRON SUN CHAT RESTORED**; its saved rollout contains both the assistant response and completed-turn record. The view was returned to the latest reply and the tab renamed IRON SUN - Talk Here. No game task, account reauthentication, credit purchase or model change was performed.

Use this fallback only after confirming the tab is an idle shell and no other process is writing to the session. The account selector and the running process can differ until restart. Inspect `/status` after resuming. A usage-limit message restored from history is not a new rejection; test a fresh, bounded reply. If the active view is browsing earlier messages, its displayed Enter/Escape action returns to the latest response.

This verifies Windows A → B exact-session recovery by manual resume. It does not prove the automatic restart button or B → A return. Keep the saved conversation; do not create a replacement chat just to clear old error messages.

## Separate memory warnings

The successful verification turn showed two hooks timing out after 3 seconds. Mem0's status reported capture active, a configured key, repository scope `https://github.com/Wilcox-Hub/fps-game`, user ID YaYa, 1,107 events, 86 completed flushes and 20 retrievals. Its doctor passed Python, data directory, repository, key configuration and live authentication (461 ms). These are diagnostic observations, not proof that the timed-out automatic hooks captured the new turn. Recent plugin error logs also contain Unicode encoding failures; causation for the timeout was not established. Hook settings/capture and credentials were preserved. Canonical file-based project memory remains the continuation record.

Evidence: `C:/Users/YaYa/Documents/ChatGPT/FPS-1/output/orca-setup/2026-10-20/account-repair/verification.json` and the selected account's saved rollout for session `01a14bab-ce64-7530-bb8b-0eae10c928bd`. No raw credentials or email addresses are included in the repair receipt.

References: [Orca account switching](https://www.onorca.dev/docs/agents/codex-hot-swap), [OpenAI authentication](https://learn.chatgpt.com/docs/auth).

Follow-up: [hook repair](MEM0_HOOK_REPAIR.md) fixes UTF-8 input handling and adds timeout headroom/background capture. Installed October 20; seven modified definitions require owner trust before live verification. The earlier diagnostic counts above retain their original scope.
