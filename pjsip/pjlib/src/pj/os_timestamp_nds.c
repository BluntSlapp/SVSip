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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include <errno.h>

#define USEC_PER_SEC      1000000
#define MACHINE_SPEED_MHZ 67

extern int NDS_gettimeofday(struct timeval *tv, struct timezone *tz);

PJ_DEF(pj_status_t) pj_get_timestamp(pj_timestamp *ts)
{
    struct timeval tv;

    if (NDS_gettimeofday(&tv, NULL) != 0) {
    return PJ_RETURN_OS_ERROR(pj_get_native_os_error());
    }

    ts->u64 = tv.tv_sec;
    ts->u64 *= USEC_PER_SEC;
    ts->u64 += tv.tv_usec;

    return PJ_SUCCESS;
}

PJ_DEF(pj_status_t) pj_get_timestamp_freq(pj_timestamp *freq)
{
//    freq->u32.hi = 0;
//    freq->u32.lo = USEC_PER_SEC;
    freq->u64 = MACHINE_SPEED_MHZ * 1000000.0;

    return PJ_SUCCESS;
}
