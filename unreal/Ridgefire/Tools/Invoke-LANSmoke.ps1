param(
    [string]$EngineRoot = 'G:\Unreal Engine\UE_5.8',
    [ValidateRange(2, 4)]
    [int]$PlayerCount = 2,
    [ValidateRange(1024, 65535)]
    [int]$Port = 7777,
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
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$hostLog = Join-Path $logs "IRON-SUN-LAN-$stamp-Host.log"
$clientLogs = @()
$processes = @()

function Read-Log([string]$Path) {
    if (Test-Path -LiteralPath $Path) {
        return Get-Content -LiteralPath $Path -Raw
    }
    return ''
}

function Get-ArenaState([string]$Content, [string]$Role) {
    $pattern = "(?m)RIDGEFIRE ARENA STATE role=$Role kind=([^ ]+) revision=(\d+) components=(\d+) center=(.*?) ground=([-\d.]+)"
    $stateMatches = [regex]::Matches($Content, $pattern)
    if ($stateMatches.Count -eq 0) {
        return $null
    }
    return $stateMatches[$stateMatches.Count - 1]
}

function Format-ArenaState([System.Text.RegularExpressions.Match]$State) {
    return "kind=$($State.Groups[1].Value), revision=$($State.Groups[2].Value), components=$($State.Groups[3].Value), center=$($State.Groups[4].Value), ground=$($State.Groups[5].Value)"
}

try {
    $hostArguments = @(
        ('"' + $project + '"'), '/Game/Variant_Shooter/Lvl_Shooter?listen',
        '-game', "-port=$Port", '-NullRHI', '-NoSound', '-unattended', '-nop4', '-nosplash',
        ('-abslog="' + $hostLog + '"')
    )
    $hostProcess = Start-Process -FilePath $editor -ArgumentList $hostArguments -PassThru -WindowStyle Hidden
    $processes += $hostProcess
    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    do {
        Start-Sleep -Milliseconds 500
        $hostContent = Read-Log $hostLog
        if ($hostContent -match "IpNetDriver listening on port $Port") {
            break
        }
        if ($hostProcess.HasExited -or $hostContent -match 'Fatal error:|Assertion failed:') {
            throw "Listen host failed before accepting clients. Log: $hostLog"
        }
    } while ((Get-Date) -lt $deadline)
    if ((Get-Date) -ge $deadline) {
        throw "Listen host did not open port $Port. Log: $hostLog"
    }

    for ($clientIndex = 1; $clientIndex -lt $PlayerCount; ++$clientIndex) {
        $clientLog = Join-Path $logs "IRON-SUN-LAN-$stamp-Client$clientIndex.log"
        $clientLogs += $clientLog
        $clientArguments = @(
            ('"' + $project + '"'), "127.0.0.1:$Port", '-game', '-NullRHI', '-NoSound',
            '-unattended', '-nop4', '-nosplash', ('-abslog="' + $clientLog + '"')
        )
        $processes += Start-Process -FilePath $editor -ArgumentList $clientArguments -PassThru -WindowStyle Hidden
        Start-Sleep -Seconds 1
    }

    $arenaStateDiagnostics = @()
    $arenaStateParity = $false
    do {
        Start-Sleep -Milliseconds 500
        $hostContent = Read-Log $hostLog
        $joinCount = ([regex]::Matches($hostContent, 'LogNet: Join succeeded:')).Count
        $welcomeCount = 0
        $networkFailureDiagnostics = @()
        $arenaStateDiagnostics = @()
        $hostArenaState = Get-ArenaState $hostContent 'authority'
        if (-not $hostArenaState) {
            $arenaStateDiagnostics += 'Host: missing authority arena state'
        }
        for ($clientIndex = 0; $clientIndex -lt $clientLogs.Count; ++$clientIndex) {
            $clientLog = $clientLogs[$clientIndex]
            $clientContent = Read-Log $clientLog
            if ($clientContent -match 'Fatal error:|Assertion failed:|PendingConnectionFailure') {
                $networkFailureDiagnostics += "Client$($clientIndex + 1): network failure in $clientLog"
            }
            if ($clientContent -match 'Welcomed by server') {
                ++$welcomeCount
            }
            $clientArenaState = Get-ArenaState $clientContent 'client'
            $clientName = "Client$($clientIndex + 1)"
            if (-not $clientArenaState) {
                $arenaStateDiagnostics += "${clientName}: missing client arena state"
            }
            elseif ($hostArenaState) {
                $stateMatches = $true
                for ($group = 1; $group -le 5; $group++) {
                    if ($hostArenaState.Groups[$group].Value -ne $clientArenaState.Groups[$group].Value) {
                        $stateMatches = $false
                        break
                    }
                }
                if ([int]$hostArenaState.Groups[3].Value -le 0 -or [int]$clientArenaState.Groups[3].Value -le 0) {
                    $stateMatches = $false
                }
                if (-not $stateMatches) {
                    $arenaStateDiagnostics += "${clientName}: arena state mismatch (host: $(Format-ArenaState $hostArenaState); client: $(Format-ArenaState $clientArenaState))"
                }
            }
        }
        if ($hostContent -match 'Fatal error:|Assertion failed:|PendingConnectionFailure') {
            $networkFailureDiagnostics += "Host: network failure in $hostLog"
        }
        $arenaStateParity = $arenaStateDiagnostics.Count -eq 0
        if ($networkFailureDiagnostics.Count -gt 0) {
            throw "LAN process failure(s): $($networkFailureDiagnostics -join '; '). Arena state: $($arenaStateDiagnostics -join '; ')"
        }
        if ($joinCount -ge $PlayerCount - 1 -and $welcomeCount -eq $PlayerCount - 1 -and $arenaStateParity) {
            break
        }
        for ($processIndex = 0; $processIndex -lt $processes.Count; ++$processIndex) {
            if ($processes[$processIndex].HasExited) {
                $processName = if ($processIndex -eq 0) { 'Host' } else { "Client$processIndex" }
                $processLog = if ($processIndex -eq 0) { $hostLog } else { $clientLogs[$processIndex - 1] }
                throw "$processName process exited before all clients joined and arena state matched. Arena state: $($arenaStateDiagnostics -join '; '). Process log: $processLog"
            }
        }
    } while ((Get-Date) -lt $deadline)

    if ($joinCount -lt $PlayerCount - 1 -or $welcomeCount -ne $PlayerCount - 1 -or -not $arenaStateParity) {
        throw "LAN assertions incomplete for $PlayerCount players: joins=$joinCount/$($PlayerCount - 1), welcomes=$welcomeCount/$($PlayerCount - 1), arena-state-parity=$arenaStateParity. Arena state: $($arenaStateDiagnostics -join '; '). Host log: $hostLog; client logs: $($clientLogs -join '; ')"
    }
    Write-Output "LAN transport and arena-state parity passed: $PlayerCount processes, $joinCount host joins, $welcomeCount client welcomes. Host arena: $(Format-ArenaState $hostArenaState). Host log: $hostLog"
    foreach ($clientLog in $clientLogs) {
        Write-Output "Client log: $clientLog"
    }
}
finally {
    foreach ($process in $processes) {
        if (-not $process.HasExited) {
            Stop-Process -InputObject $process
        }
    }
}
