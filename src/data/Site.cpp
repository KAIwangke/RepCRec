// Site.cpp
#include "Site.h"
#include "TransactionManager.h" // To access globalTimestamp
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;


Site::Site(int id) : id(id), status(SiteStatus::UP)
{
    initializeVariables();
}

int Site::getId() const
{
    return id;
}

SiteStatus Site::getStatus() const
{
    return status;
}

void Site::setStatus(SiteStatus status)
{
    this->status = status;
}

bool Site::hasCommittedWrite(const string &variableName, long startTime) const
{
    auto it = variables.find(variableName);
    if (it != variables.end())
    {
        return it->second.wasModifiedAfter(startTime);
    }
    return false;
}

bool Site::hasVariable(const string &variableName) const
{
    return variables.find(variableName) != variables.end();
}

int Site::readVariable(const std::string &variableName, long timestamp)
{
    std::lock_guard<std::mutex> lock(siteMutex);

    if (status == SiteStatus::DOWN)
    {
        throw std::runtime_error("Site is down.");
    }

    // If variable is unavailable for reading, throw exception
    if (unavailableVariables.find(variableName) != unavailableVariables.end())
    {
        throw std::runtime_error("Variable " + variableName + " is unavailable for reading at site " + std::to_string(id));
    }

    return variables[variableName].readValue(timestamp);
}

void Site::writeVariable(const std::string &variableName, int value, long commitTime)
{
    std::lock_guard<std::mutex> lock(siteMutex);
    variables[variableName].writeValue(value, commitTime);

    // If variable was unavailable for reading, now it becomes available
    unavailableVariables.erase(variableName);
}

void Site::fail()
{
    std::lock_guard<std::mutex> lock(siteMutex);
    status = SiteStatus::DOWN;
    long failTime = ++TransactionManager::globalTimestamp;
    failureTimes.emplace_back(failTime, -1); // -1 indicates not yet recovered
    unavailableVariables.clear();
}

void Site::dump() const
{
    cout << "=== Site " << id << " ===" << endl;
    if (status == SiteStatus::DOWN)
    {
        cout << "Site " << id << " is down" << endl;
        return;
    }

    bool hasModifiedVars = false;

    // Check for and print modified variables at this site
    for (const auto &pair : variables)
    {
        string varName = pair.first;
        int value = pair.second.readValue(TransactionManager::globalTimestamp.load());
        int initialValue = stoi(varName.substr(1)) * 10;
        if (value != initialValue)
        {
            cout << varName << ": " << value << endl;
            hasModifiedVars = true;
        }
    }

    if (!hasModifiedVars)
    {
        cout << "All variables have their initial values" << endl;
    }
}

void Site::recover()
{
    std::lock_guard<std::mutex> lock(siteMutex);
    status = SiteStatus::RECOVERING;
    long recoverTime = ++TransactionManager::globalTimestamp;
    if (!failureTimes.empty() && failureTimes.back().second == -1)
    {
        failureTimes.back().second = recoverTime;
    }

    // Mark even variables as unavailable
    unavailableVariables.clear();
    for (const auto &varPair : variables)
    {
        int varIndex = stoi(varPair.first.substr(1));
        if (varIndex % 2 == 0) // Even variables
        {
            unavailableVariables.insert(varPair.first);
        }
    }
}

void Site::initializeVariables()
{
    for (int i = 1; i <= 20; ++i)
    {
        std::string varName = "x" + std::to_string(i);
        int initialValue = i * 10;
        if (i % 2 == 0)
        {
            // Even-indexed variable, replicated at all sites
            variables[varName] = Variable(varName, initialValue);
        }
        else
        {
            // Odd-indexed variable, stored at one site
            int assignedSite = 1 + (i % 10);
            if (assignedSite == id)
            {
                variables[varName] = Variable(varName, initialValue);
            }
        }
    }
}

const std::vector<std::pair<long, long>> &Site::getFailureTimes() const
{
    return failureTimes;
}
