#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <string>
#include <vector>

class TransactionManager;

class CommandParser {
public:
    CommandParser(TransactionManager& tm);
    void parseCommand(const std::string& command);

private:
    TransactionManager& transactionManager;
    std::vector<std::string> tokenize(const std::string& str, char delimiter);
};

#endif // COMMAND_PARSER_H
