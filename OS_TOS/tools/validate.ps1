# param (
#     [Parameter(Mandatory = $true)]
#     [string]$FilePath
# )
$FilePath = "data"
if (-Not (Test-Path -Path $FilePath)) {
    Write-Error "Error: File '$FilePath' does not exist."
    exit 1
}

$hashAlgorithm = "SHA256"
$checksum = Get-FileHash -Path $FilePath -Algorithm $hashAlgorithm | Select-Object -ExpandProperty Hash

Write-Output "Disk format started..."
& cmd.exe /c "tostool fs format disk.tos -f --isectors 7 -s 1024"
Write-Output "`nDisk upload started..."
& cmd.exe /c 'tostool fs shell disk.tos -c "upload data data"'
Write-Output "Disk download started..."
& cmd.exe /c 'tostool fs shell disk.tos -c "download data tmp"'

$checksum2 = Get-FileHash -Path "tmp" -Algorithm $hashAlgorithm | Select-Object -ExpandProperty Hash

Write-Output "`nOriginal Checksum: $checksum"
Write-Output "Got Checksum: $checksum2"

if ($checksum -eq $checksum2) {
    Write-Output "`nChecksum matches the expected value."
} else {
    Write-Output "`nChecksum does not match the expected value."
}

Remove-Item tmp
cmd /c 'pause'
