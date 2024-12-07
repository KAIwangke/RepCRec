/**
 * @ Author: Ke Wang & Siwen Tao
 * @ Email: kw3484@nyu.edu & st5297@nyu.edu
 * @ Create Time: 2024-12-05 20:25:29
 * @ Modified time: 2024-12-05 21:32:36
 */

#include "TransactionManager.h"
#include "CommandParser.h"
#include <iostream>
#include <chrono>
#include <string>
#include <regex>

using namespace std;

TransactionManager::TransactionManager(shared_ptr<DataManager> dm)
    : dataManager(dm) {}

namespace
{
    // Helper function to extract variable index from name (e.g., "x3" -> 3)
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

void TransactionManager::read(const string &transactionName, const string &variableName)
{
    auto it = transactions.find(transactionName);
    if (it == transactions.end() || it->second->getStatus() != TransactionStatus::ACTIVE)
    {
        cout << "Transaction " << transactionName << " is not active.\n";
        return;
    }

    auto transaction = it->second;
    int varIndex = getVarIndex(variableName);
    if (varIndex < 1 || varIndex > 20)
    {
        cout << "Invalid variable name: " << variableName << endl;
        abortTransaction(transaction);
        return;
    }

    try
    {
        int value = dataManager->read(transactionName, variableName, transaction->getStartTime());
        transaction->addReadVariable(variableName);
        cout << variableName << ": " << value << endl;
        readTable[variableName].insert(transaction->getName());
    }
    catch (const runtime_error &e)
    {
        string errorMsg = e.what();
        if (errorMsg == "Transaction must wait")
        {
            return; // Don't abort - transaction is waiting
        }
        // For any other error, abort the transaction
        abortTransaction(transaction);
    }
}
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

    // Determine which sites this write would affect
    std::vector<int> siteIdsToWrite;
    if (varIndex % 2 == 0)
    { // Even variable, replicated
        for (const auto &site : dataManager->getAllSites())
        {
            // Include only sites that are UP
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
        // Include only if the site is UP
        if (site && site->getStatus() == SiteStatus::UP)
        {
            if (site->hasVariable(variableName))
            {
                siteIdsToWrite.push_back(siteId);
            }
        }
    }

    // Update the transaction's sitesWrittenTo set
    transaction->addSitesWritten(siteIdsToWrite);

    // Buffer the write
    transaction->addWriteVariable(variableName, value);
    cout << "Write of " << value << " to " << variableName
         << " buffered for transaction " << transactionName << endl;
}

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

    // Validate and commit/abort the transaction
    validateAndCommit(transaction);

    // Clean up transaction entry
    // transactions.erase(it);
}

void TransactionManager::validateAndCommit(std::shared_ptr<Transaction> transaction)
{
    if (transaction->isReadOnly())
    {
        // Read-only transactions can commit without checks
        transaction->setStatus(TransactionStatus::COMMITTED);
        std::cout << transaction->getName() << " committed (Read-Only)." << std::endl;
        return;
    }

    long transactionStartTime = transaction->getStartTime();
    long transactionCommitTime = std::chrono::system_clock::now().time_since_epoch().count();

    // Check if any site written by this transaction failed during its lifetime
    for (int siteId : transaction->getSitesWrittenTo())
    {
        auto site = dataManager->getSite(siteId);
        if (site)
        {
            const auto &failureTimes = site->getFailureTimes();
            for (auto &ft : failureTimes)
            {
                long failTime = ft.first;
                long recoverTime = ft.second;
                if (failTime <= transactionCommitTime &&
                    (recoverTime == -1 || recoverTime >= transactionStartTime))
                {
                    std::cout << transaction->getName() << " aborts due to failure of site " << siteId << std::endl;
                    abortTransaction(transaction);
                    return;
                }
            }
        }
    }

    // First committer wins for write-write conflicts
    for (const auto &writePair : transaction->getWriteSet())
    {
        const std::string &variableName = writePair.first;
        if (lastWriter.find(variableName) != lastWriter.end() &&
            lastWriter[variableName] != transaction->getName())
        {
            std::cout << transaction->getName() << " aborts due to first committer wins on " << variableName << std::endl;
            abortTransaction(transaction);
            return;
        }
    }

    // ---------------------------------------------------------------------------
    // Add Dependencies for RW Conflicts - BOTH Directions
    //
    // 1) For each variable read by this transaction, add an edge from all writers
    //    of that variable to this transaction. (Writer → Reader)
    for (const auto &variableName : transaction->getReadSet())
    {
        // All transactions that wrote this variable in the past
        for (const auto &writerTransactionName : writeTable[variableName])
        {
            if (writerTransactionName != transaction->getName())
            {
                transaction->addDependency(writerTransactionName);
            }
        }
    }

    // 2) For each variable written by this transaction, add an edge from every reader
    //    that read this variable (before this write) to the current transaction.
    //    (Reader → Writer)
    for (const auto &[variableName, value] : transaction->getWriteSet())
    {
        // If there was a previous writer, add that dependency as well (WW dependency)
        if (lastWriter.find(variableName) != lastWriter.end())
        {
            const std::string &prevWriter = lastWriter[variableName];
            if (prevWriter != transaction->getName())
            {
                transaction->addDependency(prevWriter);
            }
        }

        // Now handle Reader → Writer edges: all transactions that previously read this variable
        // must come before this transaction.
        for (const auto &readerTransactionName : readTable[variableName])
        {
            if (readerTransactionName != transaction->getName())
            {
                transaction->addDependency(readerTransactionName);
            }
        }
    }
    // ---------------------------------------------------------------------------

    // Check for cycles in the dependency graph
    if (detectCycle(transaction->getName()))
    {
        std::cout << transaction->getName() << " aborts due to cycle in dependency graph." << std::endl;
        abortTransaction(transaction);
        return;
    }

    // If no conflicts or cycles, commit
    long commitTime = std::chrono::system_clock::now().time_since_epoch().count();
    transaction->setCommitTime(commitTime);

    // Perform the actual database writes
    dataManager->commitTransaction(transaction);

    // Update lastWriter for all variables written by this transaction
    for (const auto &[variableName, val] : transaction->getWriteSet())
    {
        lastWriter[variableName] = transaction->getName();
        writeTable[variableName].insert(transaction->getName());
    }

    // Update readTable for variables read
    for (const auto &variableName : transaction->getReadSet())
    {
        readTable[variableName].insert(transaction->getName());
    }

    transaction->setStatus(TransactionStatus::COMMITTED);
    std::cout << transaction->getName() << " committed." << std::endl;
}

void TransactionManager::abortTransaction(shared_ptr<Transaction> transaction)
{
    transaction->setStatus(TransactionStatus::ABORTED);
    cout << "Transaction " << transaction->getName() << " aborted.\n";
}

void TransactionManager::dump() const
{
    dataManager->dump();
}

void TransactionManager::failSite(int siteId)
{
    dataManager->failSite(siteId);
}

void TransactionManager::recoverSite(int siteId)
{
    dataManager->recoverSite(siteId);
}

bool TransactionManager::detectCycle(const std::string &transactionName)
{
    std::set<std::string> visited;
    std::set<std::string> recursionStack;
    return dfs(transactionName, visited, recursionStack);
}

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