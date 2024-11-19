// TransactionManager.cpp
#include "TransactionManager.h"
#include "CommandParser.h"
#include <iostream>
#include <string>
#include <regex>
#include <atomic>

using namespace std;

// Initialize the static global timestamp
std::atomic<long> TransactionManager::globalTimestamp(0);

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
    long startTime = ++globalTimestamp; // Increment and get the timestamp
    auto transaction = make_shared<Transaction>(transactionName, isReadOnly, startTime);
    transactions[transactionName] = transaction;
    cout << "Transaction " << transactionName << " started"
         << (isReadOnly ? " (Read-Only)" : "") << " at time " << startTime << ".\n";
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
    }
    catch (const exception &e)
    {
        cout << "Read failed for transaction " << transactionName
             << " on variable " << variableName << ": " << e.what() << endl;
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
    transactions.erase(it);
}

void TransactionManager::validateAndCommit(shared_ptr<Transaction> transaction)
{
    if (transaction->isReadOnly())
    {
        // Read-only transactions can commit immediately
        transaction->setStatus(TransactionStatus::COMMITTED);
        cout << transaction->getName() << " committed (Read-Only).\n";
        return;
    }

    long transactionStartTime = transaction->getStartTime();

    // Check for read-write conflicts
    const auto &readSet = transaction->getReadSet();
    for (const auto &variableName : readSet)
    {
        if (dataManager->hasCommittedWrite(variableName, transactionStartTime))
        {
            cout << "Read-write conflict detected on " << variableName
                 << " for transaction " << transaction->getName() << endl;
            abortTransaction(transaction);
            return;
        }
    }

    // Check for write-write conflicts
    const auto &writeSet = transaction->getWriteSet();
    for (const auto &write : writeSet)
    {
        const string &variableName = write.first;
        if (dataManager->hasCommittedWrite(variableName, transactionStartTime))
        {
            cout << "Write-write conflict detected on " << variableName
                 << " for transaction " << transaction->getName() << endl;
            abortTransaction(transaction);
            return;
        }
    }

    // Commit the transaction
    long commitTime = ++globalTimestamp; // Increment and get the timestamp
    transaction->setCommitTime(commitTime);
    dataManager->commitTransaction(transaction);
    transaction->setStatus(TransactionStatus::COMMITTED);
    cout << transaction->getName() << " committed at time " << commitTime << ".\n";
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
    cout << "Site " << siteId << " failed.\n";
}

void TransactionManager::recoverSite(int siteId)
{
    dataManager->recoverSite(siteId);
    cout << "Site " << siteId << " recovered.\n";
}
