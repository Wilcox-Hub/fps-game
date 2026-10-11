param(
    [string]$WindowsFolder = '',
    [switch]$Apply,
    [switch]$Remove
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
if ($Apply -and $Remove) { throw 'Choose Apply or Remove, not both.' }
if (!$WindowsFolder) {
    $WindowsFolder = Join-Path (Split-Path $PSScriptRoot -Parent) 'Windows'
    if (!(Test-Path -LiteralPath $WindowsFolder -PathType Container)) {
        throw 'Pass -WindowsFolder with the extracted Windows game folder. Preview is the default; -Apply needs administrator PowerShell.'
    }
}
$windowsRoot = (Resolve-Path -LiteralPath $WindowsFolder).Path
$runtime = Join-Path $windowsRoot 'Ridgefire\Binaries\Win64\TP_FirstPerson-Win64-Shipping.exe'
$bootstrap = Join-Path $windowsRoot 'TP_FirstPerson.exe'
if (!(Test-Path -LiteralPath $bootstrap -PathType Leaf) -or !(Test-Path -LiteralPath $runtime -PathType Leaf)) {
    throw 'This is not a complete Windows game folder. Both launcher and Ridgefire/Binaries/Win64 runtime must exist.'
}
$pathBytes = [Text.Encoding]::UTF8.GetBytes($runtime.ToLowerInvariant())
$hasher = [Security.Cryptography.SHA256]::Create()
try { $suffix = ([BitConverter]::ToString($hasher.ComputeHash($pathBytes))).Replace('-','').Substring(0,16) }
finally { $hasher.Dispose() }
$ruleName = 'IRON-SUN-LAN-' + $suffix
$existing = @(Get-NetFirewallRule -Name $ruleName -PolicyStore PersistentStore -ErrorAction SilentlyContinue)
if ($existing.Count -gt 1) { throw 'More than one matching rule exists; refusing to change firewall rules.' }
if ($existing.Count -eq 1) {
    $app = $existing[0] | Get-NetFirewallApplicationFilter
    $port = $existing[0] | Get-NetFirewallPortFilter
    $address = $existing[0] | Get-NetFirewallAddressFilter
    $safe = $app.Program -eq $runtime -and $existing[0].Direction -eq 'Inbound' -and $existing[0].Action -eq 'Allow' -and $existing[0].Profile.ToString() -eq 'Private' -and $port.Protocol -eq 'UDP' -and (@($port.LocalPort | Sort-Object) -join ',') -eq '14001,7777' -and (@($address.RemoteAddress) -join ',') -eq 'LocalSubnet'
    if (!$safe) { throw 'A same-name rule has a different scope; refusing to change or remove it.' }
}
$profiles = @(Get-NetConnectionProfile | Select-Object InterfaceAlias,@{Name='Category';Expression={$_.NetworkCategory.ToString()}},@{Name='Connectivity';Expression={$_.IPv4Connectivity.ToString()}})
$plan = [ordered]@{
    mode = $(if ($Apply) {'Apply'} elseif ($Remove) {'Remove'} else {'Preview'})
    rule_name = $ruleName
    program = $runtime
    program_sha256 = (Get-FileHash -LiteralPath $runtime -Algorithm SHA256).Hash
    direction = 'Inbound'
    protocol = 'UDP'
    local_ports = @('7777','14001')
    profile = 'Private'
    remote_address = 'LocalSubnet'
    existing_matching_rule = ($existing.Count -eq 1)
    network_profiles = $profiles
    notes = '7777 is the default gameplay port; 14001 is LAN discovery. This helper never changes network categories, VPN/adapters, router settings, global firewall defaults, or other application rules. Custom gameplay ports require an explicitly scoped separate rule.'
}
if (!$Apply -and !$Remove) { $plan | ConvertTo-Json -Depth 5; return }
$identity = [Security.Principal.WindowsIdentity]::GetCurrent()
$principal = New-Object Security.Principal.WindowsPrincipal($identity)
if (!$principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    throw 'This change needs administrator PowerShell. Preview did not change any settings.'
}
if ($Remove) {
    if ($existing.Count -eq 1) { $existing[0] | Remove-NetFirewallRule }
    $plan['result'] = 'REMOVED_OR_ALREADY_ABSENT'
} else {
    if (!($profiles | Where-Object {$_.Category -eq 'Private' -and $_.Connectivity -in @('Internet','LocalNetwork','Subnet')})) {
        throw 'No connected Private IPv4 network is active. Review your home network category yourself; this helper will not change it.'
    }
    if ($existing.Count -eq 0) {
        New-NetFirewallRule -Name $ruleName -DisplayName 'IRON SUN - home LAN discovery and gameplay' -Description 'Only this extracted game runtime, inbound UDP 7777/14001, Private networks and LocalSubnet. Remove using this helper -Remove.' -PolicyStore PersistentStore -Direction Inbound -Action Allow -Enabled True -Profile Private -Program $runtime -Protocol UDP -LocalPort 7777,14001 -RemoteAddress LocalSubnet -EdgeTraversalPolicy Block | Out-Null
    } elseif ($existing[0].Enabled -ne 'True') {
        $existing[0] | Enable-NetFirewallRule
    }
    $active = Get-NetFirewallRule -Name $ruleName -PolicyStore ActiveStore
    if ($active.Enabled -ne 'True' -or $active.PrimaryStatus -ne 'OK') { throw 'Rule was saved but is not active. Check your device firewall policy.' }
    $plan['result'] = 'ACTIVE'
    $plan['verification'] = 'Exact local firewall rule present and active; discovery across two PCs still needs a real refresh/join test. Explicit block rules and router isolation can still prevent traffic.'
}
$plan | ConvertTo-Json -Depth 5

