#include "private.h"

#define TICKS_PER_SECOND 10000000
#define EPOCH_DIFFERENCE 11644473600LL
time_t wintime_to_unixtime(long long int input)
{
    long long int temp;
    temp = input / TICKS_PER_SECOND;
    temp = temp - EPOCH_DIFFERENCE;
    return (time_t)temp;
}
