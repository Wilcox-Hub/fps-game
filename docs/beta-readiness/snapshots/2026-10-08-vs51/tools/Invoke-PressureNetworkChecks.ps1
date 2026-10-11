param([int]$Port=7861,[int]$TimeoutSeconds=180,[string]$EngineRoot='G:/Unreal Engine/UE_5.8')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$project=(Resolve-Path (Join-Path $PSScriptRoot '../Ridgefire.uproject')).Path
$editor=Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor.exe'
$run=Join-Path (Split-Path $project) ('Saved/Automation/PressureNetwork/checks-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Force -Path $run | Out-Null
$processes=@();$logs=@((Join-Path $run 'Host.log'),(Join-Path $run 'Client.log'))
function Read-Log([string]$Path) {if(Test-Path -LiteralPath $Path) {return Get-Content -LiteralPath $Path -Raw};return ''}
function Check-Log([string]$Content) {if($Content -match 'PRESSURE NETWORK FAIL|Fatal error:|Assertion failed:|Ensure condition failed') {throw 'Pressure network fixture failed.'}}
try {
    $common=@('-game','-NullRHI','-NoSound','-unattended','-nop4','-nosplash','-RidgefirePressureNetworkFixture','-ExecCmds="t.MaxFPS 60,t.IdleWhenNotForeground 0"')
    $hostArgs=@(('"'+$project+'"'),'/Game/Variant_Shooter/Lvl_Shooter?listen',"-port=$Port")+$common+('-abslog="'+$logs[0]+'"')
    $processes+=Start-Process -FilePath $editor -ArgumentList $hostArgs -PassThru -WindowStyle Hidden
    $deadline=(Get-Date).AddSeconds($TimeoutSeconds)
    do {Start-Sleep -Milliseconds 250;$content=Read-Log $logs[0];Check-Log $content;if($processes[0].HasExited){throw 'Host exited.'}} while($content -notmatch "IpNetDriver listening on port $Port" -and (Get-Date)-lt $deadline)
    $clientArgs=@(('"'+$project+'"'),"127.0.0.1:$Port")+$common+('-abslog="'+$logs[1]+'"')
    $processes+=Start-Process -FilePath $editor -ArgumentList $clientArgs -PassThru -WindowStyle Hidden
    $passed=0
    do {
        Start-Sleep -Milliseconds 250;$texts=@($logs | ForEach-Object {Read-Log $_});foreach($content in $texts){Check-Log $content}
        $passed=@($texts | Where-Object {$_ -match 'IRON SUN PRESSURE NETWORK PASS'}).Count
        if($passed -eq 2) {break};foreach($process in $processes) {if($process.HasExited){throw 'Peer exited before completion.'}}
    } while((Get-Date)-lt $deadline)
    if($passed -ne 2) {throw "Pressure network timed out: $passed/2 peers: $run"}
    [ordered]@{passed=$true;peers=2;logs=$logs;scope='Actual local Editor host/client character moves, slide/ADS/release and server displacement on a replicated test platform with frozen enemies. No Internet, packaged, hardware-input, latency/fun or second-PC acceptance.';time_utc=[DateTime]::UtcNow.ToString('o')} | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $run 'result.json') -Encoding UTF8
    Get-Content -LiteralPath (Join-Path $run 'result.json') -Raw
} finally {foreach($process in $processes) {if(!$process.HasExited){Stop-Process -InputObject $process}}}


