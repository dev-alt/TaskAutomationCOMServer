param(
    [switch]$Unregister,
    [ValidateSet("x64", "Win32")]
    [string]$Platform = "x64",
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release"
)

$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "This script requires administrator privileges!" -ForegroundColor Yellow
    Write-Host "Restarting with elevation..."

    $scriptPath = $MyInvocation.MyCommand.Path
    $args = if ($Unregister) { "-Unregister" } else { "" }
    $args += " -Platform $Platform -Configuration $Configuration"

    Start-Process powershell.exe -Verb RunAs -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$scriptPath`" $args"
    exit 0
}

$dllPath = "TaskAutomationCOMServer\$Platform\$Configuration\TaskAutomationCOMServer.dll"

if (-not (Test-Path $dllPath)) {
    Write-Host "ERROR: DLL not found at: $dllPath" -ForegroundColor Red
    Write-Host "Please build the project first: .\build.ps1"
    exit 1
}

$fullPath = (Resolve-Path $dllPath).Path

if ($Unregister) {
    Write-Host "Unregistering COM server..." -ForegroundColor Cyan
    & regsvr32 /u /s $fullPath

    if ($LASTEXITCODE -eq 0) {
        Write-Host "COM server unregistered successfully!" -ForegroundColor Green
    } else {
        Write-Host "Unregistration failed!" -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "Registering COM server..." -ForegroundColor Cyan
    & regsvr32 /s $fullPath

    if ($LASTEXITCODE -eq 0) {
        Write-Host "COM server registered successfully!" -ForegroundColor Green
        Write-Host ""
        Write-Host "Shell extension is now active in Windows Explorer"
        Write-Host "Right-click any folder to see 'Task Automation' menu"
    } else {
        Write-Host "Registration failed!" -ForegroundColor Red
        exit 1
    }
}
