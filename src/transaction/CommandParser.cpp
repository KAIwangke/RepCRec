#include "CommandParser.h"
#include "TransactionManager.h"
#include <sstream>
#include <iostream>
#include <algorithm>

CommandParser::CommandParser(TransactionManager& tm)
    : transactionManager(tm) {}

// Helper function to trim whitespace
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Extract single argument from command like "begin(T1)" -> "T1"
std::string extractArgument(const std::string& command) {
    size_t start = command.find('(');
    size_t end = command.find(')');
    if (start == std::string::npos || end == std::string::npos || start >= end) {
        return "";
    }
    return trim(command.substr(start + 1, end - start - 1));
}

// Extract multiple arguments from command like "W(T1,x1,101)" -> ["T1", "x1", "101"]
std::vector<std::string> extractArguments(const std::string& command) {
    std::vector<std::string> args;
    size_t start = command.find('(');
    size_t end = command.find(')');
    
    if (start == std::string::npos || end == std::string::npos || start >= end) {
        return args;
    }
    
    std::string argsStr = command.substr(start + 1, end - start - 1);
    std::stringstream ss(argsStr);
    std::string arg;
    
    while (std::getline(ss, arg, ',')) {
        args.push_back(trim(arg));
    }
    
    return args;
}

void CommandParser::parseCommand(const std::string& command) {
    std::string trimmedCommand = trim(command);

    if (trimmedCommand.empty() || trimmedCommand[0] == '/') return;

    if (trimmedCommand.substr(0, 6) == "begin(") {
        std::string txnName = extractArgument(trimmedCommand);
        transactionManager.beginTransaction(txnName, false);
    } else if (trimmedCommand.substr(0, 8) == "beginRO(") {
        std::string txnName = extractArgument(trimmedCommand);
        transactionManager.beginTransaction(txnName, true);
    } else if (trimmedCommand.substr(0, 2) == "W(") {
        std::vector<std::string> args = extractArguments(trimmedCommand);
        if (args.size() >= 3) {
            transactionManager.write(args[0], args[1], std::stoi(args[2]));
        }
    } else if (trimmedCommand.substr(0, 2) == "R(") {
        std::vector<std::string> args = extractArguments(trimmedCommand);
        if (args.size() >= 2) {
            transactionManager.read(args[0], args[1]);
        }
    } else if (trimmedCommand.substr(0, 4) == "end(") {
        std::string txnName = extractArgument(trimmedCommand);
        transactionManager.endTransaction(txnName);
    } else if (trimmedCommand == "dump()") {
        // Access dataManager through transactionManager
        transactionManager.dump();
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
    }    
}
std::vector<std::string> CommandParser::tokenize(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}