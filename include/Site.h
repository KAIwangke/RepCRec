/**
 * @   Author: Ke Wang & Siwen Tao
 * @   Email: kw3484@nyu.edu & st5297@nyu.edu
 * @   Modified time: 2024-12-08 22:12:05
 */

// Represents a single database site in the distributed system. Manages local variables,
// handles site failures and recovery, and maintains transaction consistency. Each site
// stores a subset of database variables with their version history and tracks its
// operational status (UP/DOWN/RECOVERING) to ensure data consistency during failures.
#ifndef SITE_H
#define SITE_H
#include <string>
#include <map>
#include <mutex>
#include <unordered_set>
#include "Variable.h"

enum class SiteStatus
{
    UP,         // Site is operational and can process transactions
    DOWN,       // Site has failed and cannot process transactions
    RECOVERING  // Site is recovering from failure and has limited functionality
};

class Site
{
public:
    // Creates a new database site with the specified ID and initializes its variables
    Site(int id);
    
    // Returns the unique identifier of this database site
    int getId() const;
    
    // Returns the current operational status of the site (UP/DOWN/RECOVERING)
    SiteStatus getStatus() const;
    
    // Updates the site's operational status and triggers necessary state changes
    void setStatus(SiteStatus status);
    
    // Reads the value of a variable at a specific timestamp, ensuring transaction consistency
    int readVariable(const std::string &variableName, long timestamp);
    
    // Writes a new value to a variable with the given commit timestamp
    void writeVariable(const std::string &variableName, int value, long commitTime);
    
    // Simulates site failure by marking it as DOWN and handling necessary cleanup
    void fail();
    
    // Initiates site recovery process and marks variables as potentially inconsistent
    void recover();
    
    // Checks if this site maintains a copy of the specified variable
    bool hasVariable(const std::string &variableName) const;
    
    // Verifies if the variable has any committed writes since the given start time
    bool hasCommittedWrite(const std::string &variableName, long startTime) const;
    
    // Outputs the current state of all variables at this site for debugging
    void dump() const;
    
    // Returns the history of site failures as pairs of failure start and end times
    const std::vector<std::pair<long, long>> &getFailureTimes() const;

private:
    int id;                     // Unique identifier for this site
    SiteStatus status;          // Current operational status of the site
    std::mutex siteMutex;       // Ensures thread-safe access to site data
    std::map<std::string, Variable> variables;  // Storage for variables at this site
    std::unordered_set<std::string> unavailableVariables;  // Variables marked inconsistent during recovery
    
    // Sets up initial variables and their values when site is created
    void initializeVariables();
    
    // Tracks periods of site failure for consistency checking
    std::vector<std::pair<long, long>> failureTimes;
};

#endif // SITE_H