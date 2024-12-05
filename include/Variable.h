/**
 * @ Author: Ke Wang & Siwen Tao
 * @ Email: kw3484@nyu.edu & st5297@nyu.edu
 * @ Create Time: 2024-11-26 20:07:56
 * @ Modified time: 2024-12-05 21:33:07
 */

#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>
#include <vector>

struct Version {
    int value;
    long commitTime;
};

class Variable {
public:
    Variable(); // Default constructor
    Variable(const std::string& name, int initialValue);

    std::string getName() const;
    int readValue(long timestamp) const;
    void writeValue(int value, long commitTime);
    bool wasModifiedAfter(long timestamp) const;

private:
    std::string name;
    std::vector<Version> versions;
};

#endif // VARIABLE_H
