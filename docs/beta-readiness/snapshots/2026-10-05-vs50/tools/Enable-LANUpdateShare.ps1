param([Parameter(Mandatory)][string]$FeedRoot,[Parameter(Mandatory)][string]$ReadAccount,[string]$ClientWindowsFolder='',[switch]$Apply)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$feed=(Resolve-Path -LiteralPath $FeedRoot).Path.TrimEnd('\')
$marker=Get-Content -LiteralPath (Join-Path $feed 'managed-feed.json') -Raw | ConvertFrom-Json
if($marker.app_id -cne 'IRON-SUN-LAN' -or $marker.root -ine $feed){throw 'This is not an owned IRON SUN update feed.'}
foreach($item in @((Get-Item -LiteralPath $feed)) + @(Get-ChildItem -LiteralPath $feed -Recurse -Force)){
    if($item.Attributes -band [IO.FileAttributes]::ReparsePoint){throw 'Linked paths cannot be exposed in the update share.'}
}
$cursor=[IO.Directory]::GetParent($feed)
while($cursor){if((Get-Item -LiteralPath $cursor.FullName).Attributes -band [IO.FileAttributes]::ReparsePoint){throw 'Feed ancestor is linked.'};$cursor=$cursor.Parent}
$latest=Get-Content -LiteralPath (Join-Path $feed 'latest.json') -Raw | ConvertFrom-Json
if($latest.app_id -cne 'IRON-SUN-LAN'){throw 'No published game build is ready.'}
$sid=([Security.Principal.NTAccount]::new($ReadAccount)).Translate([Security.Principal.SecurityIdentifier]).Value
if($sid -in @('S-1-1-0','S-1-5-7','S-1-5-32-546') -or $sid.EndsWith('-501')){throw 'Use a specific authenticated Windows account, not Everyone, Anonymous or Guest.'}
$shareName='IRON-SUN-Updates'
$ruleName='IRON-SUN-LAN-UPDATES-SMB'
$existing=Get-SmbShare -Name $shareName -ErrorAction SilentlyContinue
if($existing -and $existing.Path -ine $feed){throw 'An existing share uses this name for another folder.'}
$rule=Get-NetFirewallRule -Name $ruleName -PolicyStore PersistentStore -ErrorAction SilentlyContinue
if($rule){
    $app=$rule | Get-NetFirewallApplicationFilter;$port=$rule | Get-NetFirewallPortFilter;$address=$rule | Get-NetFirewallAddressFilter
    if($rule.Direction -ne 'Inbound' -or $rule.Action -ne 'Allow' -or $rule.Profile.ToString() -ne 'Private' -or $app.Program -ne 'System' -or $port.Protocol -ne 'TCP' -or (@($port.LocalPort) -join ',') -ne '445' -or (@($address.RemoteAddress) -join ',') -ne 'LocalSubnet'){throw 'Existing same-name sharing firewall rule has another scope.'}
}
$plan=[ordered]@{mode=$(if($Apply){'Apply'}else{'Preview'});share_name=$shareName;folder=$feed;unc_path=('\\'+$env:COMPUTERNAME+'\'+$shareName);read_account=$ReadAccount;read_sid=$sid;share_permission='Read';encrypted_smb=$true;firewall_rule=$ruleName;firewall_scope='System / inbound TCP 445 / Private / LocalSubnet / edge traversal Block';build=$latest.version;stable_game_windows=$ClientWindowsFolder;existing_share=[bool]$existing;existing_firewall_rule=[bool]$rule;notes='A named authenticated read-only share for the dedicated game-update folder. Windows credentials stay with Windows; none are embedded in the launcher. Other PCs need one-time setup. No guest/public share, router forwarding, account creation, or global firewall changes.'}
if(!$Apply){$plan | ConvertTo-Json -Depth 6;return}
$principal=[Security.Principal.WindowsPrincipal]::new([Security.Principal.WindowsIdentity]::GetCurrent())
if(!$principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)){throw 'Administrator permission is needed to enable the reviewed share.'}
if((Get-Service LanmanServer).Status -ne 'Running'){throw 'Windows SMB service is not running; no service configuration was changed.'}
if(!(Get-NetConnectionProfile | Where-Object {$_.NetworkCategory -eq 'Private' -and $_.IPv4Connectivity -in @('Internet','LocalNetwork','Subnet')})){throw 'No connected Private IPv4 network is active.'}
$createdShare=$false;$createdRule=$false
try{
    if(!$existing){
        New-SmbShare -Name $shareName -Path $feed -ReadAccess $ReadAccount -EncryptData $true -FolderEnumerationMode AccessBased -CachingMode None | Out-Null
        $createdShare=$true
    }
    $access=@(Get-SmbShareAccess -Name $shareName)
    if($access.Count -ne 1 -or $access[0].AccessControlType -ne 'Allow' -or $access[0].AccessRight -ne 'Read' -or ([Security.Principal.NTAccount]::new($access[0].AccountName)).Translate([Security.Principal.SecurityIdentifier]).Value -ne $sid){throw 'Share permissions are not exactly the reviewed account/read scope.'}
    if(!$rule){
        New-NetFirewallRule -Name $ruleName -DisplayName 'IRON SUN - private LAN update downloads' -Description 'Authenticated game update share. System TCP 445 only, Private networks and LocalSubnet.' -Program System -Protocol TCP -LocalPort 445 -Profile Private -RemoteAddress LocalSubnet -Direction Inbound -Action Allow -Enabled True -EdgeTraversalPolicy Block | Out-Null
        $createdRule=$true
    }elseif($rule.Enabled -ne 'True'){$rule | Enable-NetFirewallRule}
    $active=Get-NetFirewallRule -Name $ruleName -PolicyStore ActiveStore
    $shared=Get-SmbShare -Name $shareName
    if($active.Enabled -ne 'True' -or $active.PrimaryStatus -ne 'OK' -or !$shared.EncryptData){throw 'Share/firewall did not become active with the required settings.'}
    if($ClientWindowsFolder){
        $gameProof=(& (Join-Path $PSScriptRoot 'Configure-LANFirewall.ps1') -WindowsFolder $ClientWindowsFolder -Apply) -join "`n"
        $plan['stable_game_rule']=($gameProof | ConvertFrom-Json)
    }
    $plan['result']='ACTIVE'
    $plan['verified_share_access']=$access | Select-Object AccountName,AccessControlType,AccessRight
    $plan | ConvertTo-Json -Depth 8
}catch{
    if($createdRule){Remove-NetFirewallRule -Name $ruleName}
    if($createdShare){Remove-SmbShare -Name $shareName -Force}
    throw
}
