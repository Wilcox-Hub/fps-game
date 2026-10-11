# VS48 - Packaged LAN discovery report and Windows firewall setup

Owner reports no party appearing between two Windows PCs despite matching extracted builds, the same home network, host lobby and refresh. No firewall prompt appeared. This report remains an open cross-device acceptance failure.

The current VS47 Shipping bootstrap was launched as a hidden NullRHI automated host. A real local Editor Null/LAN peer discovered one party at the Wi-Fi endpoint, joined through the session subsystem and reached replicated lobby. This checks the actual packaged hosting path, but not two-device discovery, packaged-client discovery, human play or router delivery. No game-code fix is justified from this local pass. [Exact scope and result](test-results.json).

Read-only owner-PC inspection finds a Private Wi-Fi profile, disabled firewall listen notifications and no matching game-runtime/unrestricted inbound UDP exception. The absent prompt is explained by notifications being off. A missing exception is a candidate blocker for traffic from another PC; packet drops between the two machines were not captured and the root cause remains unconfirmed. Tailscale exists, but this probe advertised the correct Wi-Fi endpoint. [Observations](firewall-observations.json), [diagnosis](diagnosis.json).

The new Tools/Configure-LANFirewall.ps1 previews a rule by default. Apply requires administrator permission and adds only an inbound UDP rule for the extracted runtime, ports 7777 and 14001, Private profiles and LocalSubnet. It does not change global firewall settings, adapters, VPN, router configuration or network categories. It validates an existing same-name rule before enabling/removing it. Custom game ports need their own explicit scope. [Rule preview](firewall-plan.json), [Microsoft rule semantics](https://learn.microsoft.com/en-us/powershell/module/netsecurity/new-netfirewallrule?view=windowsserver2025-ps).

Scoped exception prepared and previewed; administrator authorization/application and two-PC retry pending.

After applying, the owner must retry fresh lobby discovery/join from the second PC. If still absent, inspect the second-PC firewall/profile and router client isolation. Do not treat localhost success as approval for two-device LAN, Steam/Internet or beta.

All 562 game input hashes and five current packaged executable/content hashes still match VS47. No game source/content changes or rebuilds. The original transfer ZIP remains valid; the small firewall helper can be delivered separately. Retain VS41's October 4 low-confidence 69% (68.5 weighted) planning estimate and NOT_BETA_READY, with no new score credit. Tool, docs and records remain local/unpublished.
