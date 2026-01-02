#!/bin/bash

# Simple build script for game_userver project

set -e  # Exit on any error

BUILD_TYPE=${1:-debug}  # Default to debug build

case $BUILD_TYPE in
    debug|Debug)
        BUILD_DIR="build_debug"
        CMAKE_BUILD_TYPE="Debug"
        ;;
    release|Release)
        BUILD_DIR="build_release"
        CMAKE_BUILD_TYPE="Release"
        ;;
    *)
        echo "Usage: $0 [debug|release]"
        exit 1
        ;;
esac

echo "Building project in $BUILD_TYPE mode..."

# Check if userver exists, if not try to download it
if [ ! -d "third_party/userver" ]; then
    echo "userver framework not found, attempting to download..."
    mkdir -p third_party
    cd third_party
    git clone --depth 1 https://github.com/userver-framework/userver.git
    cd ..
fi

# Create build directory if it doesn't exist
mkdir -p $BUILD_DIR

# Generate build files if needed
if [ ! -f $BUILD_DIR/CMakeCache.txt ]; then
    echo "Generating CMake build files..."
    cd $BUILD_DIR
    cmake .. -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cd ..
else
    echo "CMake build files already exist."
fi

# Build the project
echo "Building project..."
cmake --build $BUILD_DIR -j$(nproc)

echo "Build complete! Binary is located at $BUILD_DIR/game_userver"