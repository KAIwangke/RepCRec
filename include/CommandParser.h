/**
 * @   Author: Ke Wang & Siwen Tao
 * @   Email: kw3484@nyu.edu & st5297@nyu.edu
 * @   Modified time: 2024-12-08 22:07:05
 */

// Parses input commands for the distributed database system and converts them into
// structured operations. Handles transaction commands (begin, read, write, end) and
// system commands (fail, recover, dump).
#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H
#include <string>
#include <vector>
class TransactionManager;
class CommandParser {
public:
 // Initialize parser with transaction manager reference
CommandParser(TransactionManager& tm);
 // Parse and execute a single command string
void parseCommand(const std::string& command);
private:
TransactionManager& transactionManager;
 // Split string into tokens based on delimiter
 std::vector<std::string> tokenize(const std::string& str, char delimiter);
};
#endif // COMMAND_PARSER_H