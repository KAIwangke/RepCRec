/**
 * @   Author: Ke Wang & Siwen Tao
 * @   Email: kw3484@nyu.edu & st5297@nyu.edu
 * @   Modified time: 2024-12-08 22:18:06
 */

#include "Variable.h"
using namespace std;

// Description: Default constructor for Variable class
// Input: None
// Output: None
// Side Effects: Creates variable with empty name and initial version {0,0}
Variable::Variable()
    : name(""), versions({{0, 0}}) {} 

// Description: Creates named variable with initial value
// Input: name (string), initialValue (int)
// Output: None
// Side Effects: Creates variable with specified name and initial version
Variable::Variable(const string& name, int initialValue)
    : name(name) {
    versions.push_back({initialValue, 0}); // Initial version at time 0
}

// Description: Returns variable's name identifier
// Input: None
// Output: string - name of variable
// Side Effects: None
string Variable::getName() const {
    return name;
}

// Description: Reads appropriate version of variable for given timestamp
// Input: timestamp (long) - time point to read from
// Output: int - value of variable at timestamp
// Side Effects: None
int Variable::readValue(long timestamp) const {
    // Return initial value if no versions exist
    if (versions.empty()) {
        return stoi(name.substr(1)) * 10;
    }
    
    // Return value of latest version before or at timestamp
    for (auto it = versions.rbegin(); it != versions.rend(); ++it) {
        if (it->commitTime <= timestamp) {
            return it->value;
        }
    }
    
    // If no appropriate version found, return initial value
    return stoi(name.substr(1)) * 10;
}

// Description: Checks if variable has been modified after given timestamp
// Input: timestamp (long) - time point to check from
// Output: bool - true if modified after timestamp
// Side Effects: None
bool Variable::wasModifiedAfter(long timestamp) const {
    for (const auto& version : versions) {
        if (version.commitTime > timestamp) {
            return true;
        }
    }
    return false;
}

// Description: Creates new version of variable with value and commit time
// Input: value (int) - new value, commitTime (long) - commit timestamp
// Output: None
// Side Effects: Adds new version to version history
void Variable::writeValue(int value, long commitTime) {
    versions.push_back({value, commitTime});
}