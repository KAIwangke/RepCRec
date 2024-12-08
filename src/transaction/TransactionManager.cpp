/**
 * @   Author: Ke Wang & Siwen Tao
 * @   Email: kw3484@nyu.edu & st5297@nyu.edu
 * @   Modified time: 2024-12-08 22:22:55
 */

#include "TransactionManager.h"
#include "CommandParser.h"
#include <iostream>
#include <chrono>
#include <string>
#include <regex>

using namespace std;

// Description: Initializes TransactionManager with data manager
// Input: dm - shared pointer to DataManager
// Output: None
// Side Effects: Sets up transaction manager state
TransactionManager::TransactionManager(shared_ptr<DataManager> dm)
    : dataManager(dm) {}

namespace
{
    // Description: Extracts numeric index from variable name
    // Input: varName - string (e.g., "x3")
    // Output: integer index or -1 if invalid
    // Side Effects: None    
    int getVarIndex(const string &varName)
    {
        regex rx("x(\\d+)");
        smatch match;
        if (regex_match(varName, match, rx))
        {
            return stoi(match[1]);
        }
        return -1;
    }
}

// Description: Starts a new transaction
// Input: transactionName - identifier, isReadOnly - read-only flag
// Output: None
// Side Effects: Creates new transaction or prints error if exists
void TransactionManager::beginTransaction(const string &transactionName, bool isReadOnly)
{
    if (transactions.find(transactionName) != transactions.end())
    {
        cout << "Transaction " << transactionName << " already exists.\n";
        return;
    }

    auto transaction = make_shared<Transaction>(transactionName, isReadOnly);
    transactions[transactionName] = transaction;
    cout << "Transaction " << transactionName << " started"
         << (isReadOnly ? " (Read-Only)" : "") << ".\n";
}

// Description: Executes read operation for transaction
// Input: transactionName - transaction ID, variableName - variable to read
// Output: None
// Side Effects: Updates read sets, prints value or errors, may abort transaction
void TransactionManager::read(const string& transactionName, const string& variableName) {
    auto it = transactions.find(transactionName);
    if (it == transactions.end() || it->second->getStatus() != TransactionStatus::ACTIVE) {
        cout << "Transaction " << transactionName << " is not active.\n";
        return;
    }

    auto transaction = it->second;
    int varIndex = getVarIndex(variableName);
    if (varIndex < 1 || varIndex > 20) {
        cout << "Invalid variable name: " << variableName << endl;
        abortTransaction(transaction);
        return;
    }

    try {
        int value = dataManager->read(transactionName, variableName, transaction->getStartTime());
        transaction->addReadVariable(variableName);
        cout << variableName << ": " << value << endl;
        readTable[variableName].insert(transaction->getName());
    }
    catch (const runtime_error& e) {
        string errorMsg = e.what();
        if (errorMsg == "Transaction must wait") {
            return;  
        }
        abortTransaction(transaction);
    }
}

// Description: Processes write operation for transaction
// Input: transactionName - transaction ID, variableName - variable to write, value - new value
// Output: None
// Side Effects: Buffers write, updates site lists, may abort transaction
void TransactionManager::write(const string &transactionName, const string &variableName, int value)
{
    auto it = transactions.find(transactionName);
    if (it == transactions.end() || it->second->getStatus() != TransactionStatus::ACTIVE)
    {
        cout << "Transaction " << transactionName << " is not active.\n";
        return;
    }

    auto transaction = it->second;
    if (transaction->isReadOnly())
    {
        cout << "Read-only transaction " << transactionName << " cannot perform writes.\n";
        abortTransaction(transaction);
        return;
    }

    int varIndex = getVarIndex(variableName);
    if (varIndex < 1 || varIndex > 20)
    {
        cout << "Invalid variable name: " << variableName << endl;
        abortTransaction(transaction);
        return;
    }

    std::vector<int> siteIdsToWrite;
    if (varIndex % 2 == 0)
    { // Even variable, replicated
        for (const auto &site : dataManager->getAllSites())
        {
            if (site->getStatus() == SiteStatus::UP)
            {
                if (site->hasVariable(variableName))
                {
                    siteIdsToWrite.push_back(site->getId());
                }
            }
        }
    }
    else
    { // Odd variable, located at one site
        int siteId = 1 + (varIndex % 10);
        auto site = dataManager->getSite(siteId);
        if (site && site->getStatus() == SiteStatus::UP)
        {
            if (site->hasVariable(variableName))
            {
                siteIdsToWrite.push_back(siteId);
            }
        }
    }

    transaction->addSitesWritten(siteIdsToWrite);

    transaction->addWriteVariable(variableName, value);
    cout << "Write of " << value << " to " << variableName
         << " buffered for transaction " << transactionName << endl;
}

// Description: Completes transaction execution
// Input: transactionName - transaction to end
// Output: None
// Side Effects: Validates and commits/aborts transaction
void TransactionManager::endTransaction(const string &transactionName)
{
    auto it = transactions.find(transactionName);
    if (it == transactions.end())
    {
        cout << "Transaction " << transactionName << " not found.\n";
        return;
    }

    auto transaction = it->second;
    if (transaction->getStatus() != TransactionStatus::ACTIVE)
    {
        cout << "Transaction " << transactionName << " is not active.\n";
        return;
    }

    validateAndCommit(transaction);

}

