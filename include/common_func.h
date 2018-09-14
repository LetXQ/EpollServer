#ifndef COMMON_FUNC_H
#define COMMON_FUNC_H

#include <sys/time.h>

class ComFuncs {
public:
    static int32_t GetNowSec()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec;
    }
};


#endif // COMMON_FUNC_H
