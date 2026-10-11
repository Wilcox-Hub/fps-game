param([string]$PackagesRoot = '', [string]$ExpectedArtifacts = '')
$ErrorActionPreference = 'Stop'
$gameRoot = 'C:\Users\YaYa\Documents\Codex\2026-09-22\connect-this-github-repo-to-a'
$launcher = Join-Path $gameRoot 'unreal\Ridgefire\Tools\Play-Latest-WindowsPackage.ps1'
$arguments = @{ ListOnly = $true }
if ($PackagesRoot) { $arguments.PackagesRoot = $PackagesRoot }
$output = & $launcher @arguments
$output
$line = @($output | Where-Object { $_ -like 'Bootstrap: *' })
if ($line.Count -ne 1) { throw 'No unique player-launch bootstrap' }
$bootstrap = $line[0].Substring('Bootstrap: '.Length)
$windowsRoot = Split-Path -Parent $bootstrap
if (!$ExpectedArtifacts) { $ExpectedArtifacts = Join-Path $gameRoot 'docs\beta-readiness\snapshots\2026-10-05-vs44\package-artifacts.json' }
$evidence = Get-Content -LiteralPath $ExpectedArtifacts -Raw | ConvertFrom-Json
foreach ($artifact in $evidence.artifacts) {
 $path = Join-Path $windowsRoot $artifact.path
 if (!(Test-Path -LiteralPath $path)) { throw ('Missing player build artifact: ' + $artifact.path) }
 $actual = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
 if ($actual -ne $artifact.sha256) { throw ('Player launcher selects an older/different build: ' + $artifact.path + '; expected tested artifact hash does not match') }
}
Write-Output 'PASS: default player launcher selects the exact expected Shipping build (all five artifact hashes match)'
