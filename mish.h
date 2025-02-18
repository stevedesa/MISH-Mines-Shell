#ifndef MISH_H
#define MISH_H

#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <map>
#include <cstring>
#include <memory>
#include <stdexcept>

using namespace std;

// Structure representing a parsed command
struct Command
{
    vector<string> tokens; // Command and its arguments
    bool redirectOutputToFile = false;
    bool redirectedInputFromFile = false;
    bool isPipeStart = false;
    bool isPipeEnd = false;
    bool isBackground = false;
    string redirectOutputFileName;  // File for output redirection
    string redirectedInputFileName; // File for input redirection
    bool appendOutput = false;      // Append mode for output redirection
};

// Custom exception class for shell errors
class ShellError : public runtime_error
{
public:
    ShellError(const string &msg) : runtime_error(msg) {}
};

// Function to handle errors and optionally terminate the program
void handleError(const string &message, bool fatal = false);

// Class to manage environment variables
class Environment
{
private:
    map<string, string> vars; // Stores environment variables

public:
    Environment(); // Constructor to initialize with system environment

    bool set(const string &name, const string &value); // Set an environment variable
    bool unset(const string &name);                    // Unset an environment variable
    string get(const string &name) const;              // Retrieve an environment variable
    const map<string, string> &getAll() const;         // Get all stored environment variables
};

// Global environment object
extern Environment env;

// Main functions
vector<string> tokenize(const string &input);
bool validateCommand(const Command &cmd);
vector<Command> parseTokens(const vector<string> &tokens);
bool isBuiltInCommand(const string &cmd);
void executeBuiltIn(const Command &cmd);
void setupRedirection(const Command &cmd);
void executePipeline(vector<Command> &pipeline);
void executeCommands(const vector<Command> &commands);
void interactiveMode();
void scriptMode(const string &fileName);

#endif // MISH_H