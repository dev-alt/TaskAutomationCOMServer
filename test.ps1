param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",
    [ValidateSet("x64", "Win32")]
    [string]$Platform = "x64"
)

Write-Host "=== TaskAutomationCOMServer Test Script ===" -ForegroundColor Cyan
Write-Host ""

Write-Host "Step 1: Building..." -ForegroundColor Yellow
& .\build.ps1 -Configuration $Configuration -Platform $Platform

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed! Aborting." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Step 2: Registering..." -ForegroundColor Yellow
& .\register.ps1 -Configuration $Configuration -Platform $Platform

if ($LASTEXITCODE -ne 0) {
    Write-Host "Registration failed! Aborting." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Step 3: Running test client..." -ForegroundColor Yellow
Write-Host ""

$clientExe = "TaskAutomationCOMClient\$Platform\$Configuration\TaskAutomationCOMClient.exe"

if (Test-Path $clientExe) {
    & $clientExe
} else {
    Write-Host "Client not found: $clientExe" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "Test complete!" -ForegroundColor Green
