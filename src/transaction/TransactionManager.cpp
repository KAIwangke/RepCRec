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
    // first checking if the transaction is readonly
    if (transaction->isReadOnly())
    {
        transaction->setStatus(TransactionStatus::COMMITTED);
        cout << transaction->getName() << " committed (Read-Only)." << endl;
        return;
    }

    long transactionStartTime = transaction->getStartTime();
    long transactionCommitTime = std::chrono::system_clock::now().time_since_epoch().count();

    // Check if any sites the transaction wrote to have failed
    for (int siteId : transaction->getSitesWrittenTo())
    {
        auto site = dataManager->getSite(siteId);
        if (site)
        {
            const auto &failureTimes = site->getFailureTimes();
            for (const auto &[failTime, recoverTime] : failureTimes)
            {
                // If the site failed after the transaction started and before it committed
                // and the failure interval overlaps with the transaction's lifetime
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

    // If no conflicts, commit the transaction
    long commitTime = chrono::system_clock::now().time_since_epoch().count();
    transaction->setCommitTime(commitTime);

    // Write all buffered writes to database
    dataManager->commitTransaction(transaction);

    transaction->setStatus(TransactionStatus::COMMITTED);
    cout << transaction->getName() << " committed." << endl;
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
