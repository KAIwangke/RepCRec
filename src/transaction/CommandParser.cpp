#include "CommandParser.h"
#include "TransactionManager.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>

using namespace std;

CommandParser::CommandParser(TransactionManager& tm)
    : transactionManager(tm) {}

// Helper function to trim whitespace
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Extract single argument from command like "begin(T1)" -> "T1"
string extractArgument(const string& command) {
    size_t start = command.find('(');
    size_t end = command.find(')');
    if (start == string::npos || end == string::npos || start >= end) {
        return "";
    }
    return trim(command.substr(start + 1, end - start - 1));
}

// Extract multiple arguments from command like "W(T1,x1,101)" -> ["T1", "x1", "101"]
vector<string> extractArguments(const string& command) {
    vector<string> args;
    size_t start = command.find('(');
    size_t end = command.find(')');
    
    if (start == string::npos || end == string::npos || start >= end) {
        return args;
    }
    
    string argsStr = command.substr(start + 1, end - start - 1);
    stringstream ss(argsStr);
    string arg;
    
    while (getline(ss, arg, ',')) {
        args.push_back(trim(arg));
    }
    
    return args;
}

void CommandParser::parseCommand(const string& command) {
    string trimmedCommand = trim(command);

    if (trimmedCommand.empty() || trimmedCommand[0] == '/') return;

    if (trimmedCommand.substr(0, 6) == "begin(") {
        string txnName = extractArgument(trimmedCommand);
        transactionManager.beginTransaction(txnName, false);
    } else if (trimmedCommand.substr(0, 8) == "beginRO(") {
        string txnName = extractArgument(trimmedCommand);
        transactionManager.beginTransaction(txnName, true);
    } else if (trimmedCommand.substr(0, 2) == "W(") {
        vector<string> args = extractArguments(trimmedCommand);
        if (args.size() >= 3) {
            transactionManager.write(args[0], args[1], stoi(args[2]));
        }
    } else if (trimmedCommand.substr(0, 2) == "R(") {
        vector<string> args = extractArguments(trimmedCommand);
        if (args.size() >= 2) {
            transactionManager.read(args[0], args[1]);
        }
    } else if (trimmedCommand.substr(0, 4) == "end(") {
        string txnName = extractArgument(trimmedCommand);
        transactionManager.endTransaction(txnName);
    } else if (trimmedCommand == "dump()") {
        // Access dataManager through transactionManager
        transactionManager.dump();
    } else {
        cerr << "Unknown command: " << command << endl;
    }    
}
vector<string> CommandParser::tokenize(const string& str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}