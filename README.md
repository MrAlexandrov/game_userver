# game_userver

Template of a C++ service that uses [userver framework](https://github.com/userver-framework/userver).

## Download and Build

To create your own userver-based service follow the following steps:

1. Press the "Use this template button" at the top right of this GitHub page
2. Clone the service `git clone your-service-repo && cd your-service-repo`
3. Give a proper name to your service and replace all the occurrences of "game_userver" string with that name
4. Feel free to tweak, adjust or fully rewrite the source code of your service.

## Development Setup

For easier development setup on Ubuntu 22.04, you can use the provided setup script:

```bash
./scripts/setup_dev_env.sh
```

This script will:
- Install required dependencies
- Initialize submodules
- Download the userver framework if needed
- Set up the build environment
- Generate compile_commands.json for autocompletion

## Makefile

`PRESET` is either `debug`, `release`, or if you've added custom presets in `CMakeUserPresets.json`, it
can also be `debug-custom`, `release-custom`.

* `make cmake-PRESET` - run cmake configure, update cmake options and source file lists
* `make build-PRESET` - build the service
* `make test-PRESET` - build the service and run all tests
* `make start-PRESET` - build the service, start it in testsuite environment and leave it running
* `make install-PRESET` - build the service and install it in directory set in environment `PREFIX`
* `make` or `make all` - build and run all tests in `debug` and `release` modes
* `make format` - reformat all C++ and Python sources
* `make dist-clean` - clean build files and cmake cache
* `make docker-COMMAND` - run `make COMMAND` in docker environment
* `make docker-clean-data` - stop docker containers

## Building with Helper Scripts

You can also use the helper scripts for building:

```bash
# Build in debug mode
./scripts/build.sh debug

# Build in release mode
./scripts/build.sh release
```

## Userver Framework

This project depends on the [userver framework](https://github.com/userver-framework/userver). The framework is automatically downloaded by the setup scripts, but you can also manually download it:

```bash
mkdir -p third_party
cd third_party
git clone --depth 1 https://github.com/userver-framework/userver.git
cd ..
```

## Autocompletion Setup

For the best development experience with autocompletion:

1. Run the setup script or ensure `compile_commands.json` is generated in the build directory
2. Install clangd extension in your editor
3. The `.clangd` configuration file is already provided for optimal settings

## Documentation

For detailed setup instructions, see:
- [Development Setup Guide](docs/development_setup.md) - Comprehensive setup guide
- [Setup Guide](docs/setup_guide.md) - Detailed instructions for compilation and running the service
- [Editor Setup](docs/editor_setup.md) - Instructions for configuring autocompletion in various editors

## License

The original template is distributed under the [Apache-2.0 License](https://github.com/userver-framework/userver/blob/develop/LICENSE)
and [CLA](https://github.com/userver-framework/userver/blob/develop/CONTRIBUTING.md). Services based on the template may change
the license and CLA.
