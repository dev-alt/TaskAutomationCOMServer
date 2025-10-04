# Build and Deployment Scripts

This project includes several scripts to streamline development and deployment.

## Quick Start (Windows)

```powershell
# 1. Build the project
.\quick-build.ps1

# 2. Register COM server (requires admin)
.\register.ps1

# 3. Test with the client
.\TaskAutomationCOMClient\x64\Release\TaskAutomationCOMClient.exe
```

## Development Workflow (WSL → Windows)

### 1. Develop in WSL

```bash
# Edit code in WSL using your favorite editor
# All source files are in /root/projects/TaskAutomationCOMServer
```

### 2. Sync to Windows

```bash
# Quick sync (minimal output)
./quick-sync.sh

# Or full sync with details
./sync-to-windows.sh
```

### 3. Build on Windows

```powershell
# Switch to Windows PowerShell
cd C:\Users\andre\Desktop\Projects\TaskAutomationCOMServer

# Quick build
.\quick-build.ps1

# Or full build with options
.\build.ps1 -Configuration Release -Platform x64
```

### 4. Register and Test

```powershell
# Register COM server (opens elevation prompt if needed)
.\register.ps1

# Run test client
.\TaskAutomationCOMClient\x64\Release\TaskAutomationCOMClient.exe

# Test shell extension
# Right-click any folder in Windows Explorer
# Look for "Task Automation" submenu
```

## Script Reference

### WSL Scripts (Linux/Bash)

#### `sync-to-windows.sh`
Full-featured sync with colored output and detailed status.

**Usage:**
```bash
./sync-to-windows.sh
```

**Features:**
- Creates destination directory if missing
- Excludes build artifacts and .git
- Shows detailed file list
- Colored output (green/blue/yellow)
- Final summary with paths

#### `quick-sync.sh`
Minimal output for fast syncing during development.

**Usage:**
```bash
./quick-sync.sh
```

**Features:**
- Same exclusions as full sync
- Minimal output
- Fast execution

---

### Windows Scripts (PowerShell)

#### `build.ps1`
Comprehensive build script with full control.

**Parameters:**
- `-Configuration`: Debug or Release (default: Release)
- `-Platform`: x64 or Win32 (default: x64)
- `-Register`: Register COM server after build (requires admin)
- `-Clean`: Clean build artifacts
- `-Rebuild`: Clean then build

**Usage:**
```powershell
# Standard build
.\build.ps1

# Debug build
.\build.ps1 -Configuration Debug

# Build and register
.\build.ps1 -Register

# Rebuild from scratch
.\build.ps1 -Rebuild

# Clean only
.\build.ps1 -Clean

# Win32 platform
.\build.ps1 -Platform Win32
```

**Features:**
- Auto-detects MSBuild from VS 2019/2022
- Colored output (success/error/info)
- Shows build artifact sizes and timestamps
- Automatic elevation for COM registration
- Displays next steps after build

#### `quick-build.ps1`
Fast build with minimal options - always builds Release x64.

**Usage:**
```powershell
.\quick-build.ps1
```

**Features:**
- No parameters needed
- Fastest way to build
- Shows success/failure clearly
- Displays output paths

#### `register.ps1`
Register or unregister the COM server and shell extension.

**Parameters:**
- `-Unregister`: Unregister instead of register
- `-Platform`: x64 or Win32 (default: x64)
- `-Configuration`: Debug or Release (default: Release)

**Usage:**
```powershell
# Register Release x64
.\register.ps1

# Unregister
.\register.ps1 -Unregister

# Register Debug build
.\register.ps1 -Configuration Debug

# Register Win32 build
.\register.ps1 -Platform Win32
```

**Features:**
- Automatic elevation prompt if not admin
- Validates DLL exists before attempting registration
- Shows full path being registered
- Provides usage hints after successful registration

## Typical Development Cycle

```bash
# In WSL
cd /root/projects/TaskAutomationCOMServer
nano TaskAutomationCOMServer/FileManagement.cpp  # Make changes
./quick-sync.sh                                   # Sync to Windows
```

```powershell
# In Windows PowerShell
cd C:\Users\andre\Desktop\Projects\TaskAutomationCOMServer
.\quick-build.ps1                                 # Build
.\register.ps1                                    # Register (first time or after changes)
.\TaskAutomationCOMClient\x64\Release\TaskAutomationCOMClient.exe  # Test
```

## Troubleshooting

### "MSBuild not found"
Install Visual Studio 2019 or 2022 with "Desktop development with C++" workload.

### "Build failed"
1. Check that all files synced: `.\build.ps1 -Clean` then rebuild
2. Look for missing includes or syntax errors
3. Ensure Windows SDK is installed

### "Registration failed"
1. Run PowerShell as Administrator
2. Or let the script auto-elevate: `.\register.ps1`
3. Check DLL exists: `dir TaskAutomationCOMServer\x64\Release\*.dll`

### Shell extension not appearing
1. Ensure registration succeeded: `.\register.ps1`
2. Restart Windows Explorer:
   - Task Manager → More details → Windows Explorer → Restart
3. Check registry: `HKCR\Directory\shellex\ContextMenuHandlers`

### Changes not taking effect
1. Unregister old version: `.\register.ps1 -Unregister`
2. Rebuild: `.\build.ps1 -Rebuild`
3. Register new version: `.\register.ps1`
4. Restart Explorer if shell extension

## Clean Uninstall

```powershell
# Unregister COM server
.\register.ps1 -Unregister

# Clean build artifacts
.\build.ps1 -Clean

# Delete output directories (optional)
Remove-Item -Recurse -Force x64, Debug, Release, TaskAutomationCOMServer\x64, TaskAutomationCOMClient\x64
```
