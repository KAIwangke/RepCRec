// DataManager.cpp
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

    if (varIndex % 2 == 1)
    { // Odd variables
        int siteId = 1 + (varIndex % 10);
        auto site = sites[siteId];

        // Allow reads from UP and RECOVERING sites for odd variables
        if (site->getStatus() == SiteStatus::UP || site->getStatus() == SiteStatus::RECOVERING)
        {
            return site->readVariable(variableName, timestamp);
        }
        throw runtime_error("Site " + to_string(siteId) + " is down or unavailable");
    }
    else
    { // Even variables - read from any up site
        for (auto &sitePair : sites)
        {
            auto site = sitePair.second;
            if (site->getStatus() == SiteStatus::UP)
            {
                // Check if variable is available
                try
                {
                    return site->readVariable(variableName, timestamp);
                }
                catch (const std::exception &)
                {
                    // Variable might be unavailable at this site, continue to next
                    continue;
                }
            }
        }
        throw runtime_error("No available site to read " + variableName);
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
