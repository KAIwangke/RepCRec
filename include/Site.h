#ifndef SITE_H
#define SITE_H

#include <string>
#include <map>
#include <mutex>
#include "Variable.h"

enum class SiteStatus {
    UP,
    DOWN,
    RECOVERING
};

class Site {
public:
    Site(int id);
    int getId() const;
    SiteStatus getStatus() const;
    void setStatus(SiteStatus status);

    int readVariable(const std::string& variableName, long timestamp);
    void writeVariable(const std::string& variableName, int value, long commitTime);

    void fail();
    void recover();

private:
    int id;
    SiteStatus status;
    std::mutex siteMutex;
    std::map<std::string, Variable> variables;

    void initializeVariables();
};

#endif // SITE_H
