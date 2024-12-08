/**
 * @ Author: Ke Wang & Siwen Tao
 * @ Email: kw3484@nyu.edu & st5297@nyu.edu
 * @ Create Time: 2024-11-28 21:40:54
 * @ Modified time: 2024-12-05 21:33:11
 */

#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include <string>
#include <map>
#include <memory>
#include "Transaction.h"
#include "DataManager.h"

class TransactionManager
{
public:
    TransactionManager(std::shared_ptr<DataManager> dm);
    void processCommand(const std::string &command);

    void beginTransaction(const std::string &transactionName, bool isReadOnly);
    void read(const std::string &transactionName, const std::string &variableName);
    void write(const std::string &transactionName, const std::string &variableName, int value);
    void endTransaction(const std::string &transactionName);
    void dump() const;
    void failSite(int siteId);
    void recoverSite(int siteId);

private:
    std::map<std::string, std::shared_ptr<Transaction>> transactions;
    std::shared_ptr<DataManager> dataManager; // Interface to Data Managers

    std::map<std::string, std::set<std::string>> readTable; // variableName -> transactions that read it
    std::map<std::string, std::set<std::string>> writeTable;

    std::map<std::string, std::string> lastWriter;

    // Helper methods
    void validateAndCommit(std::shared_ptr<Transaction> transaction);
    void abortTransaction(std::shared_ptr<Transaction> transaction);

    bool detectCycle(const std::string &transactionName);
    bool dfs(const std::string &transactionName, std::set<std::string> &visited, std::set<std::string> &recursionStack);
};

#endif // TRANSACTION_MANAGER_H
