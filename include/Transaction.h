// Transaction.h
#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <set>
#include <map>
#include <unordered_set>
#include <vector>

enum class TransactionStatus
{
    ACTIVE,
    COMMITTED,
    ABORTED
};

class Transaction
{
public:
    Transaction(const std::string &name, bool isReadOnly, long startTime);

    std::string getName() const;
    bool isReadOnly() const;
    TransactionStatus getStatus() const;
    void setStatus(TransactionStatus status);
    long getStartTime() const;
    void setCommitTime(long time);

    void addReadVariable(const std::string &variableName);
    void addWriteVariable(const std::string &variableName, int value);

    const std::set<std::string> &getReadSet() const;
    const std::map<std::string, int> &getWriteSet() const;
    long getCommitTime() const;
    void addSitesWritten(const std::vector<int> &siteIds);
    const std::unordered_set<int> &getSitesWrittenTo() const;

private:
    std::string name;
    bool readOnly;
    TransactionStatus status;
    long startTime; // Logical timestamp
    long commitTime;

    std::set<std::string> readSet;
    std::map<std::string, int> writeSet;
    std::unordered_set<int> sitesWrittenTo;
};

#endif // TRANSACTION_H
