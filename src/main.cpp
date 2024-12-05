/**
 * @ Author: Ke Wang & Siwen Tao
 * @ Email: kw3484@nyu.edu & st5297@nyu.edu
 * @ Create Time: 2024-11-26 20:07:56
 * @ Modified time: 2024-12-05 21:32:40
 */

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include "TransactionManager.h"
#include "DataManager.h"
#include "CommandParser.h"

int main(int argc, char* argv[]) {
    auto dataManager = std::make_shared<DataManager>();
    TransactionManager transactionManager(dataManager);
    CommandParser parser(transactionManager);

    std::istream* input = &std::cin; // Default to standard input
    std::ifstream inputFile;

    // If filename provided as command line argument
    if (argc > 1) {
        inputFile.open(argv[1]);
        if (!inputFile.is_open()) {
            std::cerr << "Failed to open input file '" << argv[1] << "'.\n";
            return 1;
        }
        input = &inputFile;
    }

    std::string command;
    while (std::getline(*input, command)) {
        // Skip empty lines or comments
        if (command.empty() || command[0] == '/') {
            continue;
        }

        // Process each command immediately
        parser.parseCommand(command);

        // Flush output after each command to ensure sequential output
        std::cout.flush();
    }

    if (inputFile.is_open()) {
        inputFile.close();
    }

    return 0;
}