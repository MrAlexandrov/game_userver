# Game Userver Project Setup Guide

This guide explains how to set up the game_userver project for compilation and development on Ubuntu 22.04.

## Prerequisites

Before setting up the project, ensure you have the following installed on your Ubuntu 22.04 system:

```bash
sudo apt update
sudo apt install -y build-essential cmake git clang libclang-dev llvm pkg-config \
    libboost-all-dev libssl-dev libpqxx-dev postgresql-client \
    libyaml-cpp-dev libicu-dev libcurl4-openssl-dev libprotobuf-dev \
    protobuf-compiler-grpc libgrpc++-dev libudns-dev
```

## Install userver Framework

The project depends on the userver framework. You can either:

### Option 1: Install userver system-wide (Recommended)

Follow the instructions at [userver installation guide](https://userver.tech/d7/d7b/md_en_2userver_2install.html)

### Option 2: Use userver from third_party

If you prefer to use userver from the third_party directory:
```bash
mkdir -p third_party
cd third_party
git clone --depth 1 https://github.com/userver-framework/userver.git
cd ..
```

## Project Structure

The project follows a standard userver framework structure:
- `src/` - Source code files
- `proto/` - Protocol buffer definitions
- `configs/` - Configuration files
- `postgresql/` - Database schema and initialization scripts
- `tests/` - Test files
- `cmake/` - CMake modules and utilities
- `third_party/` - Third-party dependencies (including userver framework)

## Compilation Setup

### 1. Clone the Repository

```bash
git clone <repository-url>
cd game_userver
```

### 2. Initialize Submodules

```bash
git submodule update --init --recursive
```

### 3. Build the Project

The project uses CMake as its build system with a Makefile wrapper for convenience.

#### Using Makefile (Recommended)

For debug build:
```bash
make cmake-debug
make build-debug
```

For release build:
```bash
make cmake-release
make build-release
```

#### Using Helper Scripts

```bash
# Build in debug mode
./scripts/build.sh debug

# Build in release mode
./scripts/build.sh release
```

#### Using CMake Directly

For debug build:
```bash
mkdir build_debug
cd build_debug
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
make -j$(nproc)
```

For release build:
```bash
mkdir build_release
cd build_release
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
make -j$(nproc)
```

### 4. Database Setup

The project uses PostgreSQL. You can either:

#### Option A: Use Docker (Recommended for development)
```bash
docker-compose up -d postgres
```

#### Option B: Use local PostgreSQL
1. Install PostgreSQL:
   ```bash
   sudo apt install postgresql postgresql-contrib
   ```

2. Create database and user:
   ```bash
   sudo -u postgres createdb game_userver_db_1
   sudo -u postgres createuser -s $USER
   ```

3. Initialize the schema:
   ```bash
   psql -d game_userver_db_1 -f postgresql/schemas/db_1.sql
   ```

## Autocompletion Setup

The project is configured to work with clangd for autocompletion in editors like VS Code, Vim, and others.

### Generate compile_commands.json

The `compile_commands.json` file is essential for clangd to provide accurate autocompletion. It's automatically generated when you build with CMake:

```bash
make cmake-debug
```

Or with the helper script:
```bash
./scripts/setup_dev_env.sh
```

### Configure Your Editor

#### VS Code

1. Install the "clangd" extension from the marketplace
2. Ensure the `.clangd` configuration file is in your project root (already provided)
3. The extension will automatically use the `compile_commands.json` file

#### Vim/Neovim with coc-clangd

1. Install coc-clangd:
   ```vim
   :CocInstall coc-clangd
   ```

2. The plugin will automatically detect and use the `compile_commands.json` file

#### Other Editors

Most editors with LSP support can use clangd. Make sure to:
1. Install clangd on your system
2. Configure your editor to use clangd as the C++ language server
3. Ensure the editor can find the `compile_commands.json` file

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
4. For faster iteration, you can also use:
   ```bash
   make build-debug && make test-debug
   ```

## Troubleshooting

### Common Issues

1. **Missing dependencies**: Make sure all prerequisites are installed
2. **Database connection errors**: Ensure PostgreSQL is running and the database is initialized
3. **userver not found**: Install userver system-wide or place it in `third_party/userver`
4. **Autocompletion not working**: Verify that `compile_commands.json` exists in the build directory and that clangd is properly installed in your editor
5. **CMake errors about unknown commands**: Clean your build directories and run the setup script again:
   ```bash
   rm -rf build_debug build_release
   ./scripts/setup_dev_env.sh
   ```

### Clean Build

If you encounter build issues, try cleaning the build directories:
```bash
make dist-clean
```

This will remove all build artifacts and allow you to start fresh.

If you need to re-download userver:
```bash
rm -rf third_party/userver
# Then either install userver system-wide or download it to third_party/userver