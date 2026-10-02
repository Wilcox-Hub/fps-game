param([string]$RepositoryRoot = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent))
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$latest = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'latest.json') -Raw | ConvertFrom-Json
$manifest = @(Get-Content -LiteralPath (Join-Path $PSScriptRoot $latest.input_manifest) -Raw | ConvertFrom-Json)
$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $RepositoryRoot $latest.project))
$expected = @{}
foreach ($entry in $manifest) {
    if ($expected.ContainsKey($entry.path)) { throw "Duplicate manifest path: $($entry.path)" }
    $expected[$entry.path] = $entry.sha256
}
$observed = @{}
$differences = @()
foreach ($directory in @('Source','Config','Content')) {
    $path = Join-Path $projectRoot $directory
    if (!(Test-Path -LiteralPath $path -PathType Container)) { continue }
    foreach ($file in Get-ChildItem -LiteralPath $path -File -Recurse) {
        $relative = $file.FullName.Substring($projectRoot.Length+1).Replace('\','/')
        $hash = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash
        $observed[$relative] = $hash
        if (!$expected.ContainsKey($relative)) {
            $differences += [ordered]@{path=$relative; change='added'}
        } elseif ($expected[$relative] -ne $hash) {
            $differences += [ordered]@{path=$relative; change='modified'}
        }
    }
}
foreach ($relative in $expected.Keys) {
    if (!$observed.ContainsKey($relative)) { $differences += [ordered]@{path=$relative; change='missing'} }
}
if ($latest.PSObject.Properties.Name -contains 'project_descriptor') {
    $descriptor = Join-Path $projectRoot $latest.project_descriptor.path
    if (!(Test-Path -LiteralPath $descriptor -PathType Leaf)) {
        $differences += [ordered]@{path=$latest.project_descriptor.path; change='missing'}
    } elseif ((Get-FileHash -LiteralPath $descriptor -Algorithm SHA256).Hash -ne $latest.project_descriptor.sha256) {
        $differences += [ordered]@{path=$latest.project_descriptor.path; change='modified'}
    }
}
[ordered]@{
    snapshot = $latest.snapshot_id
    tested_on = $latest.tested_on
    gameplay_readiness_percent = $latest.gameplay_readiness_percent
    verdict = $latest.verdict
    reusable_input_evidence = ($differences.Count -eq 0)
    observed_input_count = $observed.Count
    expected_input_count = $expected.Count
    differences = @($differences | Sort-Object { $_.path })
    validation_scope = 'Input hash comparison only; no gameplay tests executed. Build/package/environment changes need separate validation.'
} | ConvertTo-Json -Depth 6
