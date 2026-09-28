param(
    [string]$EngineRoot = 'G:\Unreal Engine\UE_5.8',
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
$hostLog = Join-Path $logs "RIDGEFIRE-SESSION-$stamp-Host.log"
$clientLog = Join-Path $logs "RIDGEFIRE-SESSION-$stamp-Client.log"
$processes = @()

function Read-Log([string]$Path) {
    if (Test-Path -LiteralPath $Path) {
        return Get-Content -LiteralPath $Path -Raw
    }
    return ''
}

function Assert-NoRuntimeFailure([string]$Content, [string]$LogPath) {
    if ($Content -match 'Fatal error:|Assertion failed:|RIDGEFIRE SESSION SMOKE FAIL') {
        throw "Session smoke reported a failure. Log: $LogPath"
    }
}

try {
    $hostArguments = @(
        ('"' + $project + '"'), '/Game/Variant_Shooter/Lvl_Shooter', '-game', "-port=$Port",
        '-NullRHI', '-NoSound', '-unattended', '-nop4', '-nosplash', '-RidgefireSessionSmokeHost',
        ('-abslog="' + $hostLog + '"')
    )
    $hostProcess = Start-Process -FilePath $editor -ArgumentList $hostArguments -PassThru -WindowStyle Hidden
    $processes += $hostProcess
    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    do {
        Start-Sleep -Milliseconds 500
        $hostContent = Read-Log $hostLog
        Assert-NoRuntimeFailure $hostContent $hostLog
        if ($hostContent -match 'RIDGEFIRE SESSION SMOKE HOST READY' -and $hostContent -match "IpNetDriver listening on port $Port") {
            break
        }
        if ($hostProcess.HasExited) {
            throw "Session host exited before becoming ready. Log: $hostLog"
        }
    } while ((Get-Date) -lt $deadline)
    if ((Get-Date) -ge $deadline) {
        throw "Session host did not advertise and open port $Port before timeout. Log: $hostLog"
    }

    $clientArguments = @(
        ('"' + $project + '"'), '/Game/Variant_Shooter/Lvl_Shooter', '-game', '-NullRHI', '-NoSound',
        '-unattended', '-nop4', '-nosplash', '-RidgefireSessionSmokeClient', ('-abslog="' + $clientLog + '"')
    )
    $clientProcess = Start-Process -FilePath $editor -ArgumentList $clientArguments -PassThru -WindowStyle Hidden
    $processes += $clientProcess

    do {
        Start-Sleep -Milliseconds 500
        $hostContent = Read-Log $hostLog
        $clientContent = Read-Log $clientLog
        Assert-NoRuntimeFailure $hostContent $hostLog
        Assert-NoRuntimeFailure $clientContent $clientLog
        $hostAcceptedClient = $hostContent -match 'LogNet: Join succeeded:'
        $clientTravelRequested = $clientContent -match 'RIDGEFIRE SESSION SMOKE JOINED; client travel requested'
        $clientWelcomed = $clientContent -match 'Welcomed by server'
        if ($hostAcceptedClient -and $clientTravelRequested -and $clientWelcomed) {
            break
        }
        foreach ($process in $processes) {
            if ($process.HasExited) {
                throw "A session smoke process exited before discovery/join completed. Host log: $hostLog; client log: $clientLog"
            }
        }
    } while ((Get-Date) -lt $deadline)

    if (-not $hostAcceptedClient -or -not $clientTravelRequested -or -not $clientWelcomed) {
        throw "Session discovery/join did not complete. Host accepted=$hostAcceptedClient, client travel=$clientTravelRequested, client welcomed=$clientWelcomed. Host log: $hostLog; client log: $clientLog"
    }
    Write-Output "Null/LAN session smoke passed: discovery, subsystem join, host acceptance, and client welcome."
    Write-Output "Host log: $hostLog"
    Write-Output "Client log: $clientLog"
}
finally {
    foreach ($process in $processes) {
        if (-not $process.HasExited) {
            Stop-Process -InputObject $process
        }
    }
}
