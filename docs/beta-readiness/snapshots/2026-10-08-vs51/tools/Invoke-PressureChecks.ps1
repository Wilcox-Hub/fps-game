param([string]$Filter='Ridgefire.Pressure.',[switch]$Render)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$project=(Resolve-Path (Join-Path $PSScriptRoot '../Ridgefire.uproject')).Path
$run=Join-Path (Split-Path $project) ('Saved/Automation/Pressure/checks-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Path $run -Force | Out-Null
$log=Join-Path $run 'Automation.log'
$argsForEditor=@(('"'+$project+'"'),'/Game/Variant_Shooter/Lvl_Shooter','-game','-NoSound','-unattended','-nop4','-nosplash',('-ExecCmds="t.MaxFPS 60,t.IdleWhenNotForeground 0,Automation RunTests '+$Filter+';"'),'-TestExit="Automation Test Queue Empty"',('-abslog="'+$log+'"'),('-ReportOutputPath="'+(Join-Path $run 'Report')+'"'))
if($Render){$argsForEditor+=@('-RenderOffscreen','-windowed','-ForceRes','-ResX=1920','-ResY=1080','-d3d12')}else{$argsForEditor+='-NullRHI'}
$process=Start-Process -FilePath 'G:\Unreal Engine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' -ArgumentList $argsForEditor -PassThru -WindowStyle Hidden
$null=$process.Handle
try {
 $deadline=(Get-Date).AddSeconds(180)
 while(!$process.HasExited -and (Get-Date)-lt $deadline){Start-Sleep -Milliseconds 500}
 if(!$process.HasExited){throw "Checks timed out: $log"}
 $process.WaitForExit()
 $content=Get-Content -LiteralPath $log -Raw
 $passes=@([regex]::Matches($content,'Test Completed\. Result=\{Success\} Name=\{[^}]*\} Path=\{([^}]+)\}')|ForEach-Object {$_.Groups[1].Value})
 $required=if($Filter -eq 'Ridgefire.Pressure.'){@('Ridgefire.Pressure.MovementAimAndCombat','Ridgefire.Pressure.VisualAimAndTimedBurst')}else{@($Filter)}
 $missing=@($required | Where-Object {$_ -notin $passes})
 $passed=$missing.Count -eq 0 -and $process.ExitCode -eq 0 -and $passes.Count -gt 0 -and $content -notmatch 'Test Completed\. Result=\{Fail|Fatal error:|Assertion failed:|Ensure condition failed'
 [ordered]@{passed=$passed;tests=$passes;filter=$Filter;log=$log;rendered=[bool]$Render;scope='Controlled local Editor production-path checks; no human or second-PC acceptance';exit_code=$process.ExitCode}|ConvertTo-Json -Depth 4|Set-Content (Join-Path $run 'result.json') -Encoding utf8
 Get-Content (Join-Path $run 'result.json') -Raw
 if(!$passed){throw "Checks failed: $log"}
}finally{if(!$process.HasExited){Stop-Process -InputObject $process}}

