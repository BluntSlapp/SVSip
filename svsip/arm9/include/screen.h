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

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <pj/types.h>

PJ_BEGIN_DECL

void screen_update();
void screen_init();
void screen_display();
void screen_load_skin(const char *pathname);
pj_uint16_t *screen_get_backbuffer();

void screen_draw_hline (pj_uint16_t *buffer, pj_uint16_t surface_width, 
    pj_uint16_t x1, pj_uint16_t x2, pj_uint16_t y, pj_uint16_t c);
void screen_draw_vline (pj_uint16_t *buffer, pj_uint16_t surface_width,
    pj_uint16_t x, pj_uint16_t y1, pj_uint16_t y2, pj_uint16_t c);
void screen_draw_rect  (pj_uint16_t *buffer, pj_uint16_t surface_width,
    pj_uint16_t x, pj_uint16_t y, pj_int16_t width, pj_int16_t height,
    pj_uint16_t c);

PJ_END_DECL

#endif /*__SCREEN_H__*/
