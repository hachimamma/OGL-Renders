# this will auto install dependencies and build

#!/usr/bin/env bash
set -e

echo "=== Setting up OGL-Renders ==="

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  if command -v pacman &>/dev/null; then
    OS="arch"
  elif command -v apt-get &>/dev/null; then
    OS="debian"
  else
    echo "Unsupported Linux distro. Please install dependencies manually."
    exit 1
  fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
  OS="macos"
else
  echo "Unsupported OS."
  exit 1
fi

echo "=== Installing dependencies for $OS ==="

if [[ "$OS" == "arch" ]]; then
  sudo pacman -Syu --needed --noconfirm cmake gcc glfw glew glm libx11 libxi libxrandr libxcursor libxinerama
elif [[ "$OS" == "debian" ]]; then
  sudo apt-get update
  sudo apt-get install -y cmake g++ gcc libglfw3-dev libglew-dev libglm-dev \
    libx11-dev libxi-dev libxxf86vm-dev libxrandr-dev libxinerama-dev libxcursor-dev
elif [[ "$OS" == "macos" ]]; then
  if ! command -v brew &>/dev/null; then
    echo "Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
  fi
  brew install cmake glfw glew glm
fi

echo""
echo "Dependencies installed! Building project"
echo""

echo "=== Building Project ==="
chmod +x build.sh
./build.sh
