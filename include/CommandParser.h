/**
 * @ Author: Ke Wang & Siwen Tao
 * @ Email: kw3484@nyu.edu & st5297@nyu.edu
 * @ Create Time: 2024-11-26 20:07:56
 * @ Modified time: 2024-12-05 21:33:27
 */

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
