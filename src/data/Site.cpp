/**
 * @   Author: Ke Wang & Siwen Tao
 * @   Email: kw3484@nyu.edu & st5297@nyu.edu
 * @   Modified time: 2024-12-08 22:17:55
 */

#include "Site.h"
#include <iostream>
#include <chrono>
#include <string>
#include <algorithm>
using namespace std;

// Description: Constructs a new database site with given ID
// Input: id (int) - unique identifier for the site
// Output: None
// Side Effects: Initializes variables for this site
Site::Site(int id) : id(id), status(SiteStatus::UP)
{
    initializeVariables();
}

// Description: Returns the site's unique identifier
// Input: None
// Output: int - site ID
// Side Effects: None
int Site::getId() const
{
    return id;
}

// Description: Returns current operational status of the site
// Input: None
// Output: SiteStatus enum value
// Side Effects: None
SiteStatus Site::getStatus() const
{
    return status;
}

// Description: Updates the site's operational status
// Input: status (SiteStatus) - new status to set
// Output: None
// Side Effects: Changes site's operational state
void Site::setStatus(SiteStatus status)
{
    this->status = status;
}

// Description: Checks if variable has been modified after given time
// Input: variableName (string), startTime (long)
// Output: bool - true if modified after startTime
// Side Effects: None
bool Site::hasCommittedWrite(const string &variableName, long startTime) const
{
    auto it = variables.find(variableName);
    if (it != variables.end())
    {
        return it->second.wasModifiedAfter(startTime);
    }
    return false;
}

// Description: Checks if site stores given variable
// Input: variableName (string)
// Output: bool - true if variable exists at this site
// Side Effects: None
bool Site::hasVariable(const string &variableName) const
{
    return variables.find(variableName) != variables.end();
}

// Description: Reads value of variable at specific timestamp
// Input: variableName (string), timestamp (long)
// Output: int - variable value
// Side Effects: Throws exception if site down or variable not found
int Site::readVariable(const std::string &variableName, long timestamp) {
    std::lock_guard<std::mutex> lock(siteMutex);

    if (status == SiteStatus::DOWN) {
        throw std::runtime_error("Site is down.");
    }

    auto it = variables.find(variableName);
    if (it != variables.end()) {
        return it->second.readValue(timestamp);
    }

    throw std::runtime_error("Variable " + variableName + " not found");
}

// Description: Updates variable value with commit timestamp
// Input: variableName (string), value (int), commitTime (long)
// Output: None
// Side Effects: Updates variable value, removes from unavailable list
void Site::writeVariable(const std::string &variableName, int value, long commitTime)
{
    std::lock_guard<std::mutex> lock(siteMutex);
    variables[variableName].writeValue(value, commitTime);
    unavailableVariables.erase(variableName);
}

// Description: Displays current state of all variables at this site
// Input: None
// Output: None
// Side Effects: Prints site status and variable values to console
void Site::dump() const
{
    cout << "=== Site " << id << " ===" << endl;
    if (status == SiteStatus::DOWN)
    {
        cout << "Site " << id << " is down" << endl;
        return;
    }

    bool hasModifiedVars = false;

    // Check odd variables
    for (const auto &pair : variables)
    {
        string varName = pair.first;
        int varIndex = stoi(varName.substr(1));
        if (varIndex % 2 == 1)
        {
            int value = pair.second.readValue(chrono::system_clock::now().time_since_epoch().count());
            int initialValue = varIndex * 10;
            if (value != initialValue)
            {
                cout << varName << ": " << value << endl;
                hasModifiedVars = true;
            }
        }
    }

    // Check even variables
    for (const auto &pair : variables)
    {
        string varName = pair.first;
        int varIndex = stoi(varName.substr(1));
        if (varIndex % 2 == 0)
        {
            int value = pair.second.readValue(chrono::system_clock::now().time_since_epoch().count());
            int initialValue = varIndex * 10;
            if (value != initialValue)
            {
                cout << varName << ": " << value << " at all sites" << endl;
                hasModifiedVars = true;
                break;
            }
        }
    }

    if (!hasModifiedVars)
    {
        cout << "All variables have their initial values" << endl;
    }
}

// Description: Sets up initial variables for this site
// Input: None
// Output: None
// Side Effects: Creates and initializes variables based on site ID
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

// Description: Returns history of site failures
// Input: None
// Output: Vector of failure time pairs (start, end)
// Side Effects: None
const std::vector<std::pair<long, long>> &Site::getFailureTimes() const
{
    return failureTimes;
}

// Description: Simulates site failure
// Input: None
// Output: None
// Side Effects: Changes status to DOWN, records failure time, clears unavailable variables
void Site::fail() {
    std::unique_lock<std::mutex> lock(siteMutex);
    if (status == SiteStatus::DOWN) {
        return;
    }
    status = SiteStatus::DOWN;
    long failTime = std::chrono::system_clock::now().time_since_epoch().count();
    failureTimes.emplace_back(failTime, -1);
    unavailableVariables.clear();
}

// Description: Recovers site from failure
// Input: None
// Output: None
// Side Effects: Updates status, records recovery time, marks replicated variables as unavailable
void Site::recover() {
    std::unique_lock<std::mutex> lock(siteMutex);
    if (status != SiteStatus::DOWN) {
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