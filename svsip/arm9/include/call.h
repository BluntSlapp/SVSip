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

#ifndef __CALL_H__
#define __CALL_H__

/* Include all PJSIP-UA headers */
#include <pjsip_ua.h>

PJ_BEGIN_DECL

pjsip_inv_state phone_getstate();
pj_status_t     call       (pjsua_acc_id *acc_id, char *number, pjsua_call_id *call_id);


PJ_END_DECL

#endif /* __CALL_H__ */
