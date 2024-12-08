#ifndef LOCK_H
#define LOCK_H

#include <iostream>
#include <set>

using namespace std;

#define TOTAL_VARIABLES 20

enum Lock_type
{
    READ_LOCK,
    WRITE_LOCK,
    NO_LOCK
};

class Lock
{
public:
    Lock_type type;
    // TODO: Modify the DS
    set<int> transactions;
    Lock(Lock_type type, set<int> transactions);
};

#endif