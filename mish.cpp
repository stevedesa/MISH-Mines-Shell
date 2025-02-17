#include "mish.h"
using namespace std;

// Global environment object
Environment env;

// Function to handle errors and optionally terminate the program
void handleError(const string &message, bool fatal)
{
    cerr << "Error: " << message << endl;
    if (errno != 0)
    {
        perror("System error");
    }
    if (fatal)
    {
        exit(1);
    }
}

// Constructor to initialize environment variables from system
Environment::Environment()
{
    for (char **env = environ; *env != nullptr; env++)
    {
        string envStr(*env);
        size_t pos = envStr.find('=');
        if (pos != string::npos)
        {
            vars[envStr.substr(0, pos)] = envStr.substr(pos + 1);
        }
    }
}

// Set an environment variable
bool Environment::set(const string &name, const string &value)
{
    vars[name] = value;
    return setenv(name.c_str(), value.c_str(), 1) == 0;
}

// Unset an environment variable
bool Environment::unset(const string &name)
{
    auto it = vars.find(name);
    if (it != vars.end())
    {
        vars.erase(it);
        unsetenv(name.c_str());
        return true;
    }
    return false;
}

// Retrieve an environment variable's value
string Environment::get(const string &name) const
{
    auto it = vars.find(name);
    return (it != vars.end()) ? it->second : "";
}

// Get all stored environment variables
const map<string, string> &Environment::getAll() const
{
    return vars;
}

// Function to split input string into tokens while handling quotes and escape characters
vector<string> tokenize(const string &input)
{
    vector<string> tokens;
    string current_token;
    bool in_quotes = false;
    bool escaped = false;

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
        throw ShellError("Unterminated quote");
    }

    return tokens;
}

// Function to validate parsed command structure
bool validateCommand(const Command &cmd)
{
    if (cmd.redirectOutputToFile && cmd.isPipeStart)
    {
        handleError("Cannot combine output redirection with pipe output");
        return false;
    }
    if (cmd.redirectedInputFromFile && cmd.isPipeEnd)
    {
        handleError("Cannot combine input redirection with pipe input");
        return false;
    }
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
    vector<Command> commands;
    Command currentCommand;

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
            currentCommand.isBackground = true;
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

// Function to check if a command is built-in (e.g., cd, exit, variable assignment)
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

    if (command == "exit")
    {
        if (cmd.tokens.size() > 1)
        {
            throw ShellError("exit command takes no arguments");
        }
        exit(0);
    }
    else if (command == "cd")
    {
        if (cmd.tokens.size() != 2)
        {
            throw ShellError("cd command requires exactly one argument");
        }
        if (chdir(cmd.tokens[1].c_str()) != 0)
        {
            throw ShellError("cd failed: " + string(strerror(errno)));
        }
    }
    else if (command.find('=') != string::npos) // Handle variable assignment
    {
        size_t pos = command.find('=');
        string var_name = command.substr(0, pos);
        string var_value = pos + 1 < command.length() ? command.substr(pos + 1) : "";

        if (var_value.empty())
        {
            env.unset(var_name);
        }
        else
        {
            env.set(var_name, var_value);
        }
    }
}

// Function to execute a sequence of commands, including handling built-in commands
void executeCommands(const vector<Command> &commands)
{
    vector<Command> current_pipeline;

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
                continue;
            }

            // Add command to pipeline
            current_pipeline.push_back(cmd);

            // Execute pipeline if this is the end of a pipeline or a standalone command
            if (!cmd.isPipeStart)
            {
                executePipeline(current_pipeline);
                current_pipeline.clear();
            }
        }
        catch (const ShellError &e)
        {
            handleError(e.what());
            current_pipeline.clear();
        }
    }
}

void interactiveMode()
{
    string input;
    char cwd[PATH_MAX];

    while (true)
    {
        if (getcwd(cwd, sizeof(cwd)) != nullptr)
        {
            cout << "mish:" << cwd << "> ";
        }
        else
        {
            cout << "mish> ";
        }

        if (!getline(cin, input))
            break;

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

int main(int argc, char *argv[])
{
    try
    {
        if (argc > 2)
        {
            handleError("Usage: ./mish [script.sh]", true);
        }

        // Initialize environment
        char *path = getenv("PATH");
        if (path)
        {
            env.set("PATH", path);
        }

        if (argc == 1)
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
            scriptMode(argv[1]);
        }
    }
    catch (const exception &e)
    {
        handleError(e.what(), true);
    }

    return 0;
}