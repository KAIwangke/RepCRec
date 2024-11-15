#ifndef SITE_H
#define SITE_H

#include <string>
#include <map>
#include <mutex>
#include <unordered_set>
#include "Variable.h"

enum class SiteStatus
{
    UP,
    DOWN,
    RECOVERING
};

class Site
{
public:
    Site(int id);
    int getId() const;
    SiteStatus getStatus() const;
    void setStatus(SiteStatus status);

    int readVariable(const std::string &variableName, long timestamp);
    void writeVariable(const std::string &variableName, int value, long commitTime);

    void fail();
    void recover();
    bool hasVariable(const std::string &variableName) const;
    bool hasCommittedWrite(const std::string &variableName, long startTime) const;
    void dump() const;
    const std::vector<std::pair<long, long>> &getFailureTimes() const;

private:
    int id;
    SiteStatus status;
    std::mutex siteMutex;
    std::map<std::string, Variable> variables;
    std::unordered_set<std::string> unavailableVariables;
    void initializeVariables();
    std::vector<std::pair<long, long>> failureTimes;
};

#endif // SITE_H
