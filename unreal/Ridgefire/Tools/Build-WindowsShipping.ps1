param(
    [string]$EngineRoot = 'G:\Unreal Engine\UE_5.8',
    [string]$OutputRoot = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$project = (Resolve-Path (Join-Path $PSScriptRoot '..\Ridgefire.uproject')).Path
$uat = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
if (-not (Test-Path -LiteralPath $uat)) {
    throw "Unreal Automation Tool not found: $uat"
}

if ($OutputRoot -eq '') {
    $OutputRoot = Join-Path (Split-Path $project) 'Saved\Packages'
}
$outputRootPath = [System.IO.Path]::GetFullPath($OutputRoot)
$archive = Join-Path $outputRootPath (Get-Date -Format 'yyyyMMdd-HHmmss')
New-Item -ItemType Directory -Path $archive -Force | Out-Null

& $uat BuildCookRun `
    "-project=$project" `
    -noP4 -platform=Win64 -clientconfig=Shipping `
    -build -cook -stage -pak -archive -prereqs `
    "-archivedirectory=$archive" -utf8output
if ($LASTEXITCODE -ne 0) {
    throw "Windows Shipping build/cook/package failed with exit code $LASTEXITCODE. Archive: $archive"
}

$executable = Join-Path $archive 'Windows\TP_FirstPerson.exe'
if (-not (Test-Path -LiteralPath $executable)) {
    throw "Packaging reported success but the game executable is missing: $executable"
}

Write-Output "Windows Shipping package: $archive"
Write-Output "Launch: $executable"
