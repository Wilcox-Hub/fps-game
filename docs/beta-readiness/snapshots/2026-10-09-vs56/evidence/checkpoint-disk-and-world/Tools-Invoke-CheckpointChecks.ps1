param([string]$EngineRoot='G:/Unreal Engine/UE_5.8',[switch]$Render,[int]$TimeoutSeconds=160)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$project=(Resolve-Path (Join-Path $PSScriptRoot '../Ridgefire.uproject')).Path
$run=Join-Path (Split-Path $project) ('Saved/Automation/Checkpoints/'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Force -Path $run | Out-Null
$log=Join-Path $run 'Automation.log'
$argsForEditor=@(('"'+$project+'"'),'/Game/Variant_Shooter/Lvl_Shooter','-game','-NoSound','-unattended','-nop4','-nosplash','-ExecCmds="t.MaxFPS 60,t.IdleWhenNotForeground 0,Automation RunTests Ridgefire.Checkpoints.;"','-TestExit="Automation Test Queue Empty"',('-abslog="'+$log+'"'),('-RidgefireEncounterOutput="'+$run+'"'))
if($Render){$argsForEditor+=@('-RenderOffscreen','-windowed','-ForceRes','-ResX=1920','-ResY=1080','-d3d12')}else{$argsForEditor+='-NullRHI'}
$p=Start-Process -FilePath (Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor.exe') -ArgumentList $argsForEditor -PassThru -WindowStyle Hidden
$null=$p.Handle
try{
 $deadline=(Get-Date).AddSeconds($TimeoutSeconds)
 while(!$p.HasExited -and (Get-Date)-lt $deadline){Start-Sleep -Milliseconds 500}
 if(!$p.HasExited){throw "Checkpoint checks timed out: $log"}
 $p.WaitForExit();$content=Get-Content -LiteralPath $log -Raw
 foreach($name in @('DiskValidationAndRetirement','RestoreAllArenaBoundaries')){
  if(!$content.Contains('Result={Success} Name={'+$name+'}')){throw "Checkpoint check $name did not pass: $log"}
 }
 if($p.ExitCode -ne 0 -or $content -match 'Result=\{Fail|Fatal error:|Assertion failed:|Ensure condition failed'){throw "Checkpoint process failed: $log"}
 if($Render -and !(Test-Path -LiteralPath (Join-Path $run 'continue-run-1080p.png'))){throw 'Checkpoint menu frame missing'}
 [ordered]@{passed=$true;tests=2;log=$log;render_requested=[bool]$Render;scope='Controlled actual standalone Editor world and isolated disk slots; not physical LAN, Shipping combat, human fun or performance acceptance.';time_utc=[DateTime]::UtcNow.ToString('o')}|ConvertTo-Json -Depth 4|Set-Content -LiteralPath (Join-Path $run 'result.json') -Encoding utf8
 Get-Content -LiteralPath (Join-Path $run 'result.json') -Raw
}finally{if(!$p.HasExited){Stop-Process -InputObject $p}}
