$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vswhere) {
    $vsPath = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
    if ($vsPath) {
        $msbuild = Join-Path $vsPath "MSBuild\Current\Bin\MSBuild.exe"
    }
}

if (-not $msbuild -or -not (Test-Path $msbuild)) {
    Write-Host "ERROR: MSBuild not found!" -ForegroundColor Red
    exit 1
}

Write-Host "Building Release x64..." -ForegroundColor Cyan
& $msbuild TaskAutomationCOMServer.sln /t:Build /p:Configuration=Release /p:Platform=x64 /v:minimal /nologo

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build succeeded!" -ForegroundColor Green
    Write-Host "Server: TaskAutomationCOMServer\x64\Release\TaskAutomationCOMServer.dll"
    Write-Host "Client: TaskAutomationCOMClient\x64\Release\TaskAutomationCOMClient.exe"
} else {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}
