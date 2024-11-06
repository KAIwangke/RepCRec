#include "DataManager.h"

DataManager::DataManager() {
    initializeSites();
}

void DataManager::initializeSites() {
    for (int i = 1; i <= 10; ++i) {
        sites[i] = std::make_shared<Site>(i);
    }
}

std::shared_ptr<Site> DataManager::getSite(int siteId) {
    return sites[siteId];
}

std::vector<std::shared_ptr<Site>> DataManager::getAllSites() {
    std::vector<std::shared_ptr<Site>> siteList;
    for (auto& pair : sites) {
        siteList.push_back(pair.second);
    }
    return siteList;
}


bool DataManager::hasCommittedWrite(const std::string& variableName, long startTime) {
    for (const auto& sitePair : sites) {
        auto site = sitePair.second;
        if (site->hasCommittedWrite(variableName, startTime)) {
            return true;
        }
    }
    return false;
}

void DataManager::commitTransaction(std::shared_ptr<Transaction> transaction) {
    for (const auto& [variableName, value] : transaction->getWriteSet()) {
        for (const auto& sitePair : sites) {
            auto site = sitePair.second;
            if (site->hasVariable(variableName)) {
                site->writeVariable(variableName, value, transaction->getCommitTime());
            }
        }
    }
}

int DataManager::read(const std::string& transactionName, const std::string& variableName, long timestamp) {
    // Implement read logic considering replication and site status
    return 0; // Placeholder return value
}

void DataManager::write(const std::string& transactionName, const std::string& variableName, int value, long commitTime) {
    // Implement write logic considering replication and site status
}

void DataManager::failSite(int siteId) {
    auto site = getSite(siteId);
    if (site) {
        site->fail();
    }
}


void DataManager::dump() {
    for (const auto& sitePair : sites) {
        sitePair.second->dump();
    }
}


void DataManager::recoverSite(int siteId) {
    auto site = getSite(siteId);
    if (site) {
        site->recover();
    }
}

