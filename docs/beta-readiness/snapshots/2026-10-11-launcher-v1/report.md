# IRON SUN launcher implementation and delivery hold

The Windows launcher and public download website are implemented. The launcher is **not approved for friend distribution**: Defender blocked the downloaded 1.0.0 and native 1.0.1 executables. Both launcher releases are drafts again. Microsoft review authorization is pending; no submission has been made. Windows protection was left enabled, without exclusions or quarantine restoration.

## Implemented and verified

- Branded per-user WPF launcher, complete game installation, automatic idle updates, repair, rollback and preservation of files outside the game payload.
- Native C# HTTPS, RSA/SHA256 verification, bounded downloads, exact ZIP inventory, safe owned paths, staging and recoverable directory swaps. No embedded PowerShell or execution-policy changes.
- All 14 integration cases passed through the actual compiled 1.0.1 updater with signed local fixtures. Their scope excludes live networking, game execution and human acceptance.
- Signed native self-update fixture 1.0.1 → 1.0.2 replaced the launcher after exit, preserved the previous binary and rendered the new window. The 1.0.2 fixture is not a published release.
- Publisher verifies the full tested client inventory, archive identity and matching native server protocol, signs metadata, checks GitHub asset digests and promotes complete immutable releases.
- The public phone-friendly website offers the complete 440,541,545-byte VS58 ZIP as a temporary manual download. Signed metadata and unauthenticated HTTPS ZIP availability passed; its signed archive identity matches the verified local package. Automatic updates are explicitly unavailable for this fallback.

Website: https://iron-sun-playtest.wilcoxwarrior4577.chatgpt.site

Source: https://github.com/Wilcox-Hub/iron-sun-releases

Public game-only release: https://github.com/Wilcox-Hub/iron-sun-releases/releases/tag/playtest-vs58-20261011-120705-452-game

## Release blocker and next gate

Native launcher 1.0.1 SHA256: `C293A7D60887B2D719C753E25E4AE8083A4299513A55FE9C23AD2249AF6378B4`. Defender detection: `Trojan:Win32/Sabsik.TE.A!ml`. A native rewrite did not resolve the protected download block; a false-positive suspicion is not a Microsoft clearance.

After owner approval, submit the executable and exact public source link to Microsoft's software-developer review. Preserve its real case ID and disposition. Before republishing, verify a normal protected website download, full native HTTPS install, game startup, owner shortcut migration and a friend-PC update. No code-signing purchase, new account or security exception has been authorized. The existing owner LAN updater was preserved.

## Readiness and cache boundaries

All 600 game input hashes still match VS58. Existing engine/server checks retain their recorded dates and limits; VS57 historical groups remain historical. No new engine tests or human playtests ran for this launcher task. Earlier script-driver HTTPS installation evidence is kept as historical evidence and is not relabeled as native launcher delivery.

The October 4 gameplay estimate remains **69%**, low confidence. Overall beta readiness remains unmeasured and the verdict remains **NOT_BETA_READY**. The 75% target, normal different-Wi-Fi human combat/recovery, full-run pacing, performance, arena art and owner acceptance remain open. Launcher test pass rates do not add gameplay readiness points.

Use `launcher-input-manifest.json` for launcher source/test/tool cache reuse and `input-manifest.json` for the unchanged game. Reuse every test only at its actual validation level.
