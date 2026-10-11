param([int]$Port=7903,[int]$TimeoutSeconds=180,[string]$EngineRoot='G:/Unreal Engine/UE_5.8',
    [string]$HostProject='C:/Users/YaYa/Documents/Codex/2026-09-22/connect-this-github-repo-to-a/unreal/Ridgefire/Ridgefire.uproject',
    [string]$ClientProject='',
    [string]$OutputRoot='C:/Users/YaYa/Documents/ChatGPT/FPS-1/research/vs54/entrance-network')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
if (!$ClientProject) {$ClientProject=$HostProject}
$editor=Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor.exe'
$run=Join-Path $OutputRoot ('checks-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Force -Path $run | Out-Null
$processes=@();$logs=@((Join-Path $run 'Host.log'),(Join-Path $run 'Client.log'))
function Read-Log([string]$Path) {if(Test-Path -LiteralPath $Path) {return Get-Content -LiteralPath $Path -Raw};return ''}
function Check-Log([string]$Content) {if($Content -match 'FIRE NETWORK FAIL|ENTRANCE NETWORK FAIL|Fatal error:|Assertion failed:|Ensure condition failed') {throw 'Firing network fixture failed.'}}
try {
    $common=@('-game','-NullRHI','-NoSound','-unattended','-nop4','-nosplash','-RidgefireFireNetworkFixture','-RidgefireEntranceNetworkChecks','-RidgefireHitboxNetworkChecks','-ExecCmds="t.MaxFPS 60,t.IdleWhenNotForeground 0"')
    $hostArgs=@(('"'+$HostProject+'"'),'/Game/Variant_Shooter/Lvl_Shooter?listen',"-port=$Port")+$common+('-abslog="'+$logs[0]+'"')
    $processes+=Start-Process -FilePath $editor -ArgumentList $hostArgs -PassThru -WindowStyle Hidden
    $deadline=(Get-Date).AddSeconds($TimeoutSeconds)
    do {Start-Sleep -Milliseconds 250;$content=Read-Log $logs[0];Check-Log $content;if($processes[0].HasExited){throw 'Host exited.'}} while($content -notmatch "IpNetDriver listening on port $Port" -and (Get-Date)-lt $deadline)
    $clientArgs=@(('"'+$ClientProject+'"'),"127.0.0.1:$Port")+$common+('-abslog="'+$logs[1]+'"')
    $processes+=Start-Process -FilePath $editor -ArgumentList $clientArgs -PassThru -WindowStyle Hidden
    $passed=0
    do {
        Start-Sleep -Milliseconds 250;$texts=@($logs | ForEach-Object {Read-Log $_});foreach($content in $texts){Check-Log $content}
        $passed=@($texts | Where-Object {$_ -match 'IRON SUN FIRE NETWORK PASS' -and $_ -match 'IRON SUN ENTRANCE NETWORK PASS'}).Count
        if($passed -eq 2) {break};foreach($process in $processes) {if($process.HasExited){throw 'Peer exited before completion.'}}
    } while((Get-Date)-lt $deadline)
    if($passed -ne 2) {throw "Firing network timed out: $passed/2 peers: $run"}
    [ordered]@{passed=$true;peers=2;host_project=$HostProject;client_project=$ClientProject;logs=$logs;scope='Actual two-process Editor host/client replicated first-map gates, warned opening and party snapshot; independent bot body queries and controller LeftMouseButton primary/secondary shots retain weapon and spend ammo. Frozen enemies and accelerated setup. No Internet, packaged, hardware-input, latency/fun or second-PC acceptance.';time_utc=[DateTime]::UtcNow.ToString('o')} | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $run 'result.json') -Encoding UTF8
    Get-Content -LiteralPath (Join-Path $run 'result.json') -Raw
} finally {foreach($process in $processes) {if(!$process.HasExited){Stop-Process -InputObject $process}}}

