$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$tools=Join-Path $PSScriptRoot 'source\Tools'
Import-Module (Join-Path $tools 'RidgefireUpdate.psm1') -Force
Add-Type -AssemblyName System.IO.Compression.FileSystem
$root=Join-Path $PSScriptRoot ('checks-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Path $root | Out-Null
$feed=Join-Path $root 'Feed';$client=Join-Path $root 'Client'
$checks=@()
function Check([string]$Name,[bool]$Value){if(!$Value){throw "FAILED: $Name"};$script:checks += [pscustomobject]@{check=$Name;status='PASS'};Write-Host ('PASS: '+$Name)}
function Reject([string]$Name,[scriptblock]$Action,[string]$Message){$caught='';try{& $Action | Out-Null}catch{$caught=$_.Exception.Message};Check $Name ($caught -like ('*'+$Message+'*'))}
function JsonWrite([string]$Path,$Value){$Value | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath $Path -Encoding UTF8}
function Fixture([string]$Version){
    $package=Join-Path $root $Version;$windows=Join-Path $package 'Windows'
    $paths=@('TP_FirstPerson.exe','Ridgefire/Binaries/Win64/TP_FirstPerson-Win64-Shipping.exe','Ridgefire/Content/Paks/Ridgefire-Windows.pak','Ridgefire/Content/Paks/Ridgefire-Windows.ucas','Ridgefire/Content/Paks/Ridgefire-Windows.utoc')
    $artifacts=@()
    foreach($rel in $paths){
        $path=Join-Path $windows $rel;New-Item -ItemType Directory -Path (Split-Path $path -Parent) -Force | Out-Null
        if($rel.EndsWith('.exe')){Copy-Item -LiteralPath (Join-Path $env:SystemRoot 'System32\cmd.exe') -Destination $path}
        else{[IO.File]::WriteAllText($path,($Version+'/'+$rel))}
        $artifacts += [ordered]@{path=$rel;size_bytes=(Get-Item -LiteralPath $path).Length;sha256=(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash}
    }
    JsonWrite (Join-Path $package 'PackageInfo.json') ([ordered]@{Status='COMPLETE';Configuration='Shipping';ProductStatus='Prototype';Executable='Windows/TP_FirstPerson.exe';BuildTimeUtc=[DateTime]::UtcNow.ToString('o')})
    $proof=Join-Path $package 'artifacts.json';JsonWrite $proof ([ordered]@{artifacts=$artifacts})
    return [pscustomobject]@{package=$package;proof=$proof}
}
try{
    $first=Fixture 'fixture-1';$second=Fixture 'fixture-2';$third=Fixture 'fixture-3'
    Publish-RidgefireUpdate -PackageRoot $first.package -FeedRoot $feed -ArtifactManifest $first.proof | Out-Null
    $one=Invoke-RidgefireUpdate -FeedRoot $feed -InstallRoot $client -NoLaunch
    Check 'First install from verified feed' ($one.status -eq 'UPDATED' -and $one.version -eq 'fixture-1')
    $repeat=Invoke-RidgefireUpdate -FeedRoot $feed -InstallRoot $client -NoLaunch
    Check 'Already-current launch does not reinstall' ($repeat.status -eq 'CURRENT' -and !(Test-Path (Join-Path $client 'Previous')))
    Publish-RidgefireUpdate -PackageRoot $second.package -FeedRoot $feed -ArtifactManifest $second.proof | Out-Null
    $two=Invoke-RidgefireUpdate -FeedRoot $feed -InstallRoot $client -NoLaunch
    $old=Get-Content (Join-Path $client 'Previous\installed.json') -Raw | ConvertFrom-Json
    Check 'Update preserves previous version and fixed runtime path' ($two.status -eq 'UPDATED' -and $two.version -eq 'fixture-2' -and $old.version -eq 'fixture-1' -and (Test-Path (Join-Path $client 'Game\Windows\Ridgefire\Binaries\Win64\TP_FirstPerson-Win64-Shipping.exe')))
    $offline=Invoke-RidgefireUpdate -FeedRoot (Join-Path $root 'Unavailable') -InstallRoot $client -NoLaunch
    Check 'Offline launch retains verified installed version' ($offline.status -eq 'OFFLINE_CURRENT' -and $offline.version -eq 'fixture-2')
    Reject 'Offline first install cannot fabricate success' {Invoke-RidgefireUpdate -FeedRoot (Join-Path $root 'Unavailable') -InstallRoot (Join-Path $root 'EmptyClient') -NoLaunch} 'no game is installed'
    $heldLock=[IO.File]::Open((Join-Path $client 'update.lock'),[IO.FileMode]::Open,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None)
    try{Reject 'Concurrent launcher cannot mutate installation' {Invoke-RidgefireUpdate -FeedRoot $feed -InstallRoot $client -NoLaunch} 'Another IRON SUN launcher'}finally{$heldLock.Dispose()}
    Publish-RidgefireUpdate -PackageRoot $third.package -FeedRoot $feed -ArtifactManifest $third.proof | Out-Null
    $latest=Join-Path $feed 'latest.json';$good=Get-Content $latest -Raw | ConvertFrom-Json
    $bad=Get-Content $latest -Raw | ConvertFrom-Json;$bad.archive_sha256=('0'*64);JsonWrite $latest $bad
    Reject 'Corrupt download is rejected before replacing game' {Invoke-RidgefireUpdate -FeedRoot $feed -InstallRoot $client -NoLaunch} 'checksum'
    $still=Get-Content (Join-Path $client 'Game\installed.json') -Raw | ConvertFrom-Json
    Check 'Failed update leaves current and previous intact' ($still.version -eq 'fixture-2' -and (Test-Path (Join-Path $client 'Previous\installed.json')) -and @(Get-ChildItem $client -Directory -Filter 'Staging-*').Count -eq 0)
    JsonWrite $latest $good
    $archive=Join-Path $feed $good.archive;$archiveCopy=Join-Path $root 'good-third.zip';Copy-Item $archive $archiveCopy
    $zip=[IO.Compression.ZipFile]::Open($archive,[IO.Compression.ZipArchiveMode]::Update)
    try{$entry=$zip.CreateEntry('Windows/../../escape.txt');$writer=[IO.StreamWriter]::new($entry.Open());$writer.Write('must not escape');$writer.Dispose()}finally{$zip.Dispose()}
    $bad=Get-Content $latest -Raw | ConvertFrom-Json;$bad.archive_size=(Get-Item $archive).Length;$bad.archive_sha256=(Get-FileHash $archive -Algorithm SHA256).Hash;JsonWrite $latest $bad
    Reject 'Archive path traversal is rejected' {Invoke-RidgefireUpdate -FeedRoot $feed -InstallRoot $client -NoLaunch} 'Unsafe update path'
    Check 'Traversal writes no escaped file' (!(Test-Path (Join-Path $client 'escape.txt')) -and !(Test-Path (Join-Path $root 'escape.txt')))
    Copy-Item $archiveCopy $archive -Force;JsonWrite $latest $good
    $fakeGame=Start-Process -FilePath (Join-Path $client 'Game\Windows\Ridgefire\Binaries\Win64\TP_FirstPerson-Win64-Shipping.exe') -ArgumentList '/c ping -n 6 127.0.0.1' -PassThru -WindowStyle Hidden -RedirectStandardOutput (Join-Path $root 'fake-process.log')
    try{Reject 'Running installed game blocks replacement' {Invoke-RidgefireUpdate -FeedRoot $feed -InstallRoot $client -NoLaunch} 'Close IRON SUN'}finally{if(!$fakeGame.HasExited){Stop-Process -InputObject $fakeGame}}
    $three=Invoke-RidgefireUpdate -FeedRoot $feed -InstallRoot $client -NoLaunch
    Check 'Update succeeds after game closes' ($three.version -eq 'fixture-3')
    $back=Invoke-RidgefireUpdate -FeedRoot $feed -InstallRoot $client -NoLaunch -Rollback
    Check 'Rollback restores previous complete version' ($back.status -eq 'ROLLED_BACK' -and $back.version -eq 'fixture-2')
    $pinned=Invoke-RidgefireUpdate -FeedRoot $feed -InstallRoot $client -NoLaunch
    Check 'Rollback is not undone at next launch' ($pinned.status -eq 'ROLLBACK_HELD' -and $pinned.version -eq 'fixture-2')
    # Simulate process interruption after moving old Game to Previous.
    $interrupted=Join-Path $root 'InterruptedClient'
    Invoke-RidgefireUpdate -FeedRoot $feed -InstallRoot $interrupted -NoLaunch | Out-Null
    Move-Item -LiteralPath (Join-Path $interrupted 'Game') -Destination (Join-Path $interrupted 'Previous')
    $recovered=Invoke-RidgefireUpdate -FeedRoot (Join-Path $root 'Unavailable') -InstallRoot $interrupted -NoLaunch
    Check 'Interrupted directory swap recovers old game offline' ($recovered.version -eq 'fixture-3' -and (Test-Path (Join-Path $interrupted 'Game\installed.json')))
    $foreign=Join-Path $root 'Foreign';New-Item -ItemType Directory $foreign | Out-Null;Set-Content (Join-Path $foreign 'keep.txt') 'owned by user'
    Reject 'Foreign install directory is preserved' {Invoke-RidgefireUpdate -FeedRoot $feed -InstallRoot $foreign -NoLaunch} 'without an IRON SUN ownership marker'
    Check 'Foreign file remains unchanged' ((Get-Content (Join-Path $foreign 'keep.txt')) -eq 'owned by user')
    $badProof=Get-Content $first.proof -Raw | ConvertFrom-Json;$badProof.artifacts[0].sha256=('0'*64);$badProofPath=Join-Path $root 'wrong-proof.json';JsonWrite $badProofPath $badProof
    Reject 'Publisher rejects build that differs from tested evidence' {Publish-RidgefireUpdate -PackageRoot $first.package -FeedRoot $feed -ArtifactManifest $badProofPath} 'does not match recorded tested artifacts'
    $installer=Install-RidgefireLauncher -FeedRoot $feed -InstallRoot (Join-Path $root 'InstallerClient') -ToolsRoot $tools
    Check 'Installer creates configured updater and launch/rollback entries' ($installer.game_launched -eq $false -and (Test-Path $installer.launcher) -and (Test-Path (Join-Path $installer.install_root 'client.json')) -and (Test-Path (Join-Path $installer.install_root 'Rollback-IRON-SUN.cmd')))
    [ordered]@{passed=$true;checks=$checks;scope='Production PowerShell updater and publisher with small synthetic ZIP fixtures; copied Microsoft cmd executable only for the running-process guard. Real disk moves, archive hashes, corruption/traversal/locking, offline recovery, rollback hold and installer tested. No game play, network share or second-PC acceptance.'} | ConvertTo-Json -Depth 6 | Set-Content (Join-Path $root 'result.json') -Encoding UTF8
    Write-Host ('ALL PASS / '+$checks.Count+' checks / '+$root)
}catch{
    [ordered]@{passed=$false;checks=$checks;error=$_.Exception.Message} | ConvertTo-Json -Depth 6 | Set-Content (Join-Path $root 'result.json') -Encoding UTF8
    throw
}
