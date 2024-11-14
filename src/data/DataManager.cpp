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

int DataManager::read(const string &transactionName, const string &variableName, long timestamp)
{
    int varIndex = stoi(variableName.substr(1));
    int initialValue = varIndex * 10; // Initial value for any variable xi is 10*i

    if (varIndex % 2 == 1)
    { // Odd variables
        int siteId = 1 + (varIndex % 10);
        auto site = sites[siteId];
        if (site->getStatus() == SiteStatus::UP)
        {
            return site->readVariable(variableName, timestamp);
        }
        throw runtime_error("Site " + to_string(siteId) + " is down");
    }
    else
    { // Even variables - read from any up site
        for (auto &sitePair : sites)
        {
            auto site = sitePair.second;
            if (site->getStatus() == SiteStatus::UP)
            {
                return site->readVariable(variableName, timestamp);
            }
        }
        throw runtime_error("No available site to read " + variableName);
    }
    return initialValue; // Should never reach here but prevents warning
}

void DataManager::write(std::shared_ptr<Transaction> transaction, const string &variableName, int value, long commitTime)
{
    int varIndex = stoi(variableName.substr(1));

    if (varIndex % 2 == 0)
    { // Even variables - write to all up sites
        for (auto &sitePair : sites)
        {
            auto site = sitePair.second;
            if (site->getStatus() == SiteStatus::UP)
            {
                site->writeVariable(variableName, value, commitTime);
                // Record the site ID in the transaction
                transaction->addSiteWritten(site->getId());
            }
        }
    }
    else
    { // Odd variables - write to specific site
        int siteId = 1 + (varIndex % 10);
        auto site = sites[siteId];
        if (site->getStatus() == SiteStatus::UP)
        {
            site->writeVariable(variableName, value, commitTime);
            transaction->addSiteWritten(site->getId());
        }
    }
}

void DataManager::failSite(int siteId)
{
    auto site = getSite(siteId);
    if (site)
    {
        site->fail();
    }
}

void DataManager::dump()
{
    for (const auto &sitePair : sites)
    {
        sitePair.second->dump();
    }
}

void DataManager::recoverSite(int siteId)
{
    auto site = getSite(siteId);
    if (site)
    {
        site->recover();
    }
}