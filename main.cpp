#include "mish.h"
#include <cstdlib> // For environ
using namespace std;

bool showPath = false;
extern char **environ;

// Function to split input string into tokens while handling quotes and escape characters
vector<string> tokenize(const string &input)
{
    vector<string> tokens;  // Store resulting tokens
    string current_token;   // Current token being build
    bool in_quotes = false; // Track if we're inside quotes
    bool escaped = false;   // Track if the next character is escaped

    for (size_t i = 0; i < input.length(); i++)
    {
        char c = input[i];

        if (escaped)
        {
            current_token += c;
            escaped = false;
            continue;
        }

        if (c == '\\')
        {
            escaped = true;
            continue;
        }

        if (c == '"' && !escaped)
        {
            in_quotes = !in_quotes;
            continue;
        }

        // Handle special characters when not in quotes
        if (!in_quotes && (c == '|' || c == '>' || c == '<' || c == '&' || c == ';'))
        {
            if (!current_token.empty())
            {
                tokens.push_back(current_token);
                current_token.clear();
            }
            // Handle ">>" as a single token
            if (c == '>' && i + 1 < input.length() && input[i + 1] == '>')
            {
                tokens.push_back(">>");
                i++;
            }
            else
            {
                tokens.push_back(string(1, c));
            }
            continue;
        }

        // Handle whitespace
        if (!in_quotes && (c == ' ' || c == '\t'))
        {
            if (!current_token.empty())
            {
                tokens.push_back(current_token);
                current_token.clear();
            }
            continue;
        }

        current_token += c;
    }

    if (!current_token.empty())
    {
        tokens.push_back(current_token);
    }

    if (in_quotes)
    {
        throw ShellError("Unterminated quote"); // Throw error
    }

    return tokens;
}

// Function to validate parsed command structure
bool validateCommand(const Command &cmd)
{
    // Check if output redirection and pipe start are both set
    if (cmd.redirectOutputToFile && cmd.isPipeStart)
    {
        handleError("Cannot combine output redirection with pipe output");
        return false;
    }
    // Check if input redirection and pipe end are both set
    if (cmd.redirectedInputFromFile && cmd.isPipeEnd)
    {
        handleError("Cannot combine input redirection with pipe input");
        return false;
    }

    // Check for invalid redirection combinations
    if (cmd.redirectOutputToFile && cmd.redirectOutputFileName.empty())
    {
        handleError("Missing output file name");
        return false;
    }
    if (cmd.redirectedInputFromFile && cmd.redirectedInputFileName.empty())
    {
        handleError("Missing input file name");
        return false;
    }

    // Check if the command has no tokens
    if (cmd.tokens.empty())
    {
        handleError("Empty command");
        return false;
    }
    return true;
}

// Function to parse tokens into commands with support for pipes, redirections, and background execution
vector<Command> parseTokens(const vector<string> &tokens)
{
    vector<Command> commands; // Store resulting commands
    Command currentCommand;   // Current command being built

    for (size_t i = 0; i < tokens.size(); i++)
    {
        if (tokens[i] == "|") // Handle pipes
        {
            if (currentCommand.tokens.empty())
            {
                throw ShellError("Invalid pipe: empty command");
            }
            currentCommand.isPipeStart = true;
            commands.push_back(currentCommand);
            currentCommand = Command();
            currentCommand.isPipeEnd = true;
        }
        else if (tokens[i] == "&") // Handle background execution
        {
            if (!currentCommand.tokens.empty())
            {
                currentCommand.isBackground = true;
                commands.push_back(currentCommand);
                currentCommand = Command();
            }

            // Ensure all previous pipeline commands are marked as background
            for (auto &cmd : commands)
            {
                cmd.isBackground = true;
            }
        }
        else if (tokens[i] == ">" || tokens[i] == ">>") // Handle output redirection
        {
            if (i + 1 >= tokens.size())
            {
                throw ShellError("Missing filename for redirection");
            }
            currentCommand.redirectOutputToFile = true;
            currentCommand.appendOutput = (tokens[i] == ">>");
            currentCommand.redirectOutputFileName = tokens[++i];
        }
        else if (tokens[i] == "<") // Handle input redirection
        {
            if (i + 1 >= tokens.size())
            {
                throw ShellError("Missing filename for input redirection");
            }
            currentCommand.redirectedInputFromFile = true;
            currentCommand.redirectedInputFileName = tokens[++i];
        }
        else if (tokens[i] == ";") // Handle command sequencing
        {
            if (!currentCommand.tokens.empty())
            {
                commands.push_back(currentCommand);
                currentCommand = Command();
            }
        }
        else // Regular command tokens
        {
            currentCommand.tokens.push_back(tokens[i]);
        }
    }

    // For command left after loop
    if (!currentCommand.tokens.empty())
    {
        commands.push_back(currentCommand);
    }

    // Validate all commands
    for (const auto &cmd : commands)
    {
        if (!validateCommand(cmd))
        {
            throw ShellError("Invalid command structure");
        }
    }

    return commands;
}

