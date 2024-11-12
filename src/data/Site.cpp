#include "Site.h"
#include <iostream>
#include <chrono>
#include <string>
#include <algorithm>
using namespace std;


Site::Site(int id) : id(id), status(SiteStatus::UP) {
    initializeVariables();
}

int Site::getId() const {
    return id;
}

SiteStatus Site::getStatus() const {
    return status;
}

void Site::setStatus(SiteStatus status) {
    this->status = status;
}

// Fixed: Added const qualifier to match the header declaration
bool Site::hasCommittedWrite(const string& variableName, long startTime) const {  
    auto it = variables.find(variableName);
    if (it != variables.end()) {
        return it->second.wasModifiedAfter(startTime);
    }
    return false;
}

bool Site::hasVariable(const string& variableName) const {
    return variables.find(variableName) != variables.end();
}

int Site::readVariable(const string& variableName, long timestamp) {
    lock_guard<mutex> lock(siteMutex);
    return variables[variableName].readValue(timestamp);
}

void Site::writeVariable(const string& variableName, int value, long commitTime) {
    lock_guard<mutex> lock(siteMutex);
    variables[variableName].writeValue(value, commitTime);
}

void Site::fail() {
    lock_guard<mutex> lock(siteMutex);
    status = SiteStatus::DOWN;
}

void Site::dump() const {
    cout << "=== Site " << id << " ===" << endl;
    if (status == SiteStatus::DOWN) {
        cout << "Site " << id << " is down" << endl;
        return;
    }
    
    bool hasModifiedVars = false;
    
    // Check for and print modified odd variables at this site
    for (const auto& pair : variables) {
        string varName = pair.first;
        int varIndex = stoi(varName.substr(1));
        if (varIndex % 2 == 1) {  // odd variable
            int value = pair.second.readValue(chrono::system_clock::now().time_since_epoch().count());
            int initialValue = varIndex * 10;
            if (value != initialValue) {
                cout << varName << ": " << value << endl;
                hasModifiedVars = true;
            }
        }
    }
    
    // Check for and print modified even variables
    for (const auto& pair : variables) {
        string varName = pair.first;
        int varIndex = stoi(varName.substr(1));
        if (varIndex % 2 == 0) {  // even variable
            int value = pair.second.readValue(chrono::system_clock::now().time_since_epoch().count());
            int initialValue = varIndex * 10;
            if (value != initialValue) {
                cout << varName << ": " << value << " at all sites" << endl;
                hasModifiedVars = true;
                break;  // Only need to show once for replicated variables
            }
        }
    }
    
    if (!hasModifiedVars) {
        cout << "All variables have their initial values" << endl;
    }
}

void Site::recover() {
    lock_guard<mutex> lock(siteMutex);
    status = SiteStatus::RECOVERING;
}

void Site::initializeVariables() {
    // Initialize variables based on site ID and replication rules

    for (int i = 1; i <= 20; i++) {
        string varName = "x" + to_string(i);
        int initialValue = 10 * i;  // x1=10, x2=20, etc.
        
        if (i % 2 == 0) {  // Even variables - replicated at all sites
            variables[varName] = Variable(varName, initialValue);
        }
        else if ((1 + i % 10) == id) {  // Odd variables - at specific sites
            variables[varName] = Variable(varName, initialValue);
        }
    }
}