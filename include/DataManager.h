#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include "Site.h"

class DataManager {
public:
    DataManager();
    void initializeSites();

    std::shared_ptr<Site> getSite(int siteId);
    std::vector<std::shared_ptr<Site>> getAllSites();

    // Methods to route read/write requests to sites
    int read(const std::string& transactionName, const std::string& variableName, long timestamp);
    void write(const std::string& transactionName, const std::string& variableName, int value, long commitTime);

    // Methods to handle site failures and recoveries
    void failSite(int siteId);
    void recoverSite(int siteId);

private:
    std::map<int, std::shared_ptr<Site>> sites;
};

#endif // DATA_MANAGER_H
