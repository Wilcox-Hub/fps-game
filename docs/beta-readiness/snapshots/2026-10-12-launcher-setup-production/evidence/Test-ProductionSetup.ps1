$ErrorActionPreference='Stop'
$root='C:/Users/YaYa/AppData/Local/IRON SUN'
$run=$PSScriptRoot
$setup=Join-Path $run 'protected-setup-download/IRON-SUN-Setup.exe'
$registration='HKCU:/Software/Microsoft/Windows/CurrentVersion/Uninstall/{47F9B5B4-36A4-46F6-B87C-1AC30C5CE690}_is1'
if((Get-FileHash -LiteralPath $setup).Hash -cne '62C789A0ABD26F2CA127635AD61FDE7B41537AAD31C4C29A2D54FF754464C9F8'){throw 'Downloaded production setup differs.'}
$owned=Get-Content -LiteralPath (Join-Path $root 'managed-install.json') -Raw|ConvertFrom-Json
if($owned.app_id -cne 'IRON-SUN-LAN' -or [IO.Path]::GetFullPath($owned.root) -ine [IO.Path]::GetFullPath($root)){throw 'Owner installation marker differs.'}
$retained=@(Get-ChildItem -LiteralPath $root -Recurse -File|Where-Object { !$_.FullName.StartsWith(([IO.Path]::GetFullPath((Join-Path $root 'Launcher'))+'\'),[StringComparison]::OrdinalIgnoreCase) }|ForEach-Object { @{path=$_.FullName;sha256=(Get-FileHash -LiteralPath $_.FullName).Hash} })
$ownerShortcut='C:/Users/YaYa/Desktop/IRON SUN.lnk'
$ownerShortcutHash=(Get-FileHash -LiteralPath $ownerShortcut).Hash
# Stop only the test-started launcher and duplicate setup, after checking their actual paths.
foreach($taskId in @(33972,7588,27020)){
 $task=Get-Process -Id $taskId -ErrorAction SilentlyContinue
 if($task){
  if($task.Path -notmatch '(?i)IRON SUN\\Launcher\\IronSunLauncher\.exe$|is-4G01MY40KM\.tmp\\iron-sun-setup\.tmp$|FPS-1\\distribution\\iron-sun-launcher\\build\\setup\\iron-sun-setup\.exe$'){throw 'A test process identity changed.'}
  Stop-Process -Id $taskId
 }
}
$uninstaller=Join-Path $root 'Launcher/unins000.exe'
if(!(Test-Path -LiteralPath $registration) -or !(Test-Path -LiteralPath $uninstaller)){throw 'Production setup registration unavailable.'}
$uninstall=Start-Process -FilePath $uninstaller -ArgumentList ('/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /LOG="'+(Join-Path $run 'production-uninstall.log')+'"') -WindowStyle Hidden -PassThru
if(!$uninstall.WaitForExit(60000) -or $uninstall.ExitCode -ne 0){throw 'Production uninstall failed.'}
if(Test-Path -LiteralPath $registration){throw 'Uninstall left app registration.'}
foreach($item in $retained){if(!(Test-Path -LiteralPath $item.path) -or (Get-FileHash -LiteralPath $item.path).Hash -cne $item.sha256){throw 'Uninstall changed retained owner data.'}}
if((Get-FileHash -LiteralPath $ownerShortcut).Hash -cne $ownerShortcutHash){throw 'Owner LAN shortcut changed.'}
$reinstall=Start-Process -FilePath $setup -ArgumentList ('/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /TASKS=desktopicon /LOG="'+(Join-Path $run 'downloaded-setup-install.log')+'"') -WindowStyle Hidden -PassThru
if(!$reinstall.WaitForExit(60000) -or $reinstall.ExitCode -ne 0){throw 'Downloaded setup installation failed.'}
if(!(Test-Path -LiteralPath $registration)){throw 'Downloaded setup missing app registration.'}
$exe=Join-Path $root 'Launcher/IronSunLauncher.exe'
if((Get-FileHash -LiteralPath $exe).Hash -cne 'C293A7D60887B2D719C753E25E4AE8083A4299513A55FE9C23AD2249AF6378B4'){throw 'Reinstalled launcher differs.'}
$shell=New-Object -ComObject WScript.Shell
$shortcuts=@('C:/Users/YaYa/Desktop/IRON SUN Launcher.lnk','C:/Users/YaYa/AppData/Roaming/Microsoft/Windows/Start Menu/Programs/IRON SUN Launcher.lnk')
foreach($path in $shortcuts){if(!(Test-Path -LiteralPath $path) -or [IO.Path]::GetFullPath($shell.CreateShortcut($path).TargetPath) -ine [IO.Path]::GetFullPath($exe)){throw 'Downloaded setup shortcut invalid.'}}
foreach($item in $retained){if((Get-FileHash -LiteralPath $item.path).Hash -cne $item.sha256){throw 'Reinstall changed retained owner data.'}}
$result=[ordered]@{passed=$true;observed_at_utc=[DateTime]::UtcNow.ToString('o');setup_sha256=(Get-FileHash -LiteralPath $setup).Hash;launcher_sha256=(Get-FileHash -LiteralPath $exe).Hash;retained_file_count=$retained.Count;uninstall_accepted=$true;reinstall_accepted=$true;app_registered=$true;start_menu_shortcut=$true;desktop_shortcut=$true;owner_lan_shortcut_retained=$true;scope='Real production uninstall and reinstallation of GitHub CLI-downloaded setup with protection enabled. All existing non-launcher files and owner shortcut preserved by hash; setup GUI completion/open observed separately. Browser/SmartScreen, normal launcher buttons and second PC pending.'}
$result|ConvertTo-Json -Depth 5|Set-Content -LiteralPath (Join-Path $run 'production-setup.json') -Encoding utf8
$result|ConvertTo-Json -Depth 5
