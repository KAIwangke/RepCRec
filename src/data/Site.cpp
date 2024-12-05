#include "Site.h"
#include <iostream>
#include <chrono>
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

// Fixed: Added const qualifier to match the header declaration
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

int Site::readVariable(const std::string &variableName, long timestamp) {
    std::lock_guard<std::mutex> lock(siteMutex);

    if (status == SiteStatus::DOWN) {
        throw std::runtime_error("Site is down.");
    }

    // Allow reading even if variable is unavailable if we have a version before timestamp
    auto it = variables.find(variableName);
    if (it != variables.end()) {
        return it->second.readValue(timestamp);
    }

    throw std::runtime_error("Variable " + variableName + " not found");
}

void Site::writeVariable(const std::string &variableName, int value, long commitTime)
{
    std::lock_guard<std::mutex> lock(siteMutex);
    variables[variableName].writeValue(value, commitTime);

    // If variable was unavailable for reading, now it becomes available
    unavailableVariables.erase(variableName);
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

    // Check for and print modified odd variables at this site
    for (const auto &pair : variables)
    {
        string varName = pair.first;
        int varIndex = stoi(varName.substr(1));
        if (varIndex % 2 == 1)
        { // odd variable
            int value = pair.second.readValue(chrono::system_clock::now().time_since_epoch().count());
            int initialValue = varIndex * 10;
            if (value != initialValue)
            {
                cout << varName << ": " << value << endl;
                hasModifiedVars = true;
            }
        }
    }

    // Check for and print modified even variables
    for (const auto &pair : variables)
    {
        string varName = pair.first;
        int varIndex = stoi(varName.substr(1));
        if (varIndex % 2 == 0)
        { // even variable
            int value = pair.second.readValue(chrono::system_clock::now().time_since_epoch().count());
            int initialValue = varIndex * 10;
            if (value != initialValue)
            {
                cout << varName << ": " << value << " at all sites" << endl;
                hasModifiedVars = true;
                break; // Only need to show once for replicated variables
            }
        }
    }

    if (!hasModifiedVars)
    {
        cout << "All variables have their initial values" << endl;
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


void Site::fail() {
    std::unique_lock<std::mutex> lock(siteMutex);
    if (status == SiteStatus::DOWN) {
        // Already down, don't do anything
        return;
    }
    status = SiteStatus::DOWN;
    long failTime = std::chrono::system_clock::now().time_since_epoch().count();
    failureTimes.emplace_back(failTime, -1);
    unavailableVariables.clear();
}

void Site::recover() {
    std::unique_lock<std::mutex> lock(siteMutex);
    if (status != SiteStatus::DOWN) {
        // Not down, can't recover
        return;
    }
    status = SiteStatus::RECOVERING;
    long recoverTime = std::chrono::system_clock::now().time_since_epoch().count();
    if (!failureTimes.empty() && failureTimes.back().second == -1) {
        failureTimes.back().second = recoverTime;
    }

    // Mark replicated variables as unavailable until a new write
    for (const auto &varPair : variables) {
        int varIndex = stoi(varPair.first.substr(1));
        if (varIndex % 2 == 0) {
            unavailableVariables.insert(varPair.first);
        }
    }
}