$ErrorActionPreference='Stop'
$helper='C:\Users\YaYa\Documents\Codex\2026-09-22\connect-this-github-repo-to-a\unreal\Ridgefire\Tools\Configure-LANFirewall.ps1'
$windowsFolder='C:\Users\YaYa\Documents\Codex\2026-09-22\connect-this-github-repo-to-a\unreal\Ridgefire\Saved\Packages\20261005-134833-568\Windows'
$expectedHelperHash='535EC3BF6DD225E997A2866CCC4ACEC768BB8A54E2CCBF8938D8BA433886A9F8'
$expectedRuntimeHash='4EC2C7520039C8C1CE1BE3B697288E9F5561EAB3474AF1448720B5E69E056277'
try {
    if((Get-FileHash -LiteralPath $helper -Algorithm SHA256).Hash -ne $expectedHelperHash){throw 'Helper changed since owner approval.'}
    $runtime=Join-Path $windowsFolder 'Ridgefire\Binaries\Win64\TP_FirstPerson-Win64-Shipping.exe'
    if((Get-FileHash -LiteralPath $runtime -Algorithm SHA256).Hash -ne $expectedRuntimeHash){throw 'Game runtime changed since owner approval.'}
    $output=& $helper -WindowsFolder $windowsFolder -Apply
    $result=($output -join "`n") | ConvertFrom-Json
    if($result.result -ne 'ACTIVE'){throw 'Helper did not verify an active rule.'}
    $output | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'firewall-applied.json') -Encoding UTF8
    exit 0
}catch{
    [ordered]@{result='FAILED';error=$_.Exception.Message} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $PSScriptRoot 'firewall-apply-error.json') -Encoding UTF8
    exit 1
}
