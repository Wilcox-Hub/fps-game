param([string]$EngineRoot='G:/Unreal Engine/UE_5.8',[int]$Port=7923,[int]$TimeoutSeconds=150,[string]$OutputRoot='')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$project=(Resolve-Path (Join-Path $PSScriptRoot '../Ridgefire.uproject')).Path
$editor=Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor.exe'
if(!$OutputRoot){$OutputRoot=Join-Path (Split-Path $project) 'Saved/Automation/CheckpointNetwork'}
$run=Join-Path $OutputRoot ('checks-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Force -Path $run | Out-Null
$prefix='IronSun_Test_Cold_'+[Guid]::NewGuid().ToString('N')
$guest=[Guid]::NewGuid().ToString('N');$wrong=[Guid]::NewGuid().ToString('N')
$owned=[Collections.Generic.List[System.Diagnostics.Process]]::new()
function Start-Peer([string]$Map,[string]$Log,[switch]$Resume){
 $argsForPeer=@(('"'+$project+'"'),$Map,'-game','-NullRHI','-NoSound','-unattended','-nop4','-nosplash','-RidgefireCheckpointNetwork',('-RidgefireCheckpointPrefix='+$prefix),'-ExecCmds="t.MaxFPS 60,t.IdleWhenNotForeground 0"',('-abslog="'+$Log+'"'))
 if($Map.Contains('?listen')){$argsForPeer+="-port=$Port"}
 if($Resume){$argsForPeer+='-RidgefireCheckpointResumePhase'}
 $p=Start-Process -FilePath $editor -ArgumentList $argsForPeer -PassThru -WindowStyle Hidden
 $null=$p.Handle;$owned.Add($p);return $p
}
function Wait-Marker([System.Diagnostics.Process]$Process,[string]$Log,[string]$Marker){
 $deadline=(Get-Date).AddSeconds($TimeoutSeconds)
 do{
  Start-Sleep -Milliseconds 250
  $content=if(Test-Path -LiteralPath $Log){Get-Content -LiteralPath $Log -Raw}else{''}
  if($content -match 'CHECKPOINT NETWORK FAIL|Fatal error:|Assertion failed:|Ensure condition failed'){throw "Checkpoint peer failed: $Log"}
  if($content.Contains($Marker)){return}
  if($Process.HasExited){throw "Checkpoint peer exited before $Marker : $Log"}
 }while((Get-Date)-lt $deadline)
 throw "Checkpoint peer timed out waiting for $Marker : $Log"
}
try{
 $saveHostLog=Join-Path $run 'SaveHost.log';$saveClientLog=Join-Path $run 'SaveClient.log'
 $saveHost=Start-Peer '/Game/Variant_Shooter/Lvl_Shooter?listen' $saveHostLog
 Wait-Marker $saveHost $saveHostLog 'CHECKPOINT NETWORK HOST READY phase=save'
 $saveClient=Start-Peer "127.0.0.1:$Port`?RidgefirePlayer=$guest" $saveClientLog
 Wait-Marker $saveHost $saveHostLog 'CHECKPOINT NETWORK SAVED party=2 wave=2 seed=1977'
 # A fresh host and fresh guest process must read disk; no surviving GameInstance is reused.
 Stop-Process -InputObject $saveClient
 Stop-Process -InputObject $saveHost
 $saveFiles=@()
 for($i=0;$i -lt 2;$i++){
  $file=Join-Path (Split-Path $project) ('Saved/SaveGames/'+$prefix+$i+'.sav')
  if(Test-Path -LiteralPath $file){$copy=Join-Path $run ('checkpoint'+$i+'.sav');Copy-Item -LiteralPath $file -Destination $copy;$saveFiles+=@{path=$copy;sha256=(Get-FileHash -LiteralPath $copy -Algorithm SHA256).Hash}}
 }
 if(!$saveFiles.Count){throw 'No actual saved checkpoint file found'}
 $resumeHostLog=Join-Path $run 'ResumeHost.log';$resumeClientLog=Join-Path $run 'ResumeClient.log';$wrongLog=Join-Path $run 'WrongSquad.log'
 $resumeHost=Start-Peer '/Game/Variant_Shooter/Lvl_Shooter?listen?RidgefireLobby?RidgefireResume?RidgefireSlots=2' $resumeHostLog -Resume
 Wait-Marker $resumeHost $resumeHostLog 'CHECKPOINT NETWORK WAITING missing_party=1 launch_blocked=1'
 $wrongClient=Start-Peer "127.0.0.1:$Port`?RidgefirePlayer=$wrong" $wrongLog -Resume
 Wait-Marker $wrongClient $wrongLog 'This checkpoint belongs to a different squad.'
 if(!$wrongClient.HasExited){Stop-Process -InputObject $wrongClient}
 $resumeClient=Start-Peer "127.0.0.1:$Port`?RidgefirePlayer=$guest" $resumeClientLog -Resume
 Wait-Marker $resumeHost $resumeHostLog 'CHECKPOINT NETWORK PASS peer=host'
 Wait-Marker $resumeClient $resumeClientLog 'CHECKPOINT NETWORK PASS peer=client'
 [ordered]@{passed=$true;cold_host_restart=$true;cold_client_restart=$true;missing_teammate_blocks_launch=$true;wrong_squad_rejected=$true;restored_players=2;native_fire_both_guns=$true;checkpoint_files=$saveFiles;logs=@($saveHostLog,$saveClientLog,$resumeHostLog,$resumeClientLog,$wrongLog);scope='Controlled real local Editor processes, isolated disk slots, host process stopped and recreated, same saved guest identity, lobby ready RPCs and native controller input. No physical second-PC, Internet, human fun or Shipping combat acceptance.';time_utc=[DateTime]::UtcNow.ToString('o')}|ConvertTo-Json -Depth 6|Set-Content -LiteralPath (Join-Path $run 'result.json') -Encoding utf8
 Get-Content -LiteralPath (Join-Path $run 'result.json') -Raw
}finally{foreach($p in $owned){if(!$p.HasExited){Stop-Process -InputObject $p}}}
