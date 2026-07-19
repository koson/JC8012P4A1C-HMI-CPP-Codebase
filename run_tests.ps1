# ESP32 Unit Test Runner Script
# Usage: .\run_tests.ps1 [component_name]

param(
    [string]$Component = "all",
    [switch]$Monitor = $false,
    [switch]$Flash = $true
)

$ErrorActionPreference = "Stop"

# Colors for output
function Write-Success { Write-Host $args -ForegroundColor Green }
function Write-Error { Write-Host $args -ForegroundColor Red }
function Write-Info { Write-Host $args -ForegroundColor Cyan }

Write-Info "========================================"
Write-Info "ESP32 Unit Test Runner"
Write-Info "========================================"

# Team baseline: keep firmware builds/tests on ESP-IDF 5.5.1.
$RequiredIdfPath = "C:\Users\koson\esp\v5.5.1\esp-idf"

function Ensure-IdfEnvironment {
    if ($env:IDF_PATH -eq $RequiredIdfPath) {
        return
    }

    if (-not (Test-Path (Join-Path $RequiredIdfPath "export.ps1"))) {
        Write-Error "Required ESP-IDF export script not found at $RequiredIdfPath"
        exit 1
    }

    Write-Info "Loading ESP-IDF environment: $RequiredIdfPath"
    & (Join-Path $RequiredIdfPath "export.ps1") | Out-Null

    if ($env:IDF_PATH -ne $RequiredIdfPath) {
        Write-Error "Failed to activate required ESP-IDF environment ($RequiredIdfPath)"
        exit 1
    }
}

Ensure-IdfEnvironment

Write-Info "ESP-IDF Path: $env:IDF_PATH"

# Component test directories
$tests = @{
    "json_parser" = "components\json_renderer\test"
    "svg_parser" = "components\svg_renderer\test"
}

# Function to build and run test
function Run-Test {
    param([string]$Name, [string]$Path)
    
    Write-Info "`n----------------------------------------"
    Write-Info "Testing: $Name"
    Write-Info "----------------------------------------"
    
    if (-not (Test-Path $Path)) {
        Write-Error "Test path not found: $Path"
        return $false
    }
    
    Push-Location $Path
    
    try {
        # Build test
        Write-Info "Building $Name tests..."
        idf.py build
        
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Build failed for $Name"
            return $false
        }
        
        Write-Success "Build successful"
        
        if ($Flash) {
            # Flash test
            Write-Info "Flashing $Name tests..."
            
            # Find COM port
            $port = Get-WmiObject Win32_SerialPort | 
                    Where-Object { $_.Description -match "USB" } | 
                    Select-Object -First 1 -ExpandProperty DeviceID
            
            if (-not $port) {
                Write-Error "No COM port found. Connect ESP32 device."
                return $false
            }
            
            Write-Info "Using port: $port"
            idf.py flash -p $port
            
            if ($LASTEXITCODE -ne 0) {
                Write-Error "Flash failed for $Name"
                return $false
            }
            
            Write-Success "Flash successful"
            
            if ($Monitor) {
                Write-Info "Starting monitor (Ctrl+] to exit)..."
                idf.py monitor -p $port
            }
        }
        
        return $true
    }
    finally {
        Pop-Location
    }
}

# Run tests
$results = @{}

if ($Component -eq "all") {
    foreach ($test in $tests.GetEnumerator()) {
        $results[$test.Key] = Run-Test -Name $test.Key -Path $test.Value
    }
} elseif ($tests.ContainsKey($Component)) {
    $results[$Component] = Run-Test -Name $Component -Path $tests[$Component]
} else {
    Write-Error "Unknown component: $Component"
    Write-Info "Available components: $($tests.Keys -join ', ')"
    exit 1
}

# Summary
Write-Info "`n========================================"
Write-Info "Test Summary"
Write-Info "========================================"

$passed = 0
$failed = 0

foreach ($result in $results.GetEnumerator()) {
    if ($result.Value) {
        Write-Success "✓ $($result.Key): PASSED"
        $passed++
    } else {
        Write-Error "✗ $($result.Key): FAILED"
        $failed++
    }
}

Write-Info "`nTotal: $($results.Count) | Passed: $passed | Failed: $failed"

if ($failed -gt 0) {
    exit 1
}

Write-Success "`nAll tests passed!"