// Description: Validates transaction and attempts to commit
// Input: transaction - pointer to transaction
// Output: None
// Side Effects: Updates transaction status, commits changes or aborts
void TransactionManager::validateAndCommit(shared_ptr<Transaction> transaction)
{
    // first checking if the transaction is readonly
    if (transaction->isReadOnly())
    {
        transaction->setStatus(TransactionStatus::COMMITTED);
        cout << transaction->getName() << " committed (Read-Only)." << endl;
        return;
    }

    long transactionStartTime = transaction->getStartTime();
    long transactionCommitTime = std::chrono::system_clock::now().time_since_epoch().count();

    for (int siteId : transaction->getSitesWrittenTo())
    {
        auto site = dataManager->getSite(siteId);
        if (site)
        {
            const auto &failureTimes = site->getFailureTimes();
            for (const auto &[failTime, recoverTime] : failureTimes)
            {
                if (failTime <= transactionCommitTime &&
                    (recoverTime == -1 || recoverTime >= transactionStartTime))
                {
                    cout << transaction->getName() << " aborts due to failure of site " << siteId << endl;
                    abortTransaction(transaction);
                    return;
                }
            }
        }
    }

    // Check write-write conflicts (first-committer wins)
    bool hasConflict = false;
    const auto &writeSet = transaction->getWriteSet();
    long startTime = transaction->getStartTime();

    for (auto it = writeSet.begin(); it != writeSet.end(); ++it)
    {
        const string &variableName = it->first;
        if (dataManager->hasCommittedWrite(variableName, startTime))
        {
            cout << "Write-write conflict detected on " << variableName
                 << " for transaction " << transaction->getName() << endl;
            hasConflict = true;
            break;
        }
    }

    if (hasConflict)
    {
        abortTransaction(transaction);
        return;
    }

    for (const auto &variableName : transaction->getReadSet())
    {
        readTable[variableName].insert(transaction->getName());
    }

    for (const auto &[variableName, value] : transaction->getWriteSet())
    {
        for (const auto &readerTransactionName : readTable[variableName])
        {
            if (readerTransactionName != transaction->getName())
            {
                auto readerTransaction = transactions[readerTransactionName];
                readerTransaction->addDependency(transaction->getName());
            }
        }
        writeTable[variableName].insert(transaction->getName());
    }

    for (const auto &variableName : transaction->getReadSet())
    {
        for (const auto &writerTransactionName : writeTable[variableName])
        {
            if (writerTransactionName != transaction->getName())
            {
                auto writerTransaction = transactions[writerTransactionName];
                if (writerTransaction->getCommitTime() == 0 || writerTransaction->getCommitTime() > transaction->getStartTime())
                {
                    transaction->addDependency(writerTransactionName);
                }
            }
        }
    }

    // Detect cycles
    if (detectCycle(transaction->getName()))
    {
        std::cout << transaction->getName() << " aborts due to cycle in dependency graph." << std::endl;
        abortTransaction(transaction);
        return;
    }

    // If no conflicts, commit the transaction
    long commitTime = chrono::system_clock::now().time_since_epoch().count();
    transaction->setCommitTime(commitTime);

    dataManager->commitTransaction(transaction);

    transaction->setStatus(TransactionStatus::COMMITTED);
    cout << transaction->getName() << " committed." << endl;
}

// Description: Aborts a transaction
// Input: transaction - pointer to transaction to abort
// Output: None
// Side Effects: Sets status to ABORTED, prints message
void TransactionManager::abortTransaction(shared_ptr<Transaction> transaction)
{
    transaction->setStatus(TransactionStatus::ABORTED);
    cout << "Transaction " << transaction->getName() << " aborted.\n";
}

// Description: Outputs current database state
// Input: None
// Output: None
// Side Effects: Prints site information to console
void TransactionManager::dump() const
{
    dataManager->dump();
}

// Description: Marks site as failed
// Input: siteId - ID of site to fail 
// Output: None
// Side Effects: Updates site status, may affect transactions
void TransactionManager::failSite(int siteId)
{
    dataManager->failSite(siteId);
}

// Description: Recovers failed site
// Input: siteId - ID of site to recover
// Output: None
// Side Effects: Updates site status, processes pending reads
void TransactionManager::recoverSite(int siteId)
{
    dataManager->recoverSite(siteId);
}

// Description: Checks for cycles in transaction dependencies
// Input: transactionName - starting transaction for check
// Output: bool - true if cycle found
// Side Effects: None
bool TransactionManager::detectCycle(const std::string &transactionName)
{
    std::set<std::string> visited;
    std::set<std::string> recursionStack;
    return dfs(transactionName, visited, recursionStack);
}

// Description: Performs DFS for cycle detection
// Input: transactionName - current node, visited/recursionStack - tracking sets
// Output: bool - true if cycle found
// Side Effects: Updates tracking sets
bool TransactionManager::dfs(const std::string &transactionName, std::set<std::string> &visited, std::set<std::string> &recursionStack)
{
    if (recursionStack.count(transactionName))
    {
        // Cycle detected
        return true;
    }
    if (visited.count(transactionName))
    {
        // Already visited
        return false;
    }
    visited.insert(transactionName);
    recursionStack.insert(transactionName);

    auto transaction = transactions[transactionName];
    for (const auto &dep : transaction->getDependencies())
    {
        if (dfs(dep, visited, recursionStack))
        {
            return true;
        }
    }
    recursionStack.erase(transactionName);
    return false;
}