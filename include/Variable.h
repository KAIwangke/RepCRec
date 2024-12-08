/**
 * @   Author: Ke Wang & Siwen Tao
 * @   Email: kw3484@nyu.edu & st5297@nyu.edu
 * @   Modified time: 2024-12-08 22:17:03
 */

// Implements multiversion concurrency control for database variables by maintaining
// a version history of values and their commit timestamps
#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>
#include <vector>

// Stores a single version of a variable's value and its commit timestamp
struct Version {
    int value;       // The stored value
    long commitTime; // When this version was committed
};

class Variable {
public:
    // Creates an uninitialized variable
    Variable();

    // Creates a variable with initial value and name
    Variable(const std::string& name, int initialValue);

    // Returns the variable's identifier
    std::string getName() const;

    // Retrieves appropriate version value based on timestamp
    int readValue(long timestamp) const;

    // Creates a new version with given value and commit time
    void writeValue(int value, long commitTime);

    // Checks if variable was modified after given timestamp
    bool wasModifiedAfter(long timestamp) const;

private:
    std::string name;             // Variable identifier
    std::vector<Version> versions; // History of variable versions
};

#endif // VARIABLE_H