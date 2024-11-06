#include "CommandParser.h"
#include "TransactionManager.h"
#include <sstream>

CommandParser::CommandParser(TransactionManager& tm)
    : transactionManager(tm) {}

void CommandParser::parseCommand(const std::string& command) {
    // Implement command parsing logic and call appropriate methods on TransactionManager
    std::string trimmedCommand = trim(command); // Implement trim function to remove whitespace

    if (trimmedCommand.empty() || trimmedCommand[0] == '/') return; // Skip comments and empty lines

    if (trimmedCommand.substr(0, 6) == "begin(") {
        std::string txnName = extractArgument(trimmedCommand); // Implement extractArgument to get transaction name
        transactionManager.beginTransaction(txnName, false);
    } else if (trimmedCommand.substr(0, 15) == "beginRO(") {
        std::string txnName = extractArgument(trimmedCommand);
        transactionManager.beginTransaction(txnName, true);
    } else if (trimmedCommand.substr(0, 2) == "W(") {
        std::vector<std::string> args = extractArguments(trimmedCommand); // Extract transaction name, variable, value
        transactionManager.write(args[0], args[1], std::stoi(args[2]));
    } else if (trimmedCommand.substr(0, 2) == "R(") {
        std::vector<std::string> args = extractArguments(trimmedCommand); // Extract transaction name, variable
        transactionManager.read(args[0], args[1]);
    } else if (trimmedCommand.substr(0, 5) == "end(") {
        std::string txnName = extractArgument(trimmedCommand);
        transactionManager.endTransaction(txnName);
    } else if (trimmedCommand == "dump()") {
        dataManager->dump();
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
    }    
}

std::vector<std::string> CommandParser::tokenize(const std::string& str, char delimiter) {
    // Implement tokenization helper method
}

