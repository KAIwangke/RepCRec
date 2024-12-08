/**
 * @   Author: Ke Wang & Siwen Tao
 * @   Email: kw3484@nyu.edu & st5297@nyu.edu
 * @   Modified time: 2024-12-08 22:18:23
 */

#include "CommandParser.h"
#include "TransactionManager.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>
using namespace std;

// Description: Constructs command parser with transaction manager reference
// Input: tm (TransactionManager&) - reference to transaction manager
// Output: None
// Side Effects: Initializes parser with transaction manager
CommandParser::CommandParser(TransactionManager &tm)
    : transactionManager(tm) {}

// Description: Removes leading and trailing whitespace from string
// Input: str (string) - input string to trim
// Output: string - trimmed string
// Side Effects: None
string trim(const string &str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Description: Extracts single argument from parentheses in command
// Input: command (string) - command containing argument in parentheses
// Output: string - extracted argument or empty string if invalid format
// Side Effects: None
string extractArgument(const string &command)
{
    size_t start = command.find('(');
    size_t end = command.find(')');
    if (start == string::npos || end == string::npos || start >= end)
    {
        return "";
    }
    return trim(command.substr(start + 1, end - start - 1));
}

// Description: Extracts multiple comma-separated arguments from command
// Input: command (string) - command containing arguments in parentheses
// Output: vector<string> - list of extracted arguments
// Side Effects: None
vector<string> extractArguments(const string &command)
{
    vector<string> args;
    size_t start = command.find('(');
    size_t end = command.find(')');
    if (start == string::npos || end == string::npos || start >= end)
    {
        return args;
    }
    string argsStr = command.substr(start + 1, end - start - 1);
    stringstream ss(argsStr);
    string arg;
    while (getline(ss, arg, ','))
    {
        args.push_back(trim(arg));
    }
    return args;
}

// Description: Parses and executes database commands
// Input: command (string) - command to parse and execute
// Output: None
// Side Effects: Executes corresponding transaction manager operations
void CommandParser::parseCommand(const string &command)
{
    string trimmedCommand = trim(command);
    if (trimmedCommand.empty() || trimmedCommand[0] == '/')
        return;

    if (trimmedCommand.substr(0, 6) == "begin(")
    {
        string txnName = extractArgument(trimmedCommand);
        transactionManager.beginTransaction(txnName, false);
    }
    else if (trimmedCommand.substr(0, 8) == "beginRO(")
    {
        string txnName = extractArgument(trimmedCommand);
        transactionManager.beginTransaction(txnName, true);
    }
    else if (trimmedCommand.substr(0, 2) == "W(")
    {
        vector<string> args = extractArguments(trimmedCommand);
        if (args.size() >= 3)
        {
            transactionManager.write(args[0], args[1], stoi(args[2]));
        }
    }
    else if (trimmedCommand.substr(0, 2) == "R(")
    {
        vector<string> args = extractArguments(trimmedCommand);
        if (args.size() >= 2)
        {
            transactionManager.read(args[0], args[1]);
        }
    }
    else if (trimmedCommand.substr(0, 4) == "end(")
    {
        string txnName = extractArgument(trimmedCommand);
        transactionManager.endTransaction(txnName);
    }
    else if (trimmedCommand == "dump()")
    {
        transactionManager.dump();
    }
    else if (trimmedCommand.substr(0, 5) == "fail(")
    {
        string siteIdStr = extractArgument(trimmedCommand);
        int siteId = stoi(siteIdStr);
        transactionManager.failSite(siteId);
    }
    else if (trimmedCommand.substr(0, 8) == "recover(")
    {
        string siteIdStr = extractArgument(trimmedCommand);
        int siteId = stoi(siteIdStr);
        transactionManager.recoverSite(siteId);
    }
    else
    {
        cerr << "Unknown command: " << command << endl;
    }
}

// Description: Splits string into tokens based on delimiter
// Input: str (string) - string to tokenize, delimiter (char) - separator
// Output: vector<string> - list of tokens
// Side Effects: None
vector<string> CommandParser::tokenize(const string &str, char delimiter)
{
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter))
    {
        tokens.push_back(trim(token));
    }
    return tokens;
}