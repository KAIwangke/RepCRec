#include "Site.h"

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

bool Site::hasCommittedWrite(const std::string& variableName, long startTime) {
    if (variables.find(variableName) != variables.end()) {
        return variables[variableName].wasModifiedAfter(startTime);
    }
    return false;
}

int Site::readVariable(const std::string& variableName, long timestamp) {
    std::lock_guard<std::mutex> lock(siteMutex);
    // Implement SSI read logic
    return variables[variableName].readValue(timestamp);
}

void Site::writeVariable(const std::string& variableName, int value, long commitTime) {
    std::lock_guard<std::mutex> lock(siteMutex);
    // Implement write logic
    variables[variableName].writeValue(value, commitTime);
}

void Site::fail() {
    std::lock_guard<std::mutex> lock(siteMutex);
    status = SiteStatus::DOWN;
    // Additional failure handling
}

void Site::dump() {
    std::cout << "=== Site " << id << " ===\n";
    if (status == SiteStatus::DOWN) {
        std::cout << "This site is down.\n";
        return;
    }
    for (const auto& [varName, variable] : variables) {
        int value = variable.getLatestValue();
        std::cout << varName << ": " << value << " at site " << id << "\n";
    }
}


void Site::recover() {
    std::lock_guard<std::mutex> lock(siteMutex);
    status = SiteStatus::RECOVERING;
    // Additional recovery handling
}

void Site::initializeVariables() {
    // Initialize variables based on site ID and replication rules
}

