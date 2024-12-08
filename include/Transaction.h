/**
 * @   Author: Ke Wang & Siwen Tao
 * @   Email: kw3484@nyu.edu & st5297@nyu.edu
 * @   Modified time: 2024-12-08 22:15:39
 */

// Manages individual transaction state and operations in the distributed database system
#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <set>
#include <map>
#include <chrono>
#include <unordered_set>
#include <vector>

// Represents the lifecycle states of a transaction
enum class TransactionStatus
{
    ACTIVE,    // Currently executing
    COMMITTED, // Successfully completed
    ABORTED    // Rolled back due to conflict or error
};

class Transaction
{
public:
    // Creates a new transaction with specified name and read-only status
    Transaction(const std::string &name, bool isReadOnly);

    // Returns the transaction's unique identifier name
    std::string getName() const;

    // Checks if this transaction is read-only
    bool isReadOnly() const;

    // Returns current status of the transaction
    TransactionStatus getStatus() const;

    // Updates the transaction's current status
    void setStatus(TransactionStatus status);

    // Returns the timestamp when this transaction started
    long getStartTime() const;

    // Sets the timestamp when this transaction committed
    void setCommitTime(long time);

    // Registers a variable as being read by this transaction
    void addReadVariable(const std::string &variableName);

    // Records a write operation and its value for later commitment
    void addWriteVariable(const std::string &variableName, int value);

    // Returns the set of all variables read by this transaction
    const std::set<std::string> &getReadSet() const;

    // Returns the map of variables and their values to be written
    const std::map<std::string, int> &getWriteSet() const;

    // Returns the timestamp when this transaction was committed
    long getCommitTime() const;

    // Records the database sites that will be modified by this transaction
    void addSitesWritten(const std::vector<int> &siteIds);

    // Returns the set of site IDs this transaction has written to
    const std::unordered_set<int> &getSitesWrittenTo() const;

    // Adds a dependency on another transaction for serialization ordering
    void addDependency(const std::string &transactionName);

    // Returns the set of transactions this transaction depends on
    const std::set<std::string> &getDependencies() const;

private:
    std::string name;              // Unique identifier for the transaction
    bool readOnly;                 // Whether this is a read-only transaction
    TransactionStatus status;      // Current state of the transaction
    long startTime;               // Transaction start timestamp for SSI
    long commitTime;              // When transaction was committed
    std::set<std::string> dependencySet;  // Other transactions this one depends on
    std::set<std::string> readSet;        // Variables read by this transaction
    std::map<std::string, int> writeSet;  // Variables and values to be written
    std::unordered_set<int> sitesWrittenTo; // Sites modified by this transaction
};

#endif // TRANSACTION_H