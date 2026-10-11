param([Parameter(Mandatory)][string]$FeedRoot,[string]$InstallRoot='',[switch]$DesktopShortcut)
$ErrorActionPreference='Stop'
Import-Module (Join-Path $PSScriptRoot 'RidgefireUpdate.psm1') -Force
if(!$InstallRoot){$InstallRoot=Join-Path ([Environment]::GetFolderPath('LocalApplicationData')) 'IRON SUN'}
Install-RidgefireLauncher -FeedRoot $FeedRoot -InstallRoot $InstallRoot -ToolsRoot $PSScriptRoot -DesktopShortcut:$DesktopShortcut | ConvertTo-Json -Depth 6
