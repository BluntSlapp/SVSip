/*
 * SvSIP, SIP/VoIP client for Nintendo DS
 * Copyright (C) 2007-2009  Samuel Vinson <samuelv0304@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef __SCREENSAVER_H__
#define __SCREENSAVER_H__

#include <pj/types.h>

PJ_BEGIN_DECL

PJ_DECL(void) screensaver_set_timeout(pj_uint32_t seconds);
PJ_DECL(void) screensaver_start      ();
PJ_DECL(void) screensaver_stop       ();


PJ_END_DECL

#endif /* __SCREENSAVER_H__ */
