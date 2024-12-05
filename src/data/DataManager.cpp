#include "DataManager.h"
#include <iostream>
using namespace std;

DataManager::DataManager()
{
    initializeSites();
}

void DataManager::initializeSites()
{
    for (int i = 1; i <= 10; ++i)
    {
        sites[i] = std::make_shared<Site>(i);
    }
}

std::shared_ptr<Site> DataManager::getSite(int siteId)
{
    return sites[siteId];
}

std::vector<std::shared_ptr<Site>> DataManager::getAllSites()
{
    std::vector<std::shared_ptr<Site>> siteList;
    for (auto &pair : sites)
    {
        siteList.push_back(pair.second);
    }
    return siteList;
}

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

void DataManager::commitTransaction(std::shared_ptr<Transaction> transaction)
{
    for (const auto &write : transaction->getWriteSet())
    {
        const std::string &variableName = write.first;
        int value = write.second;
        DataManager::write(transaction, variableName, value, transaction->getCommitTime());
    }
}



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


void DataManager::dump()
{
    for (const auto &sitePair : sites)
    {
        sitePair.second->dump();
    }
}

#include "DataManager.h"
#include <iostream>
using namespace std;

bool DataManager::hasSiteStableHistory(std::shared_ptr<Site> site, long timestamp) const {
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

bool DataManager::hasContinuousHistory(std::shared_ptr<Site> site, long fromTime, long toTime) const {
    const auto& failureTimes = site->getFailureTimes();
    for (const auto& ft : failureTimes) {
        // Check if any failure interval overlaps with [fromTime, toTime]
        if (ft.first <= toTime && (ft.second == -1 || ft.second >= fromTime)) {
            return false;
        }
    }
    return true;
}

int DataManager::read(const string& transactionName, const string& variableName, long timestamp) {
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
void DataManager::recoverSite(int siteId) {
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


void DataManager::failSite(int siteId) {
    auto site = getSite(siteId);
    if (!site) return;
    
    if (site->getStatus() != SiteStatus::DOWN) {
        site->fail();
        cout << "Site " << siteId << " failed." << endl;
    }
}
