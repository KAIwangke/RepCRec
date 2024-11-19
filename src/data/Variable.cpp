// Variable.cpp
#include "Variable.h"

Variable::Variable()
    : name(""), versions({{0, 0}}) {}

Variable::Variable(const std::string& name, int initialValue)
    : name(name) {
    versions.push_back({initialValue, 0}); // Initial version at time 0
}

std::string Variable::getName() const {
    return name;
}

int Variable::readValue(long timestamp) const {
    // Return value of latest version before or at timestamp
    for (auto it = versions.rbegin(); it != versions.rend(); ++it) {
        if (it->commitTime <= timestamp) {
            return it->value;
        }
    }
    // If no appropriate version found, return initial value
    return versions.front().value;
}

bool Variable::wasModifiedAfter(long timestamp) const {
    for (const auto& version : versions) {
        if (version.commitTime > timestamp) {
            return true;
        }
    }
    return false;
}

void Variable::writeValue(int value, long commitTime) {
    versions.push_back({value, commitTime});
}
