#include "Lock.h"

Lock::Lock(Lock_type type, set<int> transactions)
{
    this->type = type;
    this->transactions = transactions;
}