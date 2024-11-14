#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include "TransactionManager.h"
#include "DataManager.h"
#include "CommandParser.h"

int main()
{
    auto dataManager = std::make_shared<DataManager>();
    TransactionManager transactionManager(dataManager);
    CommandParser parser(transactionManager);

    std::ifstream inputFile("test3.txt");
    if (!inputFile.is_open())
    {
        std::cerr << "Failed to open input file 'test1.txt'.\n";
        return 1;
    }

    std::string command;
    while (std::getline(inputFile, command))
    {
        // Skip empty lines or comments
        if (command.empty() || command[0] == '/')
            continue;

        parser.parseCommand(command);
    }

    inputFile.close();
    return 0;
}
