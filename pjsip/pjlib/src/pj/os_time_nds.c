/* $Id$ */
/* 
 * Copyright (C) 2007-2009  Samuel Vinson <samuelv0304@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */
#include <pj/os.h>
#include <pj/errno.h>
#include <sys/time.h>
#include <errno.h>
#include <nds.h>
#if 0
#include "ipcex.h"
#endif

#include <pj/os.h>
#include <pj/errno.h>
#include <pj/compat/time.h>

#if defined(PJ_HAS_UNISTD_H) && PJ_HAS_UNISTD_H!=0
#    include <unistd.h>
#endif

#include <nds.h>

#include <errno.h>

#if 0
#define ISLEAP(y)           (!((y) % 4) && (((y) % 100) || !((y) % 400)))
#define DAY2S(dys)          (pj_uint32_t)((dys) * 86400)
#define HOURS2S(hrs)        (pj_uint32_t)((hrs) * 3600)
#define MINUTES2S(min)      (pj_uint32_t)((min) * 60)
#define rtc24hours ((IPC->time.rtc.hours > 51) ? IPC->time.rtc.hours-40 : IPC->time.rtc.hours)

pj_uint32_t mdays[]   = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
pj_uint32_t mdays_leap[] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };

static unsigned long NDS_time(time_t *t) {
    unsigned long time_sec;
    unsigned long year_shit1 = (IPC->time.rtc.year+30) >> 2; // divide by 4
    unsigned long year_shit2 = (IPC->time.rtc.year+30) & 3; // module by 4
    unsigned long years_in_sec = year_shit1 * (31536000*3 + 31622400);
    if(year_shit2 == 1)
        years_in_sec += 31536000;
    else if(year_shit2 == 2)
        years_in_sec += 31536000 + 31622400;
    else if(year_shit2 == 3)
        years_in_sec += (31536000*2) + 31622400;

    time_sec = years_in_sec 
       + (ISLEAP(IPC->time.rtc.year+2000) ?
            DAY2S(mdays_leap[IPC->time.rtc.month-1]) : DAY2S(mdays[IPC->time.rtc.month-1]))
        + DAY2S(IPC->time.rtc.day-1)
        + HOURS2S(rtc24hours)
        + MINUTES2S(IPC->time.rtc.minutes)
        + IPC->time.rtc.seconds;

    if (t != NULL) *t = time_sec;
    
    return time_sec;
}
#endif
int NDS_gettimeofday(struct timeval *tv, struct timezone *tz)
{
    if (tv == NULL)
    {
        errno = EFAULT;
        return -1;
    }

#if 0
    tv->tv_sec = NDS_time(NULL);
    tv->tv_usec = (int)(IPC2->ms * 1000000. / 60.);
#else
    tv->tv_sec = getIPC()->unixTime;
    tv->tv_usec = 0;
#endif

    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

PJ_DEF(pj_status_t) pj_gettimeofday(pj_time_val *p_tv)
{
    struct timeval the_time;
    int rc;

    PJ_CHECK_STACK();

    rc = NDS_gettimeofday(&the_time, NULL);
    //rc = gettimeofday(&the_time, NULL);
    if (rc != 0)
    return PJ_RETURN_OS_ERROR(pj_get_native_os_error());

    p_tv->sec = the_time.tv_sec;
    p_tv->msec = the_time.tv_usec / 1000;
    return PJ_SUCCESS;
}

