param([string]$Distribution = '')
$ErrorActionPreference = 'Stop'
$taskRoot = $PSScriptRoot
$taskDistroArgs = @()
if ($Distribution) { $taskDistroArgs = @('-d', $Distribution) }
$taskImage = 'ghcr.io/classpaddev/hollyhock-3@sha256:4381a460f8b19411552ac30f2aba73da38d3aba535412a66c69f8528fe94b49a'
$taskLinuxRoot = (& wsl @taskDistroArgs -- wslpath -u $taskRoot).Trim()
if ($LASTEXITCODE -ne 0) { throw 'WSL-Projektpfad konnte nicht ermittelt werden.' }
& wsl @taskDistroArgs -u root -- docker run --rm --network none --cap-drop ALL --security-opt no-new-privileges --mount "type=bind,source=$taskLinuxRoot,target=/work" --workdir /work $taskImage make -j2 hh3
if ($LASTEXITCODE -ne 0) { throw 'HH3-Build fehlgeschlagen.' }
Push-Location -LiteralPath $taskRoot
try {
    & python tools/check_binary.py
    if ($LASTEXITCODE -ne 0) { throw 'HH3-Binarypruefung fehlgeschlagen.' }
} finally { Pop-Location }
Write-Output "Fertig: $taskRoot\dist\mathe.hh3"
