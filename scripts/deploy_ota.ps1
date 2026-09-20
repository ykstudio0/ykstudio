# -------------------------------------------------------------
#  File : deploy_ota.ps1
#  Author : JongOh Kim + ChatGPT
#  Date : 2026-09-20
#  Project : SVEMS
#  Version : 0.8.3
#  Description : deploy_ota.ps1
# -------------------------------------------------------------

param(
    [Parameter(Mandatory = $true)]
    [ValidateSet("main-test", "main-car")]
    [string]$Target,

    [Parameter(Mandatory = $true)]
    [string]$Version
)

$ErrorActionPreference = "Stop"

#---------------------------------------------------------
# Project Paths
#---------------------------------------------------------

$ProjectRoot = Split-Path -Parent $PSScriptRoot

if ($Target -eq "main-test")
{
    $BuildEnv = "svems-test"
}
elseif ($Target -eq "main-car")
{
    $BuildEnv = "svems-main"
}

$FirmwarePath =
    Join-Path `
        $ProjectRoot `
        ".pio\build\$BuildEnv\firmware.bin"

$ImageShaScript =
    Join-Path `
        $PSScriptRoot `
        "get_esp_image_sha.py"

$LatestJsonPath =
    Join-Path `
        $PSScriptRoot `
        "latest.json"

#---------------------------------------------------------
# Server Settings
#---------------------------------------------------------

$ServerUser = "svems"
$ServerHost = "192.168.0.15"

$ServerBase =
    "/home/svems/svems-server/firmware/$Target"

#---------------------------------------------------------
# Validate Files
#---------------------------------------------------------

if (!(Test-Path $FirmwarePath))
{
    throw "Firmware not found: $FirmwarePath"
}

if (!(Test-Path $ImageShaScript))
{
    throw "SHA script not found: $ImageShaScript"
}

Write-Host ""
Write-Host "========================================="
Write-Host " SVEMS OTA DEPLOY"
Write-Host "========================================="
Write-Host "Target  : $Target"
Write-Host "Version : $Version"
Write-Host "Firmware: $FirmwarePath"
Write-Host ""

#---------------------------------------------------------
# File SHA256
#---------------------------------------------------------

$FileSha256 =
    (
        Get-FileHash `
            -Algorithm SHA256 `
            -Path $FirmwarePath
    ).Hash.ToLower()

Write-Host "File SHA256:"
Write-Host $FileSha256
Write-Host ""

#---------------------------------------------------------
# ESP32 Image SHA256
#---------------------------------------------------------

$ImageSha256 =
    (
        python `
            $ImageShaScript `
            $FirmwarePath
    ).Trim()

if (
    $LASTEXITCODE -ne 0 -or
    $ImageSha256.Length -ne 64
)
{
    throw "ESP32 Image SHA256 calculation failed."
}

Write-Host "Image SHA256:"
Write-Host $ImageSha256
Write-Host ""

#---------------------------------------------------------
# Create latest.json
#---------------------------------------------------------

$LatestJson = [ordered]@{
    version     = $Version
    file        = "firmware.bin"
    sha256      = $FileSha256
    imageSha256 = $ImageSha256
}

$JsonText =
    $LatestJson |
    ConvertTo-Json

$Utf8NoBom =
    New-Object System.Text.UTF8Encoding($false)

[System.IO.File]::WriteAllText(
    $LatestJsonPath,
    $JsonText,
    $Utf8NoBom
)

Write-Host "latest.json created:"
Get-Content $LatestJsonPath
Write-Host ""

#---------------------------------------------------------
# Upload Temporary Files
#---------------------------------------------------------

Write-Host "Uploading temporary files..."

scp `
    $FirmwarePath `
    "${ServerUser}@${ServerHost}:${ServerBase}/firmware.bin.tmp"

if ($LASTEXITCODE -ne 0)
{
    throw "Firmware upload failed."
}

scp `
    $LatestJsonPath `
    "${ServerUser}@${ServerHost}:${ServerBase}/latest.json.tmp"

if ($LASTEXITCODE -ne 0)
{
    throw "latest.json upload failed."
}

#---------------------------------------------------------
# Activate Files
#---------------------------------------------------------

Write-Host ""
Write-Host "Activating OTA files..."

$RemoteCommand =
    "mv '$ServerBase/firmware.bin.tmp' '$ServerBase/firmware.bin' && " +
    "mv '$ServerBase/latest.json.tmp' '$ServerBase/latest.json'"

ssh `
    "${ServerUser}@${ServerHost}" `
    $RemoteCommand

if ($LASTEXITCODE -ne 0)
{
    throw "Remote activation failed."
}

#---------------------------------------------------------
# Done
#---------------------------------------------------------

Write-Host ""
Write-Host "========================================="
Write-Host " OTA DEPLOY COMPLETE"
Write-Host "========================================="
Write-Host "Target : $Target"
Write-Host "Version: $Version"
Write-Host ""