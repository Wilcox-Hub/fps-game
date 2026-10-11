param()
$ErrorActionPreference='Stop'
Add-Type -AssemblyName System.Net.Http
[Net.ServicePointManager]::SecurityProtocol=[Net.SecurityProtocolType]::Tls12
$client=[Net.Http.HttpClient]::new()
$client.Timeout=[TimeSpan]::FromSeconds(30)
$client.DefaultRequestHeaders.UserAgent.ParseAdd('IronSunReleaseVerification/1.0')
$rsa=[Security.Cryptography.RSACryptoServiceProvider]::new()
$rsa.PersistKeyInCsp=$false
try {
 $base='https://github.com/Wilcox-Hub/iron-sun-releases/releases/latest/download/'
 $bytes=$client.GetByteArrayAsync($base+'game-manifest.json').GetAwaiter().GetResult()
 $sig=$client.GetByteArrayAsync($base+'game-manifest.sig').GetAwaiter().GetResult()
 $rsa.FromXmlString([IO.File]::ReadAllText((Join-Path $PSScriptRoot '../../distribution/iron-sun-launcher/src/ReleasePublicKey.xml')))
 if(!$rsa.VerifyData($bytes,'SHA256',$sig)){throw 'Published game metadata signature did not verify'}
 $m=[Text.Encoding]::UTF8.GetString($bytes)|ConvertFrom-Json
 if($m.protocol -ne 58 -or $m.version -cne '20261011-120705-452'){throw 'Unexpected published game version'}
 $response=$client.SendAsync([Net.Http.HttpRequestMessage]::new([Net.Http.HttpMethod]::Head,$m.archive_url)).GetAwaiter().GetResult()
 $response.EnsureSuccessStatusCode()|Out-Null
 if($response.RequestMessage.RequestUri.Scheme -cne 'https' -or $response.Content.Headers.ContentLength -ne $m.archive_size){throw 'Public archive response did not verify'}
 $zip=Join-Path $PSScriptRoot '../../distribution/IRON-SUN-LAN/iron-sun-20261011-120705-452.zip'
 if((Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash -cne $m.archive_sha256){throw 'Published archive differs from tested local package'}
 [ordered]@{passed=$true;protocol=$m.protocol;game_version=$m.version;metadata_signature_verified=$true;public_archive_http_status=[int]$response.StatusCode;archive_size=$m.archive_size;archive_sha256=$m.archive_sha256;verified_local_archive_identity=$true;scope='Unauthenticated public HTTPS metadata/signature and ZIP HEAD response; local verified archive hash matches the signed published identity. No new ZIP body download or game execution.'}|ConvertTo-Json -Depth 4
} finally {$client.Dispose();$rsa.Dispose()}
