/**
 * @   Author: Ke Wang & Siwen Tao
 * @   Email: kw3484@nyu.edu & st5297@nyu.edu
 * @   Modified time: 2024-12-08 22:15:58
 */

// Handles locking mechanism for database variables to ensure transaction isolation
#ifndef LOCK_H
#define LOCK_H

#include <iostream>
#include <set>
using namespace std;

// Total number of variables in the database system
#define TOTAL_VARIABLES 20

// Represents different types of locks that can be held on variables
enum Lock_type
{
    READ_LOCK,   // Shared lock for reading
    WRITE_LOCK,  // Exclusive lock for writing
    NO_LOCK      // No lock held
};

class Lock
{
public:
    Lock_type type;              // Type of lock currently held
    set<int> transactions;       // Set of transaction IDs holding this lock

    // Creates a new lock with specified type and set of transactions
    Lock(Lock_type type, set<int> transactions);
};

#endif