# Setup dependencies for OGL-Renders (Windows)

Write-Host "=== Installing Dependencies for OGL-Renders ===" -ForegroundColor Cyan

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "CMake not found. Installing via winget..." -ForegroundColor Yellow
    winget install -e --id Kitware.CMake
} else {
    Write-Host "CMake already installed." -ForegroundColor Green
}

if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    Write-Host "Git not found. Installing via winget..." -ForegroundColor Yellow
    winget install -e --id Git.Git
} else {
    Write-Host "Git already installed." -ForegroundColor Green
}

$vcpkgPath = Join-Path $PSScriptRoot "vcpkg"
if (!(Test-Path $vcpkgPath)) {
    Write-Host "Cloning vcpkg..." -ForegroundColor Yellow
    git clone https://github.com/microsoft/vcpkg.git $vcpkgPath
    & "$vcpkgPath\bootstrap-vcpkg.bat"
} else {
    Write-Host "vcpkg already exists." -ForegroundColor Green
}

Write-Host "`nDependencies installed successfully!" -ForegroundColor Cyan
Write-Host "You can now run ./build.ps1 to build your project."
