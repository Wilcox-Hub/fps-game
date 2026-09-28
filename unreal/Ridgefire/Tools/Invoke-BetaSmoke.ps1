param(
    [string]$EngineRoot = 'G:\Unreal Engine\UE_5.8',
    [int]$TimeoutSeconds = 90
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$project = (Resolve-Path (Join-Path $PSScriptRoot '..\Ridgefire.uproject')).Path
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not (Test-Path -LiteralPath $editor)) {
    throw "Unreal Editor not found: $editor"
}

$logs = Join-Path (Split-Path $project) 'Saved\Logs'
New-Item -ItemType Directory -Path $logs -Force | Out-Null
$log = Join-Path $logs ("IRON-SUN-Beta-Smoke-{0}.log" -f (Get-Date -Format 'yyyyMMdd-HHmmss'))
$arguments = @(
    ('"' + $project + '"'),
    '/Game/Variant_Shooter/Lvl_Shooter',
    '-game', '-NullRHI', '-NoSound', '-unattended', '-nop4', '-nosplash',
    '-RidgefireSmokeTest',
    ('-abslog="' + $log + '"')
)

$process = Start-Process -FilePath $editor -ArgumentList $arguments -PassThru -WindowStyle Hidden
$deadline = (Get-Date).AddSeconds($TimeoutSeconds)
$completionMarker = 'RIDGEFIRE SMOKE PASS: restart restores a living player'
try {
    do {
        Start-Sleep -Milliseconds 500
        if (Test-Path -LiteralPath $log) {
            $content = Get-Content -LiteralPath $log -Raw
            if ($content.Contains($completionMarker)) {
                break
            }
            if ($content -match 'Fatal error:|Assertion failed:') {
                throw "Unreal reported a fatal error or assertion. Log: $log"
            }
        }
        if ($process.HasExited) {
            throw "Unreal exited before the final restart check. Log: $log"
        }
    } while ((Get-Date) -lt $deadline)

    if ((Get-Date) -ge $deadline) {
        throw "Timed out after $TimeoutSeconds seconds before the final restart check. Log: $log"
    }

    $failures = ([regex]::Matches($content, 'RIDGEFIRE SMOKE FAIL:')).Count
    $requiredOutcomes = @(
        'RIDGEFIRE SMOKE PASS: the bound mouse fire input creates a visible muzzle-origin beam.',
        'RIDGEFIRE SMOKE PASS: firing consumed ammunition',
        'RIDGEFIRE SMOKE PASS: switching between carried primaries preserves each gun''s ammunition',
        'RIDGEFIRE SMOKE PASS: an aimed player shot damages a naturally positioned visible sentinel.',
        'RIDGEFIRE SMOKE PASS: a sentry energy shot damages the player.',
        'RIDGEFIRE SMOKE PASS: first wave cleared and wave two spawned seven sentinels.',
        'RIDGEFIRE SMOKE PASS: wave-clear transition entered Brassfall and registered its one-time furnace anchor objective.',
        'RIDGEFIRE SMOKE PASS: wave clear replenished reserve ammunition.',
        'RIDGEFIRE SMOKE PASS: player defeat enters game over without destroying the pawn.',
        'RIDGEFIRE SMOKE PASS: defeated player remains on game-over screen without an automatic respawn.',
        'RIDGEFIRE SMOKE PASS: pause/back on host game-over keeps the game running, unpaused, and cursor-accessible.',
        $completionMarker
    )
    $missingOutcomes = @($requiredOutcomes | Where-Object { -not $content.Contains($_) })
    $ammoMatch = [regex]::Match($content, 'RIDGEFIRE SMOKE PASS: firing consumed ammunition \((\d+) -> (\d+)\)\.')
    $ammoDecreased = $ammoMatch.Success -and ([int]$ammoMatch.Groups[2].Value -lt [int]$ammoMatch.Groups[1].Value)
    if (-not $ammoDecreased) {
        $missingOutcomes += 'numeric ammunition decrease (starting ammo > remaining ammo)'
    }
    if ($failures -ne 0 -or $missingOutcomes.Count -gt 0) {
        $missing = if ($missingOutcomes.Count -gt 0) { $missingOutcomes -join '; ' } else { '<none>' }
        throw "Smoke outcomes failed or were incomplete (failures=$failures; missing=$missing). Log: $log"
    }

    Write-Output "Editor smoke passed: required firing/ammo, player and enemy health, wave, arena transition, defeat, and restart outcomes observed; failures=$failures. Log: $log"
}
finally {
    if (-not $process.HasExited) {
        Stop-Process -InputObject $process
    }
}
