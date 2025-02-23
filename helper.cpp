#include "mish.h"
#include <cstdlib> // For environ
using namespace std;

// Global environment object to store env variables
Environment env;
extern char **environ;

// Function to handle errors and optionally terminate the program
void handleError(const string &message, bool fatal)
{
    cerr << "Error: " << message << endl; // Standard error
    if (errno != 0)
    {
        perror("System error"); // System error
    }
    if (fatal)
    {
        exit(1); // Terminate on fatal error
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