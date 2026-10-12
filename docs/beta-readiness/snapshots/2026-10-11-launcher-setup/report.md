# Windows setup delivery — October 11, 2026

The public download previously delivered a raw Unreal game ZIP: it contained the complete `Windows/` tree and `TP_FirstPerson.exe`, with no installer, app registration, Start menu entry or automatic updates. The label “Download for Windows” did not make that distinction clear enough.

The intended friend-facing file is now **IRON-SUN-Setup.exe**. It installs **IRON SUN Launcher** per user, registers it in Windows Installed Apps, adds a Start menu shortcut, offers an optional desktop shortcut, and opens the launcher at the end of interactive setup. The browser still requires the user to open setup once. The launcher then installs and updates the game. The existing owner LAN shortcut is not migrated.

Installer source: https://github.com/Wilcox-Hub/iron-sun-releases/tree/df48e5d0ae2c6132196a945afc8214aa894bbe0b

The real Inno Setup installer/uninstaller was compiled with an inert fixture executable. Fourteen recorded assertions pass, including exact-root ownership, foreign-folder rejection, shortcuts and app registration, opening from the shortcut, repair/save retention, preventing an old setup from downgrading a newer launcher, optional desktop shortcut and uninstall. Production directives also compile with the fixture. The interactive completion-window launch was configured but not accepted with the real product. See `setup-integration.json`.

Ten actual website release-gate checks pass with ephemeral RSA fixtures. The page only enables setup when a signed manifest records accepted production delivery and matches the public release asset's exact URL, size and digest. Raw EXEs, raw game ZIPs, unaccepted metadata and changed signatures do not activate it. The publisher rejects launcher promotion before artifacts, keys or network access when delivery approval is absent. The signature/source-feed path uses GitHub's public contents API because release download redirects do not support browser CORS. No accepted feed was published.

**Production setup is not built or released.** Defender refused access to and quarantined the original launcher. No original launcher was executed, restored, excluded or wrapped into a publicly distributed installer. The production setup build was stopped before packaging. Both previous launcher releases remain drafts, and the public game ZIP remains an explicitly manual fallback.

Microsoft submission **a42bd1fa-88a6-4c46-b7ef-d3cd5e020541** succeeded on October 11 at 8:36:44 PM Pacific. The authenticated case receipt says **Submitted**, with final determination **Pending**. It is not a clearance or beta approval. See `microsoft-review-submission.json` and [the account's case page](https://www.microsoft.com/en-us/wdsi/submission/a42bd1fa-88a6-4c46-b7ef-d3cd5e020541). Owner-approved Services Agreement, portal permissions and CAPTCHA were completed. The built-in browser attached the encrypted archive without changing Chrome's blocked extension setting or Windows protection.

Microsoft's documented encrypted review archive was prepared for the authorized analysis submission. It contains only unchanged `IronSunLauncher.exe` (1,785,344 bytes, SHA256 `C293A7D60887B2D719C753E25E4AE8083A4299513A55FE9C23AD2249AF6378B4`) from the existing private draft asset. Only the encrypted ZIP was written; the executable was not extracted or run. The archive is for Microsoft analysis, never a friend-facing download. See `microsoft-review-archive.json`.

The corrected page is live at https://iron-sun-playtest.wilcoxwarrior4577.chatgpt.site, source `c24f12f79ebb1f1fa67f4729c24e3c481e5cc53c`, saved version 5, deployment `appgdep_6acc34af74048191a639db625dd04a31` succeeded. Local final UI was inspected at 1920×1080 with no horizontal overflow, disabled launcher setup, no ZIP linked from the primary button, working copy link and explicit manual instructions. No new game execution or friend-PC acceptance is claimed.

All 600 game inputs match the cached readiness evidence. No game test, gameplay score or beta acceptance was added. The October 4 low-confidence 69% gameplay estimate and NOT_BETA_READY verdict remain dated and unchanged.

Canonical readiness metadata includes this dated report, scoped checks, source/evidence hashes and the latest pointer. An earlier automatic approval-service quota failure cleared after its reset. Unrelated game source remains unpublished; this assessment adds no gameplay score or whole-game acceptance.

Remaining delivery work: obtain Microsoft's final determination; accept the exact production binary through Windows protection; compile setup; perform normal downloaded setup/open/install/play/uninstall testing; record exact binary hashes and evidence; publish the gated setup release and feed; verify the live one-file download on a friend PC.
