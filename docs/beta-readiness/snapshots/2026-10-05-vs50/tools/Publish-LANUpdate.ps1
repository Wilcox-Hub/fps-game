param([Parameter(Mandatory)][string]$PackageRoot,[Parameter(Mandatory)][string]$FeedRoot,[Parameter(Mandatory)][string]$ArtifactManifest,[string]$ExistingZip='')
$ErrorActionPreference='Stop'
Import-Module (Join-Path $PSScriptRoot 'RidgefireUpdate.psm1') -Force
Publish-RidgefireUpdate -PackageRoot $PackageRoot -FeedRoot $FeedRoot -ArtifactManifest $ArtifactManifest -ExistingZip $ExistingZip | ConvertTo-Json -Depth 6
