#include "TransactionManager.h"
#include "CommandParser.h"
#include <iostream>

TransactionManager::TransactionManager(std::shared_ptr<DataManager> dm)
    : dataManager(dm) {}

void TransactionManager::processCommand(const std::string& command) {
    // Parse and process command (use CommandParser if necessary)
}

void TransactionManager::beginTransaction(const std::string& transactionName, bool isReadOnly) {
    auto transaction = std::make_shared<Transaction>(transactionName, isReadOnly);
    transactions[transactionName] = transaction;
    std::cout << "Transaction " << transactionName << " started.\n";
}

void TransactionManager::read(const std::string& transactionName, const std::string& variableName) {
    // Implement SSI read logic
}

void TransactionManager::write(const std::string& transactionName, const std::string& variableName, int value) {
    // Implement SSI write logic
}

void TransactionManager::endTransaction(const std::string& transactionName) {
    auto it = transactions.find(transactionName);
    if (it != transactions.end()) {
        auto transaction = it->second;
        validateAndCommit(transaction);
    }
}

void TransactionManager::validateAndCommit(std::shared_ptr<Transaction> transaction) {
    // Implement validation logic for SSI
    // Commit or abort the transaction
    if (transaction->isReadOnly()) {
        transaction->setStatus(TransactionStatus::COMMITTED);
        std::cout << transaction->getName() << " committed (Read-Only).\n";
        return;
    }

    // Check for conflicts
    bool hasConflict = false;
    long startTime = transaction->getStartTime();

    for (const auto& [variableName, value] : transaction->getWriteSet()) {
        if (dataManager->hasCommittedWrite(variableName, startTime)) {
            hasConflict = true;
            break;
        }
    }

    if (hasConflict) {
        abortTransaction(transaction);
    } else {
        // Commit the transaction
        dataManager->commitTransaction(transaction);
        transaction->setStatus(TransactionStatus::COMMITTED);
        std::cout << transaction->getName() << " committed.\n";
    }    
}

void TransactionManager::abortTransaction(std::shared_ptr<Transaction> transaction) {
    transaction->setStatus(TransactionStatus::ABORTED);
    std::cout << "Transaction " << transaction->getName() << " aborted.\n";
}

