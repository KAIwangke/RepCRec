/**
 * @   Author: Ke Wang & Siwen Tao
 * @   Email: kw3484@nyu.edu & st5297@nyu.edu
 * @   Modified time: 2024-12-08 22:16:12
 */

// Manages distributed database sites and coordinates data access across sites. Handles data
// replication, site failures/recoveries, and transaction read/write operations.
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
 // Initialize data manager with empty sites
DataManager();
 // Create and initialize all database sites
void initializeSites();
 // Get site instance by ID
 std::shared_ptr<Site> getSite(int siteId);
 // Get list of all database sites
 std::vector<std::shared_ptr<Site>> getAllSites();
 // Check if variable has committed write since given time
bool hasCommittedWrite(const std::string& variableName, long startTime);
 // Commit transaction's writes across all relevant sites
void commitTransaction(std::shared_ptr<Transaction> transaction);
 // Print current state of all sites
void dump();
 // Read variable value from appropriate site
int read(const std::string& transactionName, const std::string& variableName, long timestamp);
 // Write value to variable across all available sites
void write(std::shared_ptr<Transaction> transaction, const std::string& variableName, int value, long commitTime);
 // Mark site as failed
void failSite(int siteId);
 // Restore failed site
void recoverSite(int siteId);
private:
 std::map<int, std::shared_ptr<Site>> sites;
struct WaitingRead {
 std::string transactionName;
 std::string variableName;
long timestamp;
 };
 std::vector<WaitingRead> waitingReads;
 // Check if site has consistent history from given timestamp
bool hasSiteStableHistory(std::shared_ptr<Site> site, long timestamp) const;
 // Verify site was up continuously between time points
bool hasContinuousHistory(std::shared_ptr<Site> site, long fromTime, long toTime) const;
};
#endif // DATA_MANAGER_H