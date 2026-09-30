$Expected = "4293afc3e1725c52d95409ee585b45ed487cd3d397ff7ff1bfdb4786bd067659"
$Path = if ($args.Count -gt 0) { $args[0] } else { ".\net.dll" }
if (-not (Test-Path $Path)) { Write-Error "net.dll not found: $Path"; exit 1 }
$Actual = (Get-FileHash $Path -Algorithm SHA256).Hash.ToLowerInvariant()
Write-Host "SHA-256: $Actual"
if ($Actual -eq $Expected) { Write-Host "Compatible tested net.dll fingerprint."; exit 0 }
Write-Warning "Different net.dll. Do not use the preconfigured encoder offset without porting/testing."
exit 2
