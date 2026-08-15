# PowerShell build script for SENTAnalyzer Saleae Logic 2 Plugin
$ErrorActionPreference = "Stop"

$BuildDir = Join-Path $PSScriptRoot "build"
$TargetDll = "C:\Program Files\Logic\resources\windows-x64\Analyzers\SENTAnalyzer.dll"

Write-Host "=== Building SENTAnalyzer for Saleae Logic 2 ===" -ForegroundColor Cyan

if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

Push-Location $BuildDir
try {
    Write-Host "[1/3] Configuring CMake..." -ForegroundColor Yellow
    cmake .. -DCMAKE_BUILD_TYPE=Release

    Write-Host "[2/3] Building DLL..." -ForegroundColor Yellow
    cmake --build . --config Release

    $BuiltDll = Join-Path $BuildDir "Analyzers\Release\SENTAnalyzer.dll"
    if (-not (Test-Path $BuiltDll)) {
        $BuiltDll = Join-Path $BuildDir "Analyzers\SENTAnalyzer.dll"
    }

    if (Test-Path $BuiltDll) {
        Write-Host "[3/3] Build SUCCESS! Output DLL: $BuiltDll" -ForegroundColor Green
        
        # Check if user wants to copy to Saleae Logic 2 directory
        if (Test-Path "C:\Program Files\Logic\resources\windows-x64\Analyzers") {
            try {
                Copy-Item -Path $BuiltDll -Destination $TargetDll -Force
                Write-Host "-> Successfully installed to: $TargetDll" -ForegroundColor Green
                Write-Host "-> Please restart Saleae Logic 2 to apply the updated plugin!" -ForegroundColor Cyan
            } catch {
                Write-Warning "Could not copy directly to Program Files (requires Admin privileges). Please copy '$BuiltDll' to '$TargetDll' manually."
            }
        }
    } else {
        Write-Error "Build finished but SENTAnalyzer.dll was not found."
    }
} finally {
    Pop-Location
}
