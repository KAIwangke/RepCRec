/**
 * @ Author: Ke Wang & Siwen Tao
 * @ Email: kw3484@nyu.edu & st5297@nyu.edu
 * @ Create Time: 2024-11-26 20:07:56
 * @ Modified time: 2024-12-05 21:33:19
 */

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