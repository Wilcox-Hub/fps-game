param()
$ErrorActionPreference='Stop'
$run=$PSScriptRoot
$root=Join-Path $run 'fresh-live-install'
$exe='C:/Users/YaYa/AppData/Local/IRON SUN/Launcher/IronSunLauncher.exe'
$expected='C293A7D60887B2D719C753E25E4AE8083A4299513A55FE9C23AD2249AF6378B4'
if((Get-FileHash -LiteralPath $exe).Hash -cne $expected){throw 'Installed launcher differs from the reviewed executable.'}
function Invoke-Live([string]$action){
 $stdout=Join-Path $run ('live-'+$action+'.out')
 $stderr=Join-Path $run ('live-'+$action+'.err')
 $arguments='--install-root "'+$root+'" --no-shortcut --update-test '+$action
 $process=Start-Process -FilePath $exe -ArgumentList $arguments -WindowStyle Hidden -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
 if(!$process.WaitForExit(180000)){throw 'Live updater did not finish in three minutes; process retained for diagnosis.'}
 if($process.ExitCode -ne 0){throw ('Live updater failed: '+[IO.File]::ReadAllText($stderr))}
 return ([IO.File]::ReadAllText($stdout)|ConvertFrom-Json)
}
$check=Invoke-Live 'Check'
if($check.status -cne 'AVAILABLE' -or $check.protocol -ne 60){throw 'Fresh check did not find current VS60.'}
$install=Invoke-Live 'Update'
if($install.status -cne 'UPDATED' -or $install.version -cne '20261012-034847-088'){throw 'Fresh live download/install did not match VS60.'}
$installed=Get-Content -LiteralPath (Join-Path $root 'Game/installed.json') -Raw|ConvertFrom-Json
$verified=0
foreach($file in $installed.files){
 $path=Join-Path (Join-Path $root 'Game') $file.path
 if((Get-Item -LiteralPath $path).Length -ne $file.size -or (Get-FileHash -LiteralPath $path).Hash -ine $file.sha256){throw ('Installed payload differs: '+$file.path)}
 $verified++
}
$current=Invoke-Live 'Update'
if($current.status -cne 'CURRENT'){throw 'Repeated live update did not remain CURRENT.'}
$result=[ordered]@{passed=$true;observed_at_utc=[DateTime]::UtcNow.ToString('o');launcher_sha256=$expected;installed_file_count=$verified;fresh_check=$check;fresh_install=$install;repeat_update=$current;scope='Exact reviewed production binary installed by real setup; actual HTTPS latest feed, RSA signature validation, full VS60 download and all installed file hashes. Backend commands, not normal GUI Install/Play button acceptance.'}
$result|ConvertTo-Json -Depth 8|Set-Content -LiteralPath (Join-Path $run 'live-delivery.json') -Encoding utf8
$result|ConvertTo-Json -Depth 8
