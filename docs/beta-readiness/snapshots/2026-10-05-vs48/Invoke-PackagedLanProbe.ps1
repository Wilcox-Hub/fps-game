param([ValidateSet('Shipping','Editor')][string]$HostKind='Shipping',[int]$Port=7855,[int]$TimeoutSeconds=55)
$ErrorActionPreference='Stop'
$projectRoot='C:\Users\YaYa\Documents\Codex\2026-09-22\connect-this-github-repo-to-a\unreal\Ridgefire'
$editor='G:\Unreal Engine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
$windowsRoot=Join-Path $projectRoot 'Saved\Packages\20261005-134833-568\Windows'
$run=Join-Path $PSScriptRoot ($HostKind+'-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Path $run -Force | Out-Null
$processes=@();$children=@();$probeHostProcess=$null
function ReadLog([string]$Path){if(Test-Path -LiteralPath $Path){return Get-Content -LiteralPath $Path -Raw};return ''}
try{
    $common=@('/Game/Variant_Shooter/Lvl_Shooter?RidgefireMenu',"-port=$Port",'-NullRHI','-NoSound','-unattended','-nop4','-nosplash','-ExecCmds="t.MaxFPS 60,t.IdleWhenNotForeground 0"','-RidgefireSessionSmokeHost')
    if($HostKind -eq 'Shipping'){
        $probeHostProcess=Start-Process -FilePath (Join-Path $windowsRoot 'TP_FirstPerson.exe') -WorkingDirectory $windowsRoot -ArgumentList $common -PassThru -WindowStyle Hidden
    }else{
        $hostArgs=@(('"'+(Join-Path $projectRoot 'Ridgefire.uproject')+'"'),'-game')+$common+('-abslog="'+(Join-Path $run 'Host.log')+'"')
        $probeHostProcess=Start-Process -FilePath $editor -ArgumentList $hostArgs -PassThru -WindowStyle Hidden
    }
    $processes+=$probeHostProcess
    $deadline=(Get-Date).AddSeconds($TimeoutSeconds)
    $socketAt=(Get-Date).AddSeconds(12)
    do{
        Start-Sleep -Milliseconds 400
        if($HostKind -eq 'Shipping'){
            $children=@(Get-CimInstance Win32_Process -Filter "ParentProcessId=$($probeHostProcess.Id)" | Where-Object {$_.Name -eq 'TP_FirstPerson-Win64-Shipping.exe'} | ForEach-Object {$_.ProcessId})
        }
        $owned=@($probeHostProcess.Id)+$children
        $endpoints=@(Get-NetUDPEndpoint -LocalPort $Port -ErrorAction SilentlyContinue | Where-Object {$_.OwningProcess -in $owned})
        if($endpoints.Count){break}
    }while((Get-Date) -lt $socketAt)
    $hostLog=ReadLog (Join-Path $run 'Host.log')
    $hostSockets=@(Get-NetUDPEndpoint -ErrorAction SilentlyContinue | Where-Object {$_.OwningProcess -in $owned} | Select-Object LocalAddress,LocalPort,OwningProcess)
    $hostSockets | ConvertTo-Json | Set-Content (Join-Path $run 'Host-sockets.json') -Encoding UTF8
    $clientLog=Join-Path $run 'Client.log'
    $clientArgs=@(('"'+(Join-Path $projectRoot 'Ridgefire.uproject')+'"'),'/Game/Variant_Shooter/Lvl_Shooter?RidgefireMenu','-game','-NullRHI','-NoSound','-unattended','-nop4','-nosplash','-ExecCmds="t.MaxFPS 60,t.IdleWhenNotForeground 0"','-RidgefireSessionSmokeClient',('-abslog="'+$clientLog+'"'))
    $client=Start-Process -FilePath $editor -ArgumentList $clientArgs -PassThru -WindowStyle Hidden
    $processes+=$client
    $found=$false;$connected=$false
    do{
        Start-Sleep -Milliseconds 500
        $text=ReadLog $clientLog
        $found=$text -match 'RIDGEFIRE SESSION SMOKE DISCOVERED [1-9]'
        $connected=$text -match 'RIDGEFIRE SESSION LOBBY CONNECTED hosting=0'
        if($connected -or $text -match 'RIDGEFIRE SESSION SMOKE FAIL|Fatal error:|Assertion failed:'){break}
        if($client.HasExited){break}
    }while((Get-Date) -lt $deadline)
    $result=[ordered]@{host_kind=$HostKind;port=$Port;host_listening=($endpoints.Count -gt 0);host_sockets=$hostSockets;discovered=$found;connected=$connected;passed=($found -and $connected);client_log=$clientLog;scope='Shipping bootstrap host versus actual Editor Null/LAN client using existing session subsystem smoke flags; localhost only, headless, no two-device or human acceptance; no firewall changes'}
    $result | ConvertTo-Json -Depth 5 | Set-Content (Join-Path $run 'result.json') -Encoding UTF8
    $result | ConvertTo-Json -Depth 5
    if(!$result.passed){throw "LAN discovery or lobby connection failed: $run"}
}finally{
    foreach($id in $children){$p=Get-Process -Id $id -ErrorAction SilentlyContinue;if($p -and !$p.HasExited){Stop-Process -InputObject $p}}
    foreach($p in $processes){if(!$p.HasExited){Stop-Process -InputObject $p}}
}