// Function to check if a command is built-in (ex. cd, exit, variable assignment)
bool isBuiltInCommand(const string &cmd)
{
    return cmd == "cd" || cmd == "exit" || (cmd.find('=') != string::npos);
}

// Function to execute built-in commands like 'cd' and 'exit'
void executeBuiltIn(const Command &cmd)
{
    if (cmd.tokens.empty())
        return;

    string command = cmd.tokens[0];

    if (command == "exit") // Handle exit case
    {
        if (cmd.tokens.size() > 1)
        {
            throw ShellError("exit command takes no arguments");
        }
        exit(0);
    }
    else if (command == "cd") // Handle cd case
    {
        if (cmd.tokens.size() != 2)
        {
            throw ShellError("cd command requires exactly one argument");
        }
        if (chdir(cmd.tokens[1].c_str()) != 0) // Change working directory
        {
            throw ShellError("cd failed: " + string(strerror(errno)));
        }
    }
    else if (command.find('=') != string::npos) // Handle variable assignment
    {
        size_t pos = command.find('=');
        string var_name = command.substr(0, pos);
        string var_value = pos + 1 < command.length() ? command.substr(pos + 1) : "";

        if (var_name == "PATH")
        {
            setenv("PATH", var_value.c_str(), 1);
        }
        else
        {
            if (var_value.empty())
            {
                unsetenv(var_name.c_str());
            }
            else
            {
                setenv(var_name.c_str(), var_value.c_str(), 1);
            }
        }
    }
}

void setupRedirection(const Command &cmd)
{
    if (cmd.redirectedInputFromFile)
    {
        int fd = open(cmd.redirectedInputFileName.c_str(), O_RDONLY);
        if (fd == -1)
        {
            throw ShellError("Error opening input file: " + cmd.redirectedInputFileName);
        }
        if (dup2(fd, STDIN_FILENO) == -1)
        {
            close(fd);
            throw ShellError("Error setting up input redirection");
        }
        close(fd);
    }

    if (cmd.redirectOutputToFile)
    {
        // Set the flags for opening the output file
        int flags = O_WRONLY | O_CREAT;
        // Set append or truncate mode
        flags |= cmd.appendOutput ? O_APPEND : O_TRUNC;

        int fd = open(cmd.redirectOutputFileName.c_str(), flags, 0644);
        if (fd == -1)
        {
            throw ShellError("Error opening output file: " + cmd.redirectOutputFileName);
        }
        if (dup2(fd, STDOUT_FILENO) == -1)
        {
            close(fd);
            throw ShellError("Error setting up output redirection");
        }
        close(fd);
    }
}

// Function to execute a pipeline of commands
void executePipeline(vector<Command> &pipeline)
{
    int n = pipeline.size();
    vector<pid_t> pids(n);              // Store process IDs
    vector<array<int, 2>> pipes(n - 1); // Store pipe file descriptors

    // Create pipes
    for (int i = 0; i < n - 1; i++)
    {
        if (pipe(pipes[i].data()) == -1)
        {
            throw ShellError("Failed to create pipe");
        }
    }

    // Create processes
    for (int i = 0; i < n; i++)
    {
        pids[i] = fork();

        if (pids[i] == -1)
        {
            throw ShellError("Fork failed");
        }

        if (pids[i] == 0)
        { // Child process
            try
            {
                // Setup pipes
                // If this is not the first command, redirect input from the previous pipe
                if (i > 0)
                {
                    if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1)
                    {
                        throw ShellError("Failed to set up pipe input");
                    }
                    close(pipes[i - 1][0]);
                }
                // If this is not the last command, redirect output to the next pipe
                if (i < n - 1)
                {
                    if (dup2(pipes[i][1], STDOUT_FILENO) == -1)
                    {
                        throw ShellError("Failed to set up pipe output");
                    }
                    close(pipes[i][1]);
                }

                // Close all pipe fds
                for (auto &p : pipes)
                {
                    close(p[0]); // Rear end
                    close(p[1]); // Write end
                }

                // Handle redirections
                setupRedirection(pipeline[i]);

                // Make sure stdout is line buffered
                setvbuf(stdout, nullptr, _IOLBF, 0);

                // Convert tokens to char* array for execvp
                vector<char *> args;
                for (const auto &token : pipeline[i].tokens)
                {
                    args.push_back(const_cast<char *>(token.c_str())); // Convert to C string
                }
                args.push_back(nullptr);

                execvp(args[0], args.data());
                throw ShellError("Failed to execute command: " + string(args[0]));
            }
            catch (const ShellError &e)
            {
                cerr << "Error in child process: " << e.what() << endl;
                exit(1);
            }
        }
    }

    // Parent process
    // Close all pipe fds
    for (auto &p : pipes)
    {
        close(p[0]); // Read end
        close(p[1]); // Write end
    }

    // For background processes, only wait for the last process in the pipeline
    if (!pipeline.back().isBackground)
    {
        for (pid_t pid : pids)
        {
            int status;
            waitpid(pid, &status, 0); // Wait for each child process to finish

            // Check if the process exited with an error
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            {
                handleError("Command exited with status: " + to_string(WEXITSTATUS(status)));
            }
        }
    }
    else
    {
        for (pid_t pid : pids)
        {
            setpgid(pid, pid); // Set a new process group to detach from the shell
        }

        // For background processes, print the process ID without a newline
        cout << "[" << pids.back() << "] " << pipeline.back().tokens[0] << " &" << endl;
        cout.flush();

        // Buffer to store the current working directory
        char cwd[PATH_MAX];

        if (showPath && getcwd(cwd, sizeof(cwd)) != nullptr)
        {
            cout << "mish:" << cwd << "> " << flush;
        }
        else
        {
            cout << "mish> " << flush;
        }
    }
}

