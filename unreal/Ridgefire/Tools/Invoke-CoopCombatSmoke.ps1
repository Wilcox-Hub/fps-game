param(
    [string]$EngineRoot = 'G:\Unreal Engine\UE_5.8',
    [ValidateRange(1024, 65535)]
    [int]$Port = 7781,
    [int]$TimeoutSeconds = 120,
    [switch]$VerifyLateJoin
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
$hostLog = Join-Path $logs "RIDGEFIRE-COOP-$stamp-Host.log"
$clientLog = Join-Path $logs "RIDGEFIRE-COOP-$stamp-Client.log"
$lateClientLog = Join-Path $logs "RIDGEFIRE-COOP-$stamp-LateClient.log"
$processes = @()

function Read-Log([string]$Path) {
    if (Test-Path -LiteralPath $Path) {
        return Get-Content -LiteralPath $Path -Raw
    }
    return ''
}

function Assert-NoRuntimeFailure([string]$Content, [string]$LogPath) {
    if ($Content -match 'Fatal error:|Assertion failed:|RIDGEFIRE COOP .*SMOKE FAIL|FNetGUIDCache::SupportsObject: StaticMeshComponent .*RidgefireArenaDressing.*NOT Supported') {
        throw "Co-op combat smoke reported a failure. Log: $LogPath"
    }
}

try {
    $hostArguments = @(
        ('"' + $project + '"'), '/Game/Variant_Shooter/Lvl_Shooter?listen', '-game', "-port=$Port",
        '-NullRHI', '-NoSound', '-unattended', '-nop4', '-nosplash', '-RidgefireCoopCombatSmoke',
        ('-abslog="' + $hostLog + '"')
    )
    $hostProcess = Start-Process -FilePath $editor -ArgumentList $hostArguments -PassThru -WindowStyle Hidden
    $processes += $hostProcess
    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    do {
        Start-Sleep -Milliseconds 500
        $hostContent = Read-Log $hostLog
        Assert-NoRuntimeFailure $hostContent $hostLog
        if ($hostContent -match "IpNetDriver listening on port $Port") {
            break
        }
        if ($hostProcess.HasExited) {
            throw "Listen host exited before opening port $Port. Log: $hostLog"
        }
    } while ((Get-Date) -lt $deadline)
    if ((Get-Date) -ge $deadline) {
        throw "Listen host did not open port $Port before timeout. Log: $hostLog"
    }

    $clientArguments = @(
        ('"' + $project + '"'), "127.0.0.1:$Port", '-game', '-NullRHI', '-NoSound',
        '-unattended', '-nop4', '-nosplash', '-RidgefireCoopCombatSmoke', ('-abslog="' + $clientLog + '"')
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
        $distinctPawns = $hostContent -match 'RIDGEFIRE COOP SMOKE PASS unique possessed pawns:'
        $lineOfSightPrepared = $hostContent -match 'RIDGEFIRE COOP SMOKE LOS_PASS target='
        $targetPrepared = $hostContent -match 'RIDGEFIRE COOP SMOKE TARGET_READY'
        $targetName = if ($hostContent -match 'RIDGEFIRE COOP SMOKE TARGET_READY remote=[^ ]+ target=([A-Za-z0-9_]+) hp=1') { $Matches[1] } else { '' }
        $serverAimSynced = $hostContent -match 'RIDGEFIRE COOP SERVER AIM_SYNC pawn='
        $serverTraceConfirmed = ($targetName -ne '') -and ($hostContent -match "RIDGEFIRE COOP SERVER TRACE pawn=.*mainActor=$([regex]::Escape($targetName)) hitHP=1\.0")
        $serverCombatPassed = $hostContent -match 'RIDGEFIRE COOP SMOKE PASS server-authoritative remote fire'
        $clientReplicationPassed = $clientContent -match 'RIDGEFIRE COOP CLIENT SMOKE PASS replicated ammo='
        $remoteFireRpcCount = ([regex]::Matches($hostContent, 'RIDGEFIRE COOP SERVER FIRE_RPC pawn=')).Count
        $clientWelcomed = $clientContent -match 'Welcomed by server'
        $hostInitialBlockoutSuppressed = $hostContent -match 'RIDGEFIRE: Client/server arena setup suppressed [1-9]\d* legacy meshes[\s\S]*?RIDGEFIRE ARENA STATE role=authority kind=iron-sun'
        $clientInitialBlockoutSuppressed = $clientContent -match 'RIDGEFIRE: Client/server arena setup suppressed [1-9]\d* legacy meshes[\s\S]*?RIDGEFIRE ARENA STATE role=client kind=iron-sun'
        $hostArenaState = [regex]::Match($hostContent, 'RIDGEFIRE ARENA STATE role=authority kind=([^ ]+) revision=(\d+) components=(\d+) center=(.*?) ground=([-\d.]+)')
        $clientArenaState = [regex]::Match($clientContent, 'RIDGEFIRE ARENA STATE role=client kind=([^ ]+) revision=(\d+) components=(\d+) center=(.*?) ground=([-\d.]+)')
        $arenaReplicated = $hostArenaState.Success -and $clientArenaState.Success
        if ($arenaReplicated) {
            for ($group = 1; $group -le 5; $group++) {
                if ($hostArenaState.Groups[$group].Value -ne $clientArenaState.Groups[$group].Value) {
                    $arenaReplicated = $false
                    break
                }
            }
        }
        $hostFoundryState = [regex]::Match($hostContent, 'RIDGEFIRE ARENA STATE role=authority kind=brassfall revision=(\d+) components=(\d+) center=(.*?) ground=([-\d.]+)')
        $clientFoundryState = [regex]::Match($clientContent, 'RIDGEFIRE ARENA STATE role=client kind=brassfall revision=(\d+) components=(\d+) center=(.*?) ground=([-\d.]+)')
        $foundryReplicated = $hostFoundryState.Success -and $clientFoundryState.Success
        if ($foundryReplicated) {
            for ($group = 1; $group -le 4; $group++) {
                if ($hostFoundryState.Groups[$group].Value -ne $clientFoundryState.Groups[$group].Value) {
                    $foundryReplicated = $false
                    break
                }
            }
            $foundryReplicated = $foundryReplicated -and ($clientContent -match 'Client/server arena setup suppressed \d+ legacy meshes')
        }
        if ($hostAcceptedClient -and $distinctPawns -and $lineOfSightPrepared -and $targetPrepared -and $serverAimSynced -and $serverTraceConfirmed -and $serverCombatPassed -and $clientReplicationPassed -and $remoteFireRpcCount -ge 3 -and $clientWelcomed -and $hostInitialBlockoutSuppressed -and $clientInitialBlockoutSuppressed -and $arenaReplicated -and $foundryReplicated) {
            break
        }
        foreach ($process in $processes) {
            if ($process.HasExited) {
                throw "A co-op smoke process exited before all assertions passed. Host log: $hostLog; client log: $clientLog"
            }
        }
    } while ((Get-Date) -lt $deadline)

    if (-not $hostAcceptedClient -or -not $distinctPawns -or -not $lineOfSightPrepared -or -not $targetPrepared -or -not $serverAimSynced -or -not $serverTraceConfirmed -or -not $serverCombatPassed -or -not $clientReplicationPassed -or $remoteFireRpcCount -lt 3 -or -not $clientWelcomed -or -not $hostInitialBlockoutSuppressed -or -not $clientInitialBlockoutSuppressed -or -not $arenaReplicated -or -not $foundryReplicated) {
        throw "Co-op assertions incomplete: joined=$hostAcceptedClient distinct-pawns=$distinctPawns los=$lineOfSightPrepared target=$targetPrepared aim-sync=$serverAimSynced target-trace=$serverTraceConfirmed server-combat=$serverCombatPassed client-replication=$clientReplicationPassed remote-fire-rpcs=$remoteFireRpcCount welcomed=$clientWelcomed host-initial-blockout=$hostInitialBlockoutSuppressed client-initial-blockout=$clientInitialBlockoutSuppressed initial-arena-parity=$arenaReplicated Foundry-arena-parity=$foundryReplicated. Host log: $hostLog; client log: $clientLog"
    }

    if ($VerifyLateJoin) {
        $lateClientArguments = @(
            ('"' + $project + '"'), "127.0.0.1:$Port", '-game', '-NullRHI', '-NoSound',
            '-unattended', '-nop4', '-nosplash', ('-abslog="' + $lateClientLog + '"')
        )
        $lateClientProcess = Start-Process -FilePath $editor -ArgumentList $lateClientArguments -PassThru -WindowStyle Hidden
        $processes += $lateClientProcess
        $lateJoinPassed = $false
        do {
            Start-Sleep -Milliseconds 500
            $hostContent = Read-Log $hostLog
            $lateClientContent = Read-Log $lateClientLog
            Assert-NoRuntimeFailure $hostContent $hostLog
            Assert-NoRuntimeFailure $lateClientContent $lateClientLog
            $lateFoundryState = [regex]::Match($lateClientContent, 'RIDGEFIRE ARENA STATE role=client kind=brassfall revision=(\d+) components=(\d+) center=(.*?) ground=([-\d.]+)')
            $lateBlockoutSuppressed = $lateClientContent -match 'RIDGEFIRE: Client/server arena setup suppressed [1-9]\d* legacy meshes[\s\S]*?RIDGEFIRE ARENA STATE role=client kind=brassfall'
            $lateJoinPassed = ($lateClientContent -match 'Welcomed by server') -and $lateBlockoutSuppressed -and $lateFoundryState.Success
            if ($lateJoinPassed) {
                for ($group = 1; $group -le 4; $group++) {
                    if ($hostFoundryState.Groups[$group].Value -ne $lateFoundryState.Groups[$group].Value) {
                        $lateJoinPassed = $false
                        break
                    }
                }
            }
            if ($lateJoinPassed) {
                break
            }
            foreach ($process in $processes) {
                if ($process.HasExited) {
                    throw "A co-op process exited before late-join arena parity passed. Host log: $hostLog; late-client log: $lateClientLog"
                }
            }
        } while ((Get-Date) -lt $deadline)
        if (-not $lateJoinPassed) {
            throw "Late client did not receive matching Brassfall state and legacy blockout suppression before timeout. Host log: $hostLog; late-client log: $lateClientLog"
        }
    }

    Write-Output 'NullRHI co-op smoke passed: host/client state parity in Iron Sun and Brassfall, client blockout suppression, distinct pawns, acknowledged remote aim, authoritative target hit/damage, single reward, and replicated health/ammo/score.'
    Write-Output "Host log: $hostLog"
    Write-Output "Client log: $clientLog"
    if ($VerifyLateJoin) {
        Write-Output "Late-join NullRHI Brassfall arena-state parity and blockout suppression passed. Late-client log: $lateClientLog"
    }
}
finally {
    foreach ($process in $processes) {
        if (-not $process.HasExited) {
            Stop-Process -InputObject $process
        }
    }
}
