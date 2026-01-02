# Editor Setup for Autocompletion

This guide explains how to set up your editor for optimal autocompletion and development experience with the game_userver project.

## Prerequisites

Before setting up your editor, ensure you have:

1. Built the project at least once to generate `compile_commands.json`:
   ```bash
   ./scripts/setup_dev_env.sh
   # or
   make cmake-debug
   ```

2. Installed clangd on your system:
   ```bash
   # Ubuntu/Debian
   sudo apt install clangd
   
   # macOS with Homebrew
   brew install llvm
   ```

## VS Code Setup

### Required Extensions

1. Install the "clangd" extension by LLVM:
   - Open VS Code
   - Go to Extensions (Ctrl+Shift+X)
   - Search for "clangd" and install the extension by LLVM

### Configuration

The project already includes a `.clangd` configuration file with optimal settings. The clangd extension will automatically use this configuration.

If you need to adjust any settings, you can modify the `.clangd` file in the project root.

## Vim/Neovim Setup

### With coc.nvim

1. Install coc-clangd:
   ```vim
   :CocInstall coc-clangd
   ```

2. The plugin will automatically detect and use the `compile_commands.json` file.

### With native LSP (Neovim 0.5+)

Add this to your Neovim configuration:

```lua
require'lspconfig'.clangd.setup{
  cmd = {"clangd", "--background-index"},
  on_attach = function(client, bufnr)
    -- Your on_attach function here
  end,
  root_dir = require('lspconfig/util').root_pattern(
    '.clangd',
    'compile_commands.json',
    '.git'
  ),
}
```

## Other Editors

Most editors with LSP support can use clangd. The general steps are:

1. Install clangd on your system
2. Install the appropriate LSP plugin for your editor
3. Configure the plugin to use clangd as the C++ language server
4. Ensure the plugin can find the `compile_commands.json` file

## Troubleshooting Autocompletion

### Common Issues

1. **No autocompletion at all**:
   - Verify that `compile_commands.json` exists in the build directory
   - Check that clangd is installed and accessible in your PATH
   - Restart your editor after generating `compile_commands.json`

2. **Incomplete autocompletion**:
   - Ensure the `.clangd` configuration file is in your project root
   - Check that all necessary include paths are specified in `.clangd`
   - Verify that the build was successful and `compile_commands.json` is up to date

3. **Slow autocompletion**:
   - The first time you open a project, clangd needs to index all files
   - This is a one-time operation and will be faster on subsequent openings
   - You can monitor indexing progress in the clangd logs

### Checking clangd Logs

Most editors provide a way to view clangd logs:

- **VS Code**: Open the Output panel (View → Output) and select "clangd" from the dropdown
- **Vim/Neovim**: Check the coc.nvim logs or LSP logs depending on your setup

Look for any error messages that might indicate configuration issues.

## Project-Specific Configuration

The project's `.clangd` file includes:

1. C++20 standard specification
2. Include paths for the project and userver framework
3. Clang-Tidy checks for code quality
4. Background indexing for better performance

You can modify this file to add additional configuration options as needed.

## Updating Autocompletion After Code Changes

Whenever you add new files or change include paths:

1. Regenerate the build files:
   ```bash
   make cmake-debug
   # or
   ./scripts/build.sh debug
   ```

2. Restart clangd in your editor (most editors have a "Restart LSP" command)

This ensures that clangd has the latest information about your project structure.