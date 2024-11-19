// Transaction.cpp
#include "Transaction.h"

Transaction::Transaction(const std::string &name, bool isReadOnly, long startTime)
    : name(name),
      readOnly(isReadOnly),
      status(TransactionStatus::ACTIVE),
      startTime(startTime),
      commitTime(0) {}

std::string Transaction::getName() const { return name; }

bool Transaction::isReadOnly() const { return readOnly; }

TransactionStatus Transaction::getStatus() const { return status; }

void Transaction::setStatus(TransactionStatus newStatus)
{
    status = newStatus;
}

long Transaction::getStartTime() const { return startTime; }

void Transaction::addReadVariable(const std::string &variableName) { readSet.insert(variableName); }

void Transaction::addWriteVariable(const std::string &variableName, int value) { writeSet[variableName] = value; }

const std::set<std::string> &Transaction::getReadSet() const { return readSet; }

const std::map<std::string, int> &Transaction::getWriteSet() const { return writeSet; }

void Transaction::setCommitTime(long time) { commitTime = time; }

long Transaction::getCommitTime() const { return commitTime; }

void Transaction::addSitesWritten(const std::vector<int> &siteIds)
{
    for (int siteId : siteIds)
    {
        sitesWrittenTo.insert(siteId);
    }
}

const std::unordered_set<int> &Transaction::getSitesWrittenTo() const
{
    return sitesWrittenTo;
}
