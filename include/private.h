#ifndef __XVOL_PRIVATE_H__
#define __XVOL_PRIVATE_H__

#define PATH_MAX_LEN 1024
#define STR_BUFF 1024
#define EX_FAST_REF_MASK 7
#define HANDLE_MULTIPLIER 4

#include <stdlib.h>

time_t wintime_to_unixtime(long long int input);

#endif
