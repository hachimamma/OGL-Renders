# windows gives me the trauma of my life.

Write-Host "=== Setting up OGL-Renders (Windows) ===" -ForegroundColor Cyan

$vcpkgPath = Join-Path $PSScriptRoot "vcpkg"
if (!(Test-Path $vcpkgPath)) {
    Write-Host "Cloning vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git $vcpkgPath
    Set-Location $vcpkgPath
    .\bootstrap-vcpkg.bat
} else {
    Write-Host "vcpkg already exists. Updating..."
    Set-Location $vcpkgPath
    git pull
}

Write-Host "Installing OpenGL dependencies with vcpkg..."
.\vcpkg.exe install glew glfw3 glm --triplet x64-windows

Set-Location $PSScriptRoot
Write-Host "Running build..."
bash build.sh

Write-Host "Setup complete! You can now run your executable from the build directory."
