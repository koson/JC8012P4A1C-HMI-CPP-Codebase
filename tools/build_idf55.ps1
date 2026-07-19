param(
    [switch]$Flash = $false,
    [switch]$Monitor = $false,
    [string]$Port = ""
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$IdfRoot = "C:\Users\koson\esp\v5.5.1\esp-idf"
$ExportScript = Join-Path $IdfRoot "export.ps1"

if (-not (Test-Path $ExportScript)) {
    Write-Error "ESP-IDF export script not found: $ExportScript"
    exit 1
}

Write-Host "Using ESP-IDF: $IdfRoot"
. $ExportScript | Out-Null

Set-Location $ProjectRoot
idf.py build

if ($Flash) {
    if ([string]::IsNullOrWhiteSpace($Port)) {
        idf.py flash
    }
    else {
        idf.py -p $Port flash
    }
}

if ($Monitor) {
    if ([string]::IsNullOrWhiteSpace($Port)) {
        idf.py monitor
    }
    else {
        idf.py -p $Port monitor
    }
}
