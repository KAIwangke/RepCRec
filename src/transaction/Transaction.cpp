/**
 * @   Author: Ke Wang & Siwen Tao
 * @   Email: kw3484@nyu.edu & st5297@nyu.edu
 * @   Modified time: 2024-12-08 22:18:40
 */

#include "Transaction.h"
#include <chrono>
using namespace std;

// Description: Creates a new transaction with given name and read-only status
// Input: name (string) - transaction identifier, isReadOnly (bool) - read-only flag
// Output: None
// Side Effects: Initializes transaction with ACTIVE status and current timestamp
Transaction::Transaction(const string &name, bool isReadOnly)
    : name(name),
      readOnly(isReadOnly),
      status(TransactionStatus::ACTIVE),
      startTime(chrono::system_clock::now().time_since_epoch().count()),
      commitTime(0) {}

// Description: Returns transaction identifier
// Input: None
// Output: string - transaction name
// Side Effects: None
string Transaction::getName() const { return name; }

// Description: Checks if transaction is read-only
// Input: None
// Output: bool - true if read-only
// Side Effects: None
bool Transaction::isReadOnly() const { return readOnly; }

// Description: Gets current transaction status
// Input: None
// Output: TransactionStatus - current status
// Side Effects: None
TransactionStatus Transaction::getStatus() const { return status; }

// Description: Updates transaction status and sets commit time if committed
// Input: newStatus (TransactionStatus) - new status to set
// Output: None
// Side Effects: Updates status and possibly commit time
void Transaction::setStatus(TransactionStatus newStatus)
{
    status = newStatus;
    if (newStatus == TransactionStatus::COMMITTED)
    {
        commitTime = chrono::system_clock::now().time_since_epoch().count();
    }
}

// Description: Returns transaction start timestamp
// Input: None
// Output: long - start time
// Side Effects: None
long Transaction::getStartTime() const { return startTime; }

// Description: Adds variable to read set
// Input: variableName (string) - variable being read
// Output: None
// Side Effects: Updates read set
void Transaction::addReadVariable(const string &variableName) { readSet.insert(variableName); }

// Description: Records write operation for later commit
// Input: variableName (string) - variable to write, value (int) - new value
// Output: None
// Side Effects: Updates write set
void Transaction::addWriteVariable(const string &variableName, int value) { writeSet[variableName] = value; }

// Description: Returns set of variables read by transaction
// Input: None
// Output: const set<string>& - read variable set
// Side Effects: None
const set<string> &Transaction::getReadSet() const { return readSet; }

// Description: Returns map of variables and values to be written
// Input: None
// Output: const map<string, int>& - write variable map
// Side Effects: None
const map<string, int> &Transaction::getWriteSet() const { return writeSet; }

// Description: Sets transaction commit timestamp
// Input: time (long) - commit timestamp
// Output: None
// Side Effects: Updates commit time
void Transaction::setCommitTime(long time) { commitTime = time; }

// Description: Returns transaction commit timestamp
// Input: None
// Output: long - commit time
// Side Effects: None
long Transaction::getCommitTime() const { return commitTime; }

// Description: Records sites that will be written to
// Input: siteIds (vector<int>) - list of site IDs
// Output: None
// Side Effects: Updates set of sites written to
void Transaction::addSitesWritten(const vector<int> &siteIds)
{
    for (int siteId : siteIds)
    {
        sitesWrittenTo.insert(siteId);
    }
}

// Description: Returns set of sites written to by transaction
// Input: None
// Output: const unordered_set<int>& - set of site IDs
// Side Effects: None
const unordered_set<int> &Transaction::getSitesWrittenTo() const
{
    return sitesWrittenTo;
}

// Description: Adds dependency on another transaction
// Input: transactionName (string) - name of dependency
// Output: None
// Side Effects: Updates dependency set
void Transaction::addDependency(const string &transactionName)
{
    dependencySet.insert(transactionName);
}

// Description: Returns set of transaction dependencies
// Input: None
// Output: const set<string>& - set of dependent transaction names
// Side Effects: None
const set<string> &Transaction::getDependencies() const
{
    return dependencySet;
}