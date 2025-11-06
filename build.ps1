#  Windows PowerShell build script
# Run this script in PowerShell (not CMD)

Write-Host "What do you want to render?" -ForegroundColor Cyan
Write-Host "1) Black Hole"
Write-Host "2) Fractals"
Write-Host "3) Wave Simulation"
Write-Host ""
$choice = Read-Host "Choose (1-3)"

switch ($choice) {
    "1" {
        $exe = "blackhole"
        $title = "Blackhole Simulation"
    }
    "2" {
        $exe = "fractal"
        $title = "Fractal Zooms"
        Write-Host "`nControls:" -ForegroundColor Yellow
        Write-Host "WASD - Pan Around"
        Write-Host "Q/E - Zoom In/Zoom Out"
        Write-Host "SPACE - Toggle Mandelbrot-Julia"
        Write-Host "Mousewheel - Zoom In/Zoom Out"
        Write-Host "Left Click - Jump to location"
        Write-Host "R - Reset position"
        Write-Host "ESC - Exit`n"
    }
    "3" {
        $exe = "waves"
        $title = "Waves"
        Write-Host "`nControls:" -ForegroundColor Yellow
        Write-Host "Click & Drag - Create ripples"
        Write-Host "SPACE - Random splash"
        Write-Host "R - Clear waves"
        Write-Host "ESC - Exit`n"
    }
    default {
        Write-Host "Invalid choice!" -ForegroundColor Red
        exit 1
    }
}

Write-Host "`nBuilding $title..." -ForegroundColor Cyan

$root = $PSScriptRoot
$buildDir = Join-Path $root "build"

if (!(Test-Path $buildDir)) {
    Write-Host "Creating root build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

Set-Location $buildDir

Write-Host "Running CMake..." -ForegroundColor Yellow
& cmake .. 
if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake failed!"
    exit 1
}

Write-Host "Compiling..." -ForegroundColor Yellow
& cmake --build . --config Release
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed!"
    exit 1
}

Write-Host "`nLaunching $title..." -ForegroundColor Cyan
switch ($exe) {
    "blackhole" { Start-Process "$buildDir\blackhole\Release\blackhole.exe" }
    "fractal"   { Start-Process "$buildDir\fractal-zoom\Release\fractal.exe" }
    "waves"     { Start-Process "$buildDir\waves\Release\waves.exe" }
}