// Function to execute a sequence of commands, including handling built-in commands
void executeCommands(const vector<Command> &commands)
{
    vector<Command> current_pipeline; // Store current pipeline of commands
    bool last_was_background = false; // Track if the last command was a background command

    for (const auto &cmd : commands)
    {
        if (cmd.tokens.empty())
            continue;

        try
        {
            // Handle built-in commands
            if (isBuiltInCommand(cmd.tokens[0]))
            {
                executeBuiltIn(cmd);
                if (cmd.isBackground)
                {
                    cout << "[builtin] " << cmd.tokens[0] << " &" << endl;
                }
                last_was_background = cmd.isBackground;
                continue;
            }

            // Add command to pipeline
            current_pipeline.push_back(cmd);

            // Execute pipeline if this is the end of a pipeline or a standalone command
            if (!cmd.isPipeStart)
            {
                executePipeline(current_pipeline);
                last_was_background = cmd.isBackground;
                current_pipeline.clear();
            }
        }
        catch (const ShellError &e)
        {
            handleError(e.what());
            current_pipeline.clear();
        }
    }

    // After all commands are executed, if the last one was not a background command,
    // or if we're in interactive mode, print a newline to prepare for the next prompt
    if (!last_was_background)
    {
        cout << flush;
    }
}

// Function to run the shell in interactive mode
void interactiveMode()
{
    string input;
    // Buffer to store the current working directory
    char cwd[PATH_MAX];

    // Make sure stdout is line buffered
    setvbuf(stdout, nullptr, _IOLBF, 0);

    while (true)
    {
        // Force flush before printing prompt
        cout.flush();

        if (showPath && getcwd(cwd, sizeof(cwd)) != nullptr)
        {
            cout << "mish:" << cwd << "> " << flush;
        }
        else
        {
            cout << "mish> " << flush;
        }

        if (!getline(cin, input))
        {
            cout << endl;
            break;
        }

        try
        {
            if (input.empty())
                continue;

            vector<string> tokens = tokenize(input);
            if (tokens.empty())
                continue;

            vector<Command> commands = parseTokens(tokens);
            executeCommands(commands);
        }
        catch (const ShellError &e)
        {
            handleError(e.what());
        }
    }
}

// Function to run the shell in script mode
void scriptMode(const string &fileName)
{
    ifstream file(fileName);
    if (!file)
    {
        handleError("Cannot open file " + fileName, true);
    }

    string line;
    int lineNumber = 0;

    while (getline(file, line))
    {
        lineNumber++;
        try
        {
            if (line.empty() || line[0] == '#')
                continue;

            vector<string> tokens = tokenize(line);
            if (tokens.empty())
                continue;

            vector<Command> commands = parseTokens(tokens);
            executeCommands(commands);
        }
        catch (const ShellError &e)
        {
            handleError("Line " + to_string(lineNumber) + ": " + e.what());
        }
    }
}

// Main function to run the shell
int main(int argc, char *argv[])
{
    try
    {
        // Parse command line arguments
        int scriptArgIndex = 1;
        if (argc > 1 && string(argv[1]) == "-p")
        {
            showPath = true;
            scriptArgIndex++;
        }

        if (argc > scriptArgIndex + 1)
        {
            handleError("Usage: ./mish [-p / script.sh]", true);
        }

        // Initialize environment
        char *path = getenv("PATH");
        if (path)
        {
            env.set("PATH", path);
        }

        if (scriptArgIndex >= argc)
        {
            cout << "*******************************************" << endl;
            cout << "       WELCOME TO MISH [MINES-SHELL]       " << endl;
            cout << "*******************************************" << endl;
            interactiveMode();
        }
        else
        {
            cout << "*******************************************" << endl;
            cout << "       WELCOME TO MISH [MINES-SHELL]       " << endl;
            cout << "          YOUR SCRIPT IS RUNNING           " << endl;
            cout << "*******************************************" << endl;
            scriptMode(argv[scriptArgIndex]);
        }
    }
    catch (const exception &e)
    {
        handleError(e.what(), true);
    }

    return 0;
}