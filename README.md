# MISH (MInes SHell)

## Overview
MISH is a custom command-line interpreter (shell) implementation that provides core shell functionality similar to bash but with a simplified feature set. The shell supports both interactive and script modes, offering a robust command execution environment with various shell operations.

## Features

### Basic Shell Operations
- Interactive command-line interface with directory-aware prompt
- Script execution mode for batch processing
- Command history support
- Environment variable management
- PATH-based executable resolution

### Command Execution
- Execution of system commands and executables
- Support for command arguments and parameters
- Built-in command implementation
- Background process execution

### Built-in Commands
- `exit`: Terminates the shell
- `cd`: Changes current working directory
- Environment variable assignment (e.g., `PATH=`)

### I/O Features
- Input redirection (`<`)
- Output redirection (`>`)
- Output append redirection (`>>`)
- Pipeline support (`|`)
- Background execution (`&`)
- Command sequencing (`;`)

### Advanced Features
- Quote handling for string arguments
- Error handling and reporting
- Multi-command pipeline support
- Background process management
- Environment variable manipulation

## Usage
- Run the Makefile (make)
