#ifndef __M_TIME_H__
#define __M_TIME_H__
#include <time.h>

struct tm *gmtime_r(const time_t *timer, struct tm *tmbuf);
time_t mktime(struct tm *tmbuf);

#endif

