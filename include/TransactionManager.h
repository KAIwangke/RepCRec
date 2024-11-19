// TransactionManager.h
#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include <string>
#include <map>
#include <memory>
#include <atomic>
#include "Transaction.h"
#include "DataManager.h"

class TransactionManager
{
public:
    TransactionManager(std::shared_ptr<DataManager> dm);

    void beginTransaction(const std::string &transactionName, bool isReadOnly);
    void read(const std::string &transactionName, const std::string &variableName);
    void write(const std::string &transactionName, const std::string &variableName, int value);
    void endTransaction(const std::string &transactionName);
    void dump() const;
    void failSite(int siteId);
    void recoverSite(int siteId);
    static std::atomic<long> globalTimestamp;

private:
    std::map<std::string, std::shared_ptr<Transaction>> transactions;
    std::shared_ptr<DataManager> dataManager;

    // Static atomic counter for logical timestamps
    

    // Helper methods
    void validateAndCommit(std::shared_ptr<Transaction> transaction);
    void abortTransaction(std::shared_ptr<Transaction> transaction);
};

#endif // TRANSACTION_MANAGER_H
