/**
 * @   Author: Ke Wang & Siwen Tao
 * @   Email: kw3484@nyu.edu & st5297@nyu.edu
 * @   Modified time: 2024-12-08 22:17:45
 */

#include "DataManager.h"
#include <iostream>
using namespace std;

// Description: Constructor that sets up the distributed database system
// Input: None
// Output: None
// Side Effects: Initializes all 10 database sites
DataManager::DataManager()
{
    initializeSites();
}

// Description: Creates the initial set of database sites
// Input: None
// Output: None
// Side Effects: Creates 10 Site objects and stores them in sites map
void DataManager::initializeSites()
{
    for (int i = 1; i <= 10; ++i)
    {
        sites[i] = std::make_shared<Site>(i);
    }
}

// Description: Retrieves a specific site by its ID
// Input: siteId (int)
// Output: Shared pointer to Site object
// Side Effects: None
std::shared_ptr<Site> DataManager::getSite(int siteId)
{
    return sites[siteId];
}

// Description: Returns all database sites in the system
// Input: None
// Output: Vector of Site pointers
// Side Effects: None
std::vector<std::shared_ptr<Site>> DataManager::getAllSites()
{
    std::vector<std::shared_ptr<Site>> siteList;
    for (auto &pair : sites)
    {
        siteList.push_back(pair.second);
    }
    return siteList;
}

// Description: Checks if any site has a committed write for a variable after given time
// Input: variableName (string), startTime (long)
// Output: Boolean indicating if write exists
// Side Effects: None
bool DataManager::hasCommittedWrite(const std::string &variableName, long startTime)
{
    for (const auto &sitePair : sites)
    {
        auto site = sitePair.second;
        if (site->hasCommittedWrite(variableName, startTime))
        {
            return true;
        }
    }
    return false;
}

// Description: Performs final commit of all pending writes in a transaction
// Input: transaction pointer
// Output: None
// Side Effects: Writes all transaction's pending writes to appropriate sites
void DataManager::commitTransaction(std::shared_ptr<Transaction> transaction)
{
    for (const auto &write : transaction->getWriteSet())
    {
        const std::string &variableName = write.first;
        int value = write.second;
        DataManager::write(transaction, variableName, value, transaction->getCommitTime());
    }
}

// Description: Writes variable to either all sites or single site based on variable type
// Input: transaction pointer, variableName, value, commitTime
// Output: None
// Side Effects: Updates variable value across relevant sites
void DataManager::write(std::shared_ptr<Transaction> transaction, const std::string &variableName, int value, long commitTime)
{
    int varIndex = stoi(variableName.substr(1));

    if (varIndex % 2 == 0)
    { // Even variables - write to all up sites
        for (auto &sitePair : sites)
        {
            auto site = sitePair.second;
            if (site->getStatus() == SiteStatus::UP && site->hasVariable(variableName))
            {
                site->writeVariable(variableName, value, commitTime);
            }
        }
    }
    else
    { // Odd variables - write to specific site
        int siteId = 1 + (varIndex % 10);
        auto site = sites[siteId];
        if (site->getStatus() == SiteStatus::UP && site->hasVariable(variableName))
        {
            site->writeVariable(variableName, value, commitTime);
        }
    }
}

// Description: Outputs current state of all database sites
// Input: None
// Output: None
// Side Effects: Prints state of all sites to console
void DataManager::dump()
{
    for (const auto &sitePair : sites)
    {
        sitePair.second->dump();
    }
}

// Description: Verifies if site has consistent history at given timestamp
// Input: site pointer, timestamp
// Output: Boolean indicating stability
// Side Effects: None
bool DataManager::hasSiteStableHistory(std::shared_ptr<Site> site, long timestamp) const 
{
    if (!site || site->getStatus() == SiteStatus::DOWN) {
        return false;
    }

    const auto& failureTimes = site->getFailureTimes();
    for (const auto& ft : failureTimes) {
        // Check if there was a failure between transaction start and current time
        if (ft.first <= timestamp && (ft.second == -1 || ft.second > timestamp)) {
            return false;
        }
    }
    return true;
}

// Description: Checks if site was continuously up during time period
// Input: site pointer, fromTime, toTime
// Output: Boolean indicating continuous uptime
// Side Effects: None
bool DataManager::hasContinuousHistory(std::shared_ptr<Site> site, long fromTime, long toTime) const 
{
    const auto& failureTimes = site->getFailureTimes();
    for (const auto& ft : failureTimes) {
        // Check if any failure interval overlaps with [fromTime, toTime]
        if (ft.first <= toTime && (ft.second == -1 || ft.second >= fromTime)) {
            return false;
        }
    }
    return true;
}

// Description: Reads variable from appropriate site based on variable type
// Input: transactionName, variableName, timestamp
// Output: Integer value of variable
// Side Effects: May add to waitingReads queue, throws exceptions
int DataManager::read(const string& transactionName, const string& variableName, long timestamp) 
{
    int varIndex = stoi(variableName.substr(1));

    if (varIndex % 2 == 1) { // Odd variables
        int siteId = 1 + (varIndex % 10);
        auto site = sites[siteId];
        if (site->getStatus() == SiteStatus::DOWN) {
            throw runtime_error("Site " + to_string(siteId) + " is down");
        }
        return site->readVariable(variableName, timestamp);
    }
    else { // Even variables
        // First find if there is any site that has a valid history of the variable
        long lastWriteTime = -1;
        bool foundValidVersion = false;
        
        for (auto& sitePair : sites) {
            auto site = sitePair.second;
            if (site->hasVariable(variableName) && 
                hasContinuousHistory(site, lastWriteTime, timestamp)) {
                foundValidVersion = true;
                break;
            }
        }

        // If no site has valid version, abort immediately
        if (!foundValidVersion) {
            throw runtime_error("No valid version of " + variableName);
        }

        // Try to read from an up site with valid version
        for (auto& sitePair : sites) {
            auto site = sitePair.second;
            if (site->getStatus() == SiteStatus::UP &&
                site->hasVariable(variableName) &&
                hasContinuousHistory(site, lastWriteTime, timestamp)) {
                try {
                    return site->readVariable(variableName, timestamp);
                } catch (...) {
                    continue;
                }
            }
        }

        // If we found a valid version but can't access it right now, wait
        cout << "Transaction " << transactionName << " waits for reading "
             << variableName << endl;
        waitingReads.push_back({transactionName, variableName, timestamp});
        throw runtime_error("Transaction must wait");
    }

    throw runtime_error("No available site to read " + variableName);
}

// Description: Brings a failed site back online and processes pending reads
// Input: siteId
// Output: None
// Side Effects: Recovers site, processes waiting reads, prints status
void DataManager::recoverSite(int siteId) 
{
    auto site = getSite(siteId);
    if (!site || site->getStatus() != SiteStatus::DOWN) {
        return;
    }

    site->recover();
    cout << "Site " << siteId << " recovered." << endl;

    // Process waiting reads
    auto it = waitingReads.begin();
    while (it != waitingReads.end()) {
        if (site->hasVariable(it->variableName) && 
            hasSiteStableHistory(site, it->timestamp)) {
            try {
                int value = site->readVariable(it->variableName, it->timestamp);
                cout << it->variableName << ": " << value << endl;
                it = waitingReads.erase(it);
                continue;
            } catch (...) {}
        }
        ++it;
    }
}

// Description: Simulates failure of a database site
// Input: siteId
// Output: None
// Side Effects: Marks site as failed, prints status
void DataManager::failSite(int siteId) 
{
    auto site = getSite(siteId);
    if (!site) return;
    
    if (site->getStatus() != SiteStatus::DOWN) {
        site->fail();
        cout << "Site " << siteId << " failed." << endl;
    }
}