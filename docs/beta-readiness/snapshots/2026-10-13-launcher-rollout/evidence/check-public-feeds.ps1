$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
$launcherSource = 'C:/Users/YaYa/Documents/ChatGPT/FPS-1/distribution/iron-sun-launcher'
$release = Invoke-RestMethod -Uri 'https://api.github.com/repos/Wilcox-Hub/iron-sun-releases/releases/latest' -TimeoutSec 30
$base = 'https://github.com/Wilcox-Hub/iron-sun-releases/releases/latest/download/'
$client = [Net.WebClient]::new()
$client.Headers['User-Agent'] = 'IronSunRolloutVerification/1'
$rsa = [Security.Cryptography.RSACryptoServiceProvider]::new()
$rsa.PersistKeyInCsp = $false
try {
    $rsa.FromXmlString([IO.File]::ReadAllText((Join-Path $launcherSource 'src/ReleasePublicKey.xml')))
    $feeds = @{}
    foreach ($kind in @('game', 'launcher')) {
        $bytes = $client.DownloadData($base + $kind + '-manifest.json')
        $signature = $client.DownloadData($base + $kind + '-manifest.sig')
        if (!$rsa.VerifyData($bytes, 'SHA256', $signature)) { throw "Invalid published $kind signature" }
        $manifest = [Text.Encoding]::UTF8.GetString($bytes) | ConvertFrom-Json
        $feeds[$kind] = $manifest
    }
    $setupEnvelope = Invoke-RestMethod -Uri 'https://raw.githubusercontent.com/Wilcox-Hub/iron-sun-releases/main/public/setup-feed.json' -TimeoutSec 30
    $setupBytes = [Convert]::FromBase64String($setupEnvelope.manifest_base64)
    if (!$rsa.VerifyData($setupBytes, 'SHA256', [Convert]::FromBase64String($setupEnvelope.signature_base64))) { throw 'Invalid setup feed signature' }
    $setup = [Text.Encoding]::UTF8.GetString($setupBytes) | ConvertFrom-Json
    $expectedTag = 'playtest-vs61-20261013-184128-652-launcher-1.0.1-setup-1'
    if ($release.tag_name -ne $expectedTag) { throw 'Latest release changed; reassess before testing' }
    if ($feeds.game.version -ne '20261013-184128-652' -or $feeds.game.protocol -ne 61) { throw 'Unexpected game feed' }
    if ($feeds.launcher.version -ne '1.0.1') { throw 'Unexpected launcher feed' }
    if (!$setup.url.Contains('/' + $expectedTag + '/')) { throw 'Setup feed and latest release differ' }
    foreach ($asset in @(
        @{ name = $feeds.game.archive; hash = $feeds.game.archive_sha256; size = $feeds.game.archive_size },
        @{ name = 'IronSunLauncher.exe'; hash = $feeds.launcher.sha256; size = $feeds.launcher.size_bytes },
        @{ name = 'IRON-SUN-Setup.exe'; hash = $setup.sha256; size = $setup.size_bytes }
    )) {
        $remote = @($release.assets | Where-Object { $_.name -eq $asset.name })
        if ($remote.Count -ne 1 -or $remote[0].size -ne $asset.size -or $remote[0].digest -ne ('sha256:' + $asset.hash.ToLowerInvariant())) {
            throw "Published asset mismatch: $($asset.name)"
        }
    }
    [ordered]@{
        passed = $true
        checked_utc = [DateTime]::UtcNow.ToString('o')
        release = $release.html_url
        game = $feeds.game
        launcher = $feeds.launcher
        setup = $setup
        game_launcher_setup_signatures_verified = $true
        published_asset_digests_verified = 3
        scope = 'Public HTTPS latest release, production-key-signed manifests and exact release asset digests; no private credentials or friend-PC access.'
    } | ConvertTo-Json -Depth 8
} finally {
    $rsa.Dispose()
    $client.Dispose()
}
