#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include "Site.h"
#include "Transaction.h"

class DataManager {
public:
    DataManager();
    void initializeSites();

    std::shared_ptr<Site> getSite(int siteId);
    std::vector<std::shared_ptr<Site>> getAllSites();

    bool hasCommittedWrite(const std::string& variableName, long startTime);
    void commitTransaction(std::shared_ptr<Transaction> transaction);
    void dump();

    // Methods to route read/write requests to sites
    int read(const std::string& transactionName, const std::string& variableName, long timestamp);
    void write(std::shared_ptr<Transaction> transaction, const std::string& variableName, int value, long commitTime);

    // Methods to handle site failures and recoveries
    void failSite(int siteId);
    void recoverSite(int siteId);

private:
    std::map<int, std::shared_ptr<Site>> sites;
    
    // Structure to track waiting transactions
    struct WaitingRead {
        std::string transactionName;
        std::string variableName;
        long timestamp;
    };
    std::vector<WaitingRead> waitingReads;

    // Helper method to check if a site has stable history
    bool hasSiteStableHistory(std::shared_ptr<Site> site, long timestamp) const;
    bool hasContinuousHistory(std::shared_ptr<Site> site, long fromTime, long toTime) const;
};

#endif // DATA_MANAGER_H