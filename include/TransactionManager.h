/**
 * @   Author: Ke Wang & Siwen Tao
 * @   Email: kw3484@nyu.edu & st5297@nyu.edu
 * @   Modified time: 2024-12-08 22:16:43
 */

// Coordinates transaction execution, manages concurrency control, and ensures ACID properties
// across the distributed database system
#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include <string>
#include <map>
#include <memory>
#include "Transaction.h"
#include "DataManager.h"

class TransactionManager
{
public:
    // Initializes transaction manager with a data manager reference
    TransactionManager(std::shared_ptr<DataManager> dm);

    // Processes and executes a database command
    void processCommand(const std::string &command);

    // Creates a new transaction with specified properties
    void beginTransaction(const std::string &transactionName, bool isReadOnly);

    // Executes a read operation for the specified transaction
    void read(const std::string &transactionName, const std::string &variableName);

    // Records a write operation for the transaction
    void write(const std::string &transactionName, const std::string &variableName, int value);

    // Attempts to commit or abort the specified transaction
    void endTransaction(const std::string &transactionName);

    // Displays current state of all database sites
    void dump() const;

    // Marks a site as failed and handles cleanup
    void failSite(int siteId);

    // Recovers a failed site and processes pending operations
    void recoverSite(int siteId);

private:
    std::map<std::string, std::shared_ptr<Transaction>> transactions;  // Active transactions in the system
    std::shared_ptr<DataManager> dataManager;                          // Interface to distributed data sites
    std::map<std::string, std::set<std::string>> readTable;           // Tracks which transactions read each variable
    std::map<std::string, std::set<std::string>> writeTable;          // Tracks which transactions wrote each variable

    // Validates transaction's operations and commits if valid
    void validateAndCommit(std::shared_ptr<Transaction> transaction);

    // Rolls back a transaction's operations
    void abortTransaction(std::shared_ptr<Transaction> transaction);

    // Checks for dependency cycles in transaction graph
    bool detectCycle(const std::string &transactionName);

    // Performs depth-first search for cycle detection
    bool dfs(const std::string &transactionName, std::set<std::string> &visited, std::set<std::string> &recursionStack);
};

#endif // TRANSACTION_MANAGER_H