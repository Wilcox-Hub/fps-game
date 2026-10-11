# IRON SUN downloads and launcher status

Share https://iron-sun-playtest.wilcoxwarrior4577.chatgpt.site with friends. The page works on phones; the game runs on Windows x64.

For now, download the full VS58 game ZIP, extract it completely, and run `Windows/TP_FirstPerson.exe` inside its folder. Keep every game file together. This temporary path uses manual updates. Obtain the server address and private invite separately from the host; no invite or publisher secret is embedded in the public download.

The custom launcher is implemented and its native integration/self-update fixtures pass. Windows Defender blocks its executable, so both launcher releases are withheld pending security review and a successful protected download/install test. Do not disable protection or add exclusions. The existing owner desktop LAN launcher remains in place.

Launcher source, build/test tools and verified publisher: https://github.com/Wilcox-Hub/iron-sun-releases

When approved, the launcher will install the complete game, check published updates at startup and every ten idle minutes, preserve saves, offer repair/rollback and update itself. Source edits reach friends only after they are built, tested and published as a compatible game release. A source commit alone does not deploy a game or server.

The publisher requires the recorded tested Shipping package and matching native-server protocol. `-GameOnly` is the temporary manual path. See that repository's README for the exact owner command and signing-key handling.

Read `docs/beta-readiness/latest.json` before selecting new tests. Launcher readiness is separate from gameplay beta approval; the dated 69% estimate and 75% target remain unchanged.
