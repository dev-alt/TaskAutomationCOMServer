param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",

    [ValidateSet("x64", "Win32")]
    [string]$Platform = "x64",

    [switch]$Register,
    [switch]$Clean,
    [switch]$Rebuild
)

function Write-Success { Write-Host $args -ForegroundColor Green }
function Write-Info { Write-Host $args -ForegroundColor Cyan }
function Write-Err { Write-Host $args -ForegroundColor Red }

Write-Info "=== TaskAutomationCOMServer Build Script ==="
Write-Host ""

# Find MSBuild
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$msbuild = $null

if (Test-Path $vswhere) {
    $vsPath = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
    if ($vsPath) {
        $msbuild = Join-Path $vsPath "MSBuild\Current\Bin\MSBuild.exe"
    }
}

if (-not $msbuild -or -not (Test-Path $msbuild)) {
    Write-Err "ERROR: MSBuild not found!"
    Write-Host "Please install Visual Studio 2019 or 2022"
    exit 1
}

Write-Info "Using MSBuild: $msbuild"
Write-Host ""

$solution = "TaskAutomationCOMServer.sln"
$buildTarget = if ($Rebuild) { "Rebuild" } elseif ($Clean) { "Clean" } else { "Build" }

Write-Info "Configuration: $Configuration | Platform: $Platform | Target: $buildTarget"
Write-Host ""

# Clean
if ($Clean -or $Rebuild) {
    Write-Info "Cleaning..."
    & $msbuild $solution /t:Clean /p:Configuration=$Configuration /p:Platform=$Platform /v:minimal /nologo
    if ($Clean) { exit 0 }
}

# Build
Write-Info "Building..."
& $msbuild $solution /t:$buildTarget /p:Configuration=$Configuration /p:Platform=$Platform /v:minimal /nologo

if ($LASTEXITCODE -ne 0) {
    Write-Err "Build failed!"
    exit $LASTEXITCODE
}

Write-Success "Build succeeded!"
Write-Host ""

$serverDll = "TaskAutomationCOMServer\$Platform\$Configuration\TaskAutomationCOMServer.dll"
$clientExe = "TaskAutomationCOMClient\$Platform\$Configuration\TaskAutomationCOMClient.exe"

if (Test-Path $serverDll) {
    Write-Info "Server: $serverDll"
}
if (Test-Path $clientExe) {
    Write-Info "Client: $clientExe"
}
Write-Host ""

# Register
if ($Register) {
    $isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

    if (-not $isAdmin) {
        Write-Host "Restarting with admin privileges..."
        $scriptPath = $MyInvocation.MyCommand.Path
        Start-Process powershell.exe -Verb RunAs -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$scriptPath`" -Configuration $Configuration -Platform $Platform -Register" -Wait
        exit 0
    }

    Write-Info "Registering COM server..."
    $fullPath = (Resolve-Path $serverDll).Path
    & regsvr32 /s $fullPath

    if ($LASTEXITCODE -eq 0) {
        Write-Success "COM server registered!"
    } else {
        Write-Err "Registration failed!"
        exit 1
    }
} else {
    Write-Info "Next: .\register.ps1"
}
