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


#include <pj/types.h>

#ifndef __SVSIP_TEXT_H__
#define __SVSIP_TEXT_H__

PJ_BEGIN_DECL

void dispString(pj_uint16_t *buffer, pj_uint16_t surface_width,
    pj_int16_t x_offset, pj_int16_t y_offset, pj_uint16_t mask, const char *text, 
    pj_uint16_t **font, pj_uint16_t width, pj_uint16_t height);


void dispString2 (pj_uint16_t *buffer, pj_uint16_t surface_width,
    pj_int16_t x_offset, pj_int16_t y_offset, pj_uint16_t mask, const char *text,  
    pj_uint16_t **font, pj_uint16_t width, pj_uint16_t height);

PJ_END_DECL

#endif /* __SVSIP_TEXT_H__ */
