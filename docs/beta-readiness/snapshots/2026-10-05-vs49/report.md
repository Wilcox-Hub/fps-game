# VS49 - Approved home LAN firewall rule active

The owner explicitly approved the previously prepared runtime-specific LAN exception. The elevated helper applied it, and a separate ActiveStore query verifies the rule is enabled with status OK: inbound UDP ports 7777 (default gameplay) and 14001 (LAN discovery), the exact current Shipping runtime, Private networks, LocalSubnet and edge traversal Block. The small launcher/bootstrap alone was not given the rule. [Applied output](firewall-applied.json), [independent verification](firewall-active-verification.json).

This resolves the missing game-runtime permission on the owner PC. It does not prove the earlier two-PC discovery failure is fixed. The owner should create a fresh two-player lobby and have the second PC refresh/join. That PC's firewall/profile and router client isolation remain uninspected. The unchanged small LAN-Setup ZIP is available separately for another PC if needed; moving the game to a different path requires an exception for that new runtime location.

VS48's actual packaged-host/same-PC Editor discovery/join pass is reused only at its recorded date and scope; it happened before rule application. All 562 game inputs, the helper and five packaged artifacts still match. No game source, content, build or engine-test changes. [Current checks](test-results.json), [scoped reuse](scoped-reuse.json).

Keep the October 4 VS41 low-confidence 69% (68.5 weighted) planning estimate, no readiness credit or beta approval. Two-PC LAN remains an open acceptance gate. Source/tool/docs/records remain local/unpublished; the operating-system rule is active on the owner PC.
