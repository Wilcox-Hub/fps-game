Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$package='C:\Users\YaYa\Documents\Codex\2026-09-22\connect-this-github-repo-to-a\unreal\Ridgefire\Saved\Packages\20261008-230233-968'
$windows=Join-Path $package 'Windows'
$metadata=Get-Content -LiteralPath (Join-Path $package 'PackageInfo.json') -Raw|ConvertFrom-Json
if($metadata.Status -ne 'COMPLETE' -or $metadata.Configuration -ne 'Shipping'){throw 'Package is not complete Shipping'}
$paths=@('TP_FirstPerson.exe','Ridgefire/Binaries/Win64/TP_FirstPerson-Win64-Shipping.exe','Ridgefire/Content/Paks/Ridgefire-Windows.pak','Ridgefire/Content/Paks/Ridgefire-Windows.ucas','Ridgefire/Content/Paks/Ridgefire-Windows.utoc')
$artifacts=@()
foreach($relative in $paths){$p=Join-Path $windows $relative;$f=Get-Item -LiteralPath $p;if($f.Length -le 0){throw 'Empty package artifact'};$artifacts+=@{path=$relative;size_bytes=$f.Length;sha256=(Get-FileHash -LiteralPath $p).Hash}}
$log=Join-Path $PSScriptRoot 'shipping-startup.log'
$runtime=Join-Path $windows $paths[1]
$screen=Join-Path $PSScriptRoot 'ShippingMenu.png'
$argsForGame=@('/Game/Variant_Shooter/Lvl_Shooter?RidgefireMenu','-RenderOffscreen','-windowed','-ForceRes','-ResX=1920','-ResY=1080','-d3d12','-NoSound','-unattended','-nosplash',('-ExecCmds="Shot SHOWUI NAME='+$screen.Replace('\','/')+'"'),('-abslog="'+$log+'"'))
$process=Start-Process -FilePath $runtime -WorkingDirectory $windows -ArgumentList $argsForGame -PassThru -WindowStyle Hidden
$null=$process.Handle
try {
 $deadline=(Get-Date).AddSeconds(45)
 do {
  Start-Sleep -Milliseconds 500
  $text=if(Test-Path -LiteralPath $log){Get-Content -LiteralPath $log -Raw}else{''}
  if($process.HasExited){throw 'Shipping game exited during startup'}
  if($text -match 'Fatal error:|Assertion failed:|Ensure condition failed'){throw 'Shipping startup failure'}
  $initialized=$text -match 'Game Engine Initialized|Bringing World .* up for play|RIDGEFIRE ARENA STATE'
 }while(!$initialized -and (Get-Date)-lt $deadline)
 if(!$initialized){throw 'Shipping initialization marker missing'}
 Start-Sleep -Seconds 8
 if($process.HasExited){throw 'Shipping game did not remain alive'}
 $text=Get-Content -LiteralPath $log -Raw
 if($text -match 'Fatal error:|Assertion failed:|Ensure condition failed'){throw 'Shipping bounded startup failure'}
 $proof=[ordered]@{archive_absolute_path=$windows;artifacts=$artifacts;launched=$true;launch_scope='Actual Shipping runtime bounded D3D12 offscreen/no-sound main-menu startup at 1080p. Console screenshot attempted and must be inspected separately; no hardware/full-run/friend co-op acceptance'}
 $proof|ConvertTo-Json -Depth 5|Set-Content -LiteralPath (Join-Path $PSScriptRoot 'package-artifacts.json') -Encoding utf8
 [ordered]@{passed=$true;package_root=$package;runtime=$runtime;initialized=$true;screenshot=$screen;screenshot_exists=(Test-Path $screen);scope=$proof.launch_scope;log=$log;time_utc=[datetime]::UtcNow.ToString('o')}|ConvertTo-Json|Set-Content (Join-Path $PSScriptRoot 'shipping-startup.json') -Encoding utf8
 Get-Content (Join-Path $PSScriptRoot 'shipping-startup.json') -Raw
}finally{if(!$process.HasExited){Stop-Process -InputObject $process}}
