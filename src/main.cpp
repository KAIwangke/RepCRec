/**
 * @   Author: Ke Wang & Siwen Tao
 * @   Email: kw3484@nyu.edu & st5297@nyu.edu
 * @   Modified time: 2024-12-08 22:24:00
 */

// Description: Main entry point for distributed database system. Handles command input
// processing from either stdin or file input, and coordinates system components.
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include "TransactionManager.h"
#include "DataManager.h"
#include "CommandParser.h"
using namespace std;

// Description: Main program entry point
// Input: argc (int) - argument count, argv (char*[]) - argument values
// Output: int - 0 for success, 1 for file error
// Side Effects: Processes commands, manages database state
int main(int argc, char* argv[]) {
    auto dataManager = make_shared<DataManager>();
    TransactionManager transactionManager(dataManager);
    CommandParser parser(transactionManager);

    istream* input = &cin;
    ifstream inputFile;
    if (argc > 1) {
        inputFile.open(argv[1]);
        if (!inputFile.is_open()) {
            cerr << "Failed to open input file '" << argv[1] << "'.\n";
            return 1;
        }
        input = &inputFile;
    }

    string command;
    while (getline(*input, command)) {
        // Skip empty lines or comments
        if (command.empty() || command[0] == '/') {
            continue;
        }
        // Process each command immediately
        parser.parseCommand(command);
        // Flush output after each command to ensure sequential output
        cout.flush();
    }

    if (inputFile.is_open()) {
        inputFile.close();
    }

    return 0;
}