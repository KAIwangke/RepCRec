#include "Transaction.h"

#include <chrono>

Transaction::Transaction(const std::string& name, bool isReadOnly)
    : name(name),
      readOnly(isReadOnly),
      status(TransactionStatus::ACTIVE),
      startTime(std::chrono::system_clock::now().time_since_epoch().count()),
      commitTime(0) {}   // Initialize commitTime

std::string Transaction::getName() const { return name; }

bool Transaction::isReadOnly() const { return readOnly; }

TransactionStatus Transaction::getStatus() const { return status; }

void Transaction::setStatus(TransactionStatus newStatus) {
    status = newStatus;
    if (newStatus == TransactionStatus::COMMITTED) {
        commitTime = std::chrono::system_clock::now().time_since_epoch().count();
    }
}

long Transaction::getStartTime() const { return startTime; }

void Transaction::addReadVariable(const std::string& variableName) { readSet.insert(variableName); }

void Transaction::addWriteVariable(const std::string& variableName, int value) { writeSet[variableName] = value; }

const std::set<std::string>& Transaction::getReadSet() const { return readSet; }

const std::map<std::string, int>& Transaction::getWriteSet() const { return writeSet; }

// In src/transaction/Transaction.cpp, add these implementations:
void Transaction::setCommitTime(long time) { commitTime = time; }

long Transaction::getCommitTime() const { return commitTime; }