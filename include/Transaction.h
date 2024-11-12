#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <set>
#include <map>
#include <chrono>

enum class TransactionStatus {
    ACTIVE,
    COMMITTED,
    ABORTED
};

class Transaction {
public:
    Transaction(const std::string& name, bool isReadOnly);

    std::string getName() const;
    bool isReadOnly() const;
    TransactionStatus getStatus() const;
    void setStatus(TransactionStatus status);
    long getStartTime() const;
    void setCommitTime(long time);  

    void addReadVariable(const std::string& variableName);
    void addWriteVariable(const std::string& variableName, int value);

    const std::set<std::string>& getReadSet() const;
    const std::map<std::string, int>& getWriteSet() const;
    long getCommitTime() const;

private:
    std::string name;
    bool readOnly;
    TransactionStatus status;
    long startTime; // Use timestamp for SSI
    long commitTime;

    std::set<std::string> readSet;
    std::map<std::string, int> writeSet;
};

#endif // TRANSACTION_H
