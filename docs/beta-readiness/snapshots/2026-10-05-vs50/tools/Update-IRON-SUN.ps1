param([string]$FeedRoot='', [string]$InstallRoot='', [switch]$NoLaunch, [switch]$Rollback)
$ErrorActionPreference='Stop'
Import-Module (Join-Path $PSScriptRoot 'RidgefireUpdate.psm1') -Force
if(!$InstallRoot){$InstallRoot=Join-Path ([Environment]::GetFolderPath('LocalApplicationData')) 'IRON SUN'}
if(!$FeedRoot){
    $config=Get-Content -LiteralPath (Join-Path $InstallRoot 'client.json') -Raw | ConvertFrom-Json
    if($config.app_id -cne 'IRON-SUN-LAN' -or $config.schema_version -ne 1){throw 'IRON SUN update settings are invalid.'}
    $FeedRoot=$config.feed_root
}
if(!$NoLaunch){Write-Host 'Checking IRON SUN updates...'}
$result=Invoke-RidgefireUpdate -FeedRoot $FeedRoot -InstallRoot $InstallRoot -NoLaunch:$NoLaunch -Rollback:$Rollback
if($NoLaunch){$result | ConvertTo-Json -Depth 6}
