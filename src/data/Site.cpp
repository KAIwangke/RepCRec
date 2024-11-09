#include "Site.h"
#include <iostream>
#include <chrono>

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
bool Site::hasCommittedWrite(const std::string& variableName, long startTime) const {  
    auto it = variables.find(variableName);
    if (it != variables.end()) {
        return it->second.wasModifiedAfter(startTime);
    }
    return false;
}

bool Site::hasVariable(const std::string& variableName) const {
    return variables.find(variableName) != variables.end();
}

int Site::readVariable(const std::string& variableName, long timestamp) {
    std::lock_guard<std::mutex> lock(siteMutex);
    return variables[variableName].readValue(timestamp);
}

void Site::writeVariable(const std::string& variableName, int value, long commitTime) {
    std::lock_guard<std::mutex> lock(siteMutex);
    variables[variableName].writeValue(value, commitTime);
}

void Site::fail() {
    std::lock_guard<std::mutex> lock(siteMutex);
    status = SiteStatus::DOWN;
}

void Site::dump() const {
    std::cout << "=== Site " << id << " ===\n";
    if (status == SiteStatus::DOWN) {
        std::cout << "This site is down.\n";
        return;
    }
    
    for (std::map<std::string, Variable>::const_iterator it = variables.begin(); 
         it != variables.end(); ++it) {
        const std::string& varName = it->first;
        const Variable& variable = it->second;
        int value = variable.readValue(std::chrono::system_clock::now().time_since_epoch().count());
        std::cout << varName << ": " << value << " at site " << id << "\n";
    }
}

void Site::recover() {
    std::lock_guard<std::mutex> lock(siteMutex);
    status = SiteStatus::RECOVERING;
}

void Site::initializeVariables() {
    // Initialize variables based on site ID and replication rules
}