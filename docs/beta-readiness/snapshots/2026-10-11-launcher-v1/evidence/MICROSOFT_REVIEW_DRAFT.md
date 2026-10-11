# IRON SUN launcher — review submission draft

Status: not submitted. Owner approval to send the executable and public source link to Microsoft is pending.

Destination: [Microsoft Security Intelligence, software developer submission](https://www.microsoft.com/en-us/wdsi/filesubmission). [Microsoft's submission guidance](https://learn.microsoft.com/en-us/unified-secops/submission-guide) recommends the software developer category for incorrectly detected software.

Product: IRON SUN Launcher 1.0.1, Windows x64, C# / .NET Framework WPF.

Detection: `Trojan:Win32/Sabsik.TE.A!ml`, threat ID 2147780193. Defender blocked the browser download and a subsequent authenticated GitHub draft download. Both launcher releases were withdrawn to draft; no exclusions, quarantine restoration, endpoint-protection changes or security bypasses were used.

Exact tested binary identity:

- Filename: `IronSunLauncher.exe`
- Size: 1,785,344 bytes
- SHA256: `C293A7D60887B2D719C753E25E4AE8083A4299513A55FE9C23AD2249AF6378B4`
- Exact compiled source: https://github.com/Wilcox-Hub/iron-sun-releases/tree/8252cee924f7867ab1f1efeb15819e1e4ffcbfac

Proposed submission description:

> Please review a suspected false positive affecting our original game launcher. It downloads our signed release metadata and complete game archives from the project's GitHub Releases over HTTPS, checks RSA/SHA256 signatures and exact file inventory, stages installations under the current user's LocalAppData, retains a previous game build, and offers repair and rollback. The native self-updater verifies the next executable against separately signed metadata, waits for the current launcher to exit, replaces only its owned launcher file and reopens it. It creates the game's desktop shortcut. The 1.0.1 executable does not embed or execute PowerShell, request elevation, install a service, change Windows security settings, or include private signing keys, GitHub tokens or game-server invite credentials. Fourteen compiled updater integration cases passed, and a production-key-signed local self-update fixture replaced and reopened the launcher successfully. The live website download was blocked by Defender before game installation could begin. We have withheld distribution pending your review and a successful normal protected download/install test.

Do not submit player saves, private invite files, publisher private keys or unrelated system logs. A hash or source link alone is not evidence that Microsoft has cleared this executable. Save the real submission ID and disposition when a submission succeeds; do not relabel this draft as a submitted case.
