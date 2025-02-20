# MInes SHell (MISH)

## Overview
MISH is a simple command-line interpreter (shell) implemented in C++. It supports basic shell functionalities such as command execution, input/output redirection, pipes, background execution, and built-in commands like `cd`, `exit`, and environment variable assignment. MISH can operate in both interactive and non-interactive (script) modes.

## Features

### 1. Command Execution
- Executes external commands by creating a child process using `fork()` and `execvp()`.
- Searches for executables in directories specified by the `PATH` environment variable.
- Supports absolute and relative paths for commands (e.g., `/bin/ls` or `./my_program`).

### 2. Built-in Commands
- `exit`: Exits the shell. No arguments are allowed.
- `cd <directory>`: Changes the current working directory. Exactly one argument is required.
- `<var>=<value>`: Assigns a value to an environment variable. If no value is provided, the variable is unset.

### 3. Input/Output Redirection
- `>`: Redirects standard output to a file (overwrites the file).
- `>>`: Redirects standard output to a file (appends to the file).
- `<`: Redirects standard input from a file.

### 4. Pipes
- Supports piping the output of one command to the input of another using the `|` operator.
- Example: `cmd1 | cmd2 | cmd3`

### 5. Background Execution
- Commands can be executed in the background using the `&` operator.
- Example: `cmd1 & cmd2 &`

### 6. Command Sequencing
- Commands can be executed sequentially using the `;` operator.
- Example: `cmd1 ; cmd2 ; cmd3`

### 7. Interactive and Non-Interactive Modes
- **Interactive Mode**: MISH prompts the user for input and executes commands one by one.
- **Non-Interactive Mode**: MISH reads commands from a script file and executes them sequentially.

### 8. Environment Variables
- MISH inherits the `PATH` environment variable from the parent process.
- Users can modify the `PATH` variable or create new environment variables using the `<var>=<value>` syntax.

### 9. Error Handling
- Provides informative error messages for invalid commands, syntax errors, and system errors.
- Errors are printed to `stderr`.

## How to Run MISH

### Prerequisites
- A Unix-like operating system (Linux, macOS, etc.).
- A C++ compiler (e.g., `g++`).

### Compilation
To compile MISH, navigate to the project directory and run:

```bash
make
```
This will create an executable named `mish`.

### Running MISH

#### Interactive Mode
To run MISH in interactive mode, simply execute the compiled binary:

```bash
./mish
```
You will see a prompt like `mish>` or `mish:/current/directory>` (if the `-p` flag is used). Type commands directly into the shell.

#### Non-Interactive Mode (Script Mode)
To run MISH in non-interactive mode, provide a script file as an argument:

```bash
./mish script.sh
```
MISH will execute the commands in `script.sh` sequentially without displaying a prompt.

#### Showing the Current Directory in the Prompt
You can enable the display of the current working directory in the prompt by using the `-p` flag:

```bash
./mish -p
```

## Special Features

### 1. Custom Prompt with Current Directory
- When the `-p` flag is used, MISH displays the current working directory in the prompt (e.g., `mish:/home/user>`).

### 2. Robust Tokenization
- MISH handles complex input with quotes and escape characters. Example:

```bash
echo "Hello, World!" > output.txt
```
- Supports multi-line commands and nested quotes.

### 3. Error Recovery
- MISH continues running after most errors, allowing users to correct mistakes without restarting the shell.

### 4. Background Execution with Process IDs
- When a command is executed in the background, MISH prints the process ID for reference:

```bash
[1234] cmd1 &
```

## Example Usage

### Interactive Mode
```bash
$ ./mish
mish> ls -la
mish> cd /tmp
mish:/tmp> echo "Hello" > output.txt
mish:/tmp> cat output.txt
Hello
mish:/tmp> exit
```

### Non-Interactive Mode
```bash
$ cat script.sh
ls -la
cd /tmp
echo "Hello" > output.txt
cat output.txt
exit

$ ./mish script.sh
```

### Pipes and Redirection
```bash
mish> ls -la | grep "txt" > files.txt
mish> sort < input.txt > sorted.txt
```

### Background Execution
```bash
mish> sleep 10 &
[1234] sleep 10 &
mish> # Shell is immediately available for new commands
```

### Error Handling Examples

#### Invalid Command
```bash
mish> invalid_cmd
Error: Failed to execute command: invalid_cmd
```

#### Invalid Syntax
```bash
mish> ls > output.txt | grep "txt"
Error: Cannot combine output redirection with pipe output
```

#### Invalid `cd` Usage
```bash
mish> cd
Error: cd command requires exactly one argument
```

## Limitations
- MISH does not support advanced shell features like wildcards (`*`), command substitution, or scripting constructs (e.g., loops, conditionals).
- Error messages could be more detailed in some cases.

