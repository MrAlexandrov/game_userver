#!/bin/bash

# Script to set up the development environment for game_userver project

set -e  # Exit on any error

echo "Setting up development environment for game_userver..."

# Check if we're on Ubuntu 22.04
if ! grep -q "Ubuntu 22.04" /etc/os-release 2>/dev/null; then
    echo "Warning: This script is designed for Ubuntu 22.04. You may need to adjust package names for your distribution."
fi

# Install dependencies if on Ubuntu/Debian
if command -v apt &> /dev/null; then
    echo "Installing dependencies..."
    sudo apt update
    sudo apt install -y build-essential cmake git clang libclang-dev llvm pkg-config \
        libboost-all-dev libssl-dev libpqxx-dev postgresql-client \
        libyaml-cpp-dev libicu-dev libcurl4-openssl-dev libprotobuf-dev \
        protobuf-compiler-grpc libgrpc++-dev libudns-dev
fi

# Initialize submodules
echo "Initializing submodules..."
git submodule update --init --recursive

# If third_party/userver doesn't exist, try to download it
if [ ! -d "third_party/userver" ]; then
    echo "Downloading userver framework..."
    mkdir -p third_party
    cd third_party
    git clone --depth 1 https://github.com/userver-framework/userver.git
    cd ..
else
    echo "userver framework already exists in third_party/userver"
fi

# Clean any existing build directories to avoid conflicts
echo "Cleaning existing build directories..."
rm -rf build_debug build_release

# Create build directory and generate compile_commands.json
echo "Setting up build environment..."
mkdir -p build_debug
cd build_debug
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cd ..

# Create a symlink to compile_commands.json in the project root for editor tools
if [ ! -f compile_commands.json ]; then
    echo "Creating symlink for compile_commands.json..."
    ln -s build_debug/compile_commands.json compile_commands.json
fi

echo "Development environment setup complete!"
echo ""
echo "To build the project, run:"
echo "  make build-debug"
echo ""
echo "To run tests, run:"
echo "  make test-debug"
echo ""
echo "For autocompletion to work properly, ensure your editor is configured to use clangd."