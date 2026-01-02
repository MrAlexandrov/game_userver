# Game Userver Development Setup

This document provides a comprehensive guide to setting up your development environment for the game_userver project.

## Overview

The game_userver project is a C++ service built using the [userver framework](https://github.com/userver-framework/userver). This guide covers everything you need to know to set up your development environment, including compilation, autocompletion, and running the service.

## Prerequisites

Before setting up the project, ensure you have the following installed on your Ubuntu 22.04 system:

```bash
sudo apt update
sudo apt install -y build-essential cmake git clang libclang-dev llvm pkg-config \
    libboost-all-dev libssl-dev libpqxx-dev postgresql-client \
    libyaml-cpp-dev libicu-dev libcurl4-openssl-dev libprotobuf-dev \
    protobuf-compiler-grpc libgrpc++-dev libudns-dev
```

## Quick Start

For the fastest setup, use the provided setup script:

```bash
./scripts/setup_dev_env.sh
```

This script will:
- Install required dependencies (on Ubuntu/Debian)
- Initialize submodules
- Download the userver framework if needed
- Set up the build environment
- Generate `compile_commands.json` for autocompletion

## Compilation Setup

### Using Makefile (Recommended)

The project includes a Makefile wrapper around CMake for convenience:

```bash
# Generate build files
make cmake-debug

# Build the project
make build-debug

# Run tests
make test-debug
```

### Using Helper Scripts

Alternative helper scripts are provided for simpler commands:

```bash
# Build in debug mode
./scripts/build.sh debug

# Build in release mode
./scripts/build.sh release
```

### Using CMake Directly

For more control, you can use CMake directly:

```bash
# Debug build
mkdir -p build_debug
cd build_debug
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
make -j$(nproc)

# Release build
mkdir -p build_release
cd build_release
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
make -j$(nproc)
```

## Userver Framework

The project depends on the userver framework, which is automatically downloaded by the setup scripts. If you need to manually download it:

```bash
mkdir -p third_party
cd third_party
git clone --depth 1 https://github.com/userver-framework/userver.git
cd ..
```

## Autocompletion Setup

### Prerequisites

1. Install clangd:
   ```bash
   # Ubuntu/Debian
   sudo apt install clangd
   
   # macOS with Homebrew
   brew install llvm
   ```

2. Generate `compile_commands.json`:
   ```bash
   ./scripts/setup_dev_env.sh
   # or
   make cmake-debug
   ```

### Editor Configuration

#### VS Code

1. Install the "clangd" extension by LLVM
2. The extension will automatically use the project's `.clangd` configuration

#### Vim/Neovim

For coc.nvim:
```vim
:CocInstall coc-clangd
```

For native LSP (Neovim 0.5+):
```lua
require'lspconfig'.clangd.setup{
  cmd = {"clangd", "--background-index"},
  root_dir = require('lspconfig/util').root_pattern(
    '.clangd',
    'compile_commands.json',
    '.git'
  ),
}
```

#### Other Editors

Most editors with LSP support can use clangd. Configure your editor to:
1. Use clangd as the C++ language server
2. Find the `compile_commands.json` file in the build directory

## Project Structure

```
game_userver/
├── src/                 # Source code
├── proto/               # Protocol buffer definitions
├── configs/             # Configuration files
├── postgresql/          # Database schema and initialization
├── tests/               # Test files
├── cmake/               # CMake modules
├── third_party/         # Third-party dependencies (including userver)
├── scripts/             # Helper scripts
├── docs/                # Documentation
├── build_debug/         # Debug build directory (created during build)
├── build_release/       # Release build directory (created during build)
├── compile_commands.json # Symlink to build directory (created by setup script)
├── CMakeLists.txt       # Main CMake file
├── Makefile             # Makefile wrapper
├── .clangd              # Clangd configuration
└── README.md            # Project README
```

## Database Setup

The project uses PostgreSQL. For development, you can use Docker:

```bash
docker-compose up -d postgres
```

Or set up a local PostgreSQL instance:
```bash
sudo apt install postgresql postgresql-contrib
sudo -u postgres createdb game_userver_db_1
sudo -u postgres createuser -s $USER
psql -d game_userver_db_1 -f postgresql/schemas/db_1.sql
```

## Running the Service

### Using Docker (Recommended)

```bash
# Start both the service and database
docker-compose up -d

# View logs
docker-compose logs -f
```

### Locally

After building:
```bash
# Start the service
./build_debug/game_userver --config configs/static_config.yaml --config_vars configs/config_vars.yaml
```

## Testing

### Unit Tests

```bash
make test-debug
```

### Functional Tests

```bash
# Start the service in testsuite mode
make start-debug

# In another terminal, run functional tests
cd tests
python3 -m pytest basic/test_basic.py -v
```

## Development Workflow

1. Make changes to source code
2. Rebuild the project:
   ```bash
   make build-debug
   ```
3. Run tests to verify changes:
   ```bash
   make test-debug
   ```
4. For faster iteration:
   ```bash
   make build-debug && make test-debug
   ```

## Troubleshooting

### Common Issues

1. **Missing dependencies**: Make sure all prerequisites are installed
2. **Database connection errors**: Ensure PostgreSQL is running and the database is initialized
3. **userver download issues**: If automatic download fails, manually clone the userver repository to `third_party/userver`
4. **Autocompletion not working**: Verify that `compile_commands.json` exists and that clangd is properly installed in your editor

### Clean Build

If you encounter build issues:
```bash
make dist-clean
./scripts/setup_dev_env.sh
```

If you need to re-download userver:
```bash
rm -rf third_party/userver
./scripts/setup_dev_env.sh
```

## Additional Documentation

- [Setup Guide](setup_guide.md) - Detailed setup instructions
- [Editor Setup](editor_setup.md) - Editor-specific configuration
- [Userver Framework Documentation](https://userver.tech/) - Official userver documentation

## Contributing

Feel free to modify and extend this project. When making changes:

1. Follow the existing code style
2. Add appropriate tests for new functionality
3. Update documentation as needed
4. Run all tests before submitting changes