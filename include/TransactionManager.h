#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include <string>
#include <map>
#include <memory>
#include "Transaction.h"
#include "DataManager.h"

class TransactionManager {
public:
    TransactionManager(std::shared_ptr<DataManager> dm);
    void processCommand(const std::string& command);

    void beginTransaction(const std::string& transactionName, bool isReadOnly);
    void read(const std::string& transactionName, const std::string& variableName);
    void write(const std::string& transactionName, const std::string& variableName, int value);
    void endTransaction(const std::string& transactionName);
    void dump() const;

private:
    std::map<std::string, std::shared_ptr<Transaction>> transactions;
    std::shared_ptr<DataManager> dataManager; // Interface to Data Managers

    // Helper methods
    void validateAndCommit(std::shared_ptr<Transaction> transaction);
    void abortTransaction(std::shared_ptr<Transaction> transaction);
};

#endif // TRANSACTION_MANAGER_H
