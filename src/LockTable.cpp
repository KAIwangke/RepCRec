/**
 * @ Author: Ke Wang & Siwen Tao
 * @ Email: kw3484@nyu.edu & st5297@nyu.edu
 * @ Create Time: 2024-11-26 20:07:56
 * @ Modified time: 2024-12-05 21:32:27
 */

#include "Lock.h"

Lock::Lock(Lock_type type, set<int> transactions)
{
    this->type = type;
    this->transactions = transactions;
}