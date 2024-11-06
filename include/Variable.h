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

private:
    std::string name;
    std::vector<Version> versions;
};

#endif // VARIABLE_H
