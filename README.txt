MInes SHell (MISH)
==================

Overview
--------
MISH is a simple command-line interpreter (shell) implemented in C++. It supports basic shell functionalities such as command execution, input/output redirection, pipes, background execution, and built-in commands like `cd`, `exit`, and environment variable assignment. MISH can operate in both interactive and non-interactive (script) modes.

Features
--------
1. **Command Execution**
   - Executes external commands by creating a child process using `fork()` and `execvp()`.
   - Searches for executables in directories specified by the `PATH` environment variable.
   - Supports absolute and relative paths for commands (e.g., `/bin/ls` or `./my_program`).

2. **Built-in Commands**
   - `exit`: Exits the shell. No arguments are allowed.
   - `cd <directory>`: Changes the current working directory. Exactly one argument is required.
   - `<var>=<value>`: Assigns a value to an environment variable. If no value is provided, the variable is unset.

3. **Input/Output Redirection**
   - `>`: Redirects standard output to a file (overwrites the file).
   - `>>`: Redirects standard output to a file (appends to the file).
   - `<`: Redirects standard input from a file.

4. **Pipes**
   - Supports piping the output of one command to the input of another using the `|` operator.
   - Example: `cmd1 | cmd2 | cmd3`.

5. **Background Execution**
   - Commands can be executed in the background using the `&` operator.
   - Example: `cmd1 & cmd2 & cmd3`.

6. **Command Sequencing**
   - Commands can be executed sequentially using the `;` operator.
   - Example: `cmd1 ; cmd2 ; cmd3`.

7. **Interactive and Non-Interactive Modes**
   - **Interactive Mode**: MISH prompts the user for input and executes commands one by one.
   - **Non-Interactive Mode**: MISH reads commands from a script file and executes them sequentially.

8. **Environment Variables**
   - MISH inherits the `PATH` environment variable from the parent process.
   - Users can modify the `PATH` variable or create new environment variables using the `<var>=<value>` syntax.

9. **Error Handling**
   - Provides informative error messages for invalid commands, syntax errors, and system errors.
   - Errors are printed to `stderr`.

How to Run MISH
---------------
### Prerequisites
- A Unix-like operating system (Linux was used to develop and test this application).
- A C++ compiler (`g++` preferably. If you'd like to use `gcc`, there's commented-out code for this in the `makefile`).

### Compilation
To compile MISH, navigate to the project directory and run:
make
This will create an executable named `mish`.

### Running MISH
#### Interactive Mode
To run MISH in interactive mode, simply execute the compiled binary:
./mish
You will see a prompt like `mish>` or `mish:/current/directory>` (if the `-p` flag is used). Type commands directly into the shell.

#### Non-Interactive Mode (Script Mode)
To run MISH in non-interactive mode, provide a script file as an argument:
./mish script.sh
MISH will execute the commands in `script.sh` sequentially without displaying a prompt.

#### Showing the Current Directory in the Prompt
You can enable the display of the current working directory in the prompt by using the `-p` flag:
./mish -p

Special Features
---------------
1. **Custom Prompt with Current Directory**
   - When the `-p` flag is used, MISH displays the current working directory in the prompt (e.g., `mish:/home/user>`).

2. **Robust Tokenization**
   - MISH handles complex input with quotes and escape characters. Example:
     ```
     echo "Hello, World!" > output.txt
     ```
   - Supports multi-line commands and nested quotes.

3. **Error Recovery**
   - MISH continues running after most errors, allowing users to correct mistakes without restarting the shell.

4. **Background Execution with Process IDs**
   - When a command is executed in the background, MISH prints the process ID for reference:
     ```
     [1234] cmd1 &
     ```

Limitations
-----------
- MISH does not support advanced shell features like wildcards (`*`), command substitution, or scripting constructs (e.g., loops, conditionals).
- Error messages aren't the most detailed (Could've been more detailed in some cases, but it's a small project :)).