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

#include <nds.h>

#include <string.h>

#include "text.h"

#define RGBToGreyScale(x) ((((x&0x03E0)>>5)+((x&0x7C00)>>10)+((x&0x001F)))/3)

u16 getStringWidth(const char *s, uint16 **f) {
    u16 i;
    u16 w, max;
    u32 len;
    unsigned char c;

    len = strlen(s);
    w = max = 0;
    for(i = 0; i < len; i++) {
        c = s[i];
        if(c < 32 || f[c - 32] == 0)
            c = '?';

        if(s[i] == '\n') {
            if(w > max)
                max = w;
            w = 0;
            continue;
        }

        w += f[c - 32][0];
    }
    if(w > max)
        max = w;

    return max;
}

u16 getStringHeight(const char *s, uint16 **f) {
    return f[0][1];
}

static void dispChar(s16 x_offset, s16 y_offset, u16 mask, unsigned char c, 
    u16 *buffer, uint16 **font, u16 surface_width) {
    int w, h;
    int i, j;
    u16 color;

    if(c < 32 || font[c - 32] == 0)
        return;

    w = font[c - 32][0];
    h = font[c - 32][1];

    if(mask == 0)
        for(i = 0; i < h; i++)
            for(j = 0; j < w; j++) {
                color = RGBToGreyScale(font[c-32][i*(w + 1) + j + 2]) & 31;
                if(color != 0) {
                    color |= (color << 10) | (color << 5);
                    color |= color & buffer[(y_offset + i) * surface_width + x_offset + j];
                    buffer[(y_offset + i) * surface_width + x_offset + j] = (0x7FFF - color) | BIT(15);
                }
            }
    else {
        for(i = 0; i < h; i++)
            for(j = 0; j < w; j++) {
                color = RGBToGreyScale(font[c-32][i*(w + 1) + j + 2]) & 31;
                if(color != 0) {
                    if(color > 24)
                        buffer[(y_offset + i) * surface_width + x_offset + j] = (RGB15(31,31,31) | BIT(15)) & mask;
                    else if(color > 16)
                        buffer[(y_offset + i) * surface_width + x_offset + j] = (RGB15(28,28,28) | BIT(15)) & mask;
                    else if(color > 8)
                        buffer[(y_offset + i) * surface_width + x_offset + j] = (RGB15(25,25,25) | BIT(15)) & mask;

/*                  color |= (color << 10) | (color << 5);
                    color |= (0x7fff - color) & buffer[(y_offset + i) * surface_width + x_offset + j];
                    buffer[(y_offset + i) * surface_width + x_offset + j] = color | BIT(15); */
                }
            }
    }
}

//void dispString(s16 x_offset, s16 y_offset, u16 mask, const char *text, u16 *buffer, uint16 **font, u16 width, u16 height, u16 surface_width) 
void dispString(pj_uint16_t *buffer, pj_uint16_t surface_width,
    pj_int16_t x_offset, pj_int16_t y_offset, pj_uint16_t mask, const char *text, 
    pj_uint16_t **font, pj_uint16_t width, pj_uint16_t height)
{
    u32 i;
    u32 len;
    u16 x_pos;
    unsigned char c;

    len = strlen(text);
    x_pos = 0;
    for(i = 0; i < len; i++) {
        c = text[i];
        if(c < 32 || font[c - 32] == 0)
            c = '?';

        if(text[i] == '\n' || x_offset + x_pos + font[c - 32][0] >= width) {
            y_offset += font[0][1] + 2;
            x_pos = 0;
            continue;
        }

        dispChar(x_offset + x_pos, y_offset, mask, c, buffer, font, surface_width);
        x_pos += font[c - 32][0];
    }
}

//void dispString2(s16 x_offset, s16 y_offset, u16 mask, const char *text, 
//    u16 *buffer, uint16 **font, u16 width, u16 height, u16 surface_width)
void dispString2 (pj_uint16_t *buffer, pj_uint16_t surface_width,
    s16 x_offset, s16 y_offset, u16 mask, const char *text,  
    uint16 **font, u16 width, u16 height)
{
    u32 i;
    u32 len;
    u16 x_pos;
    unsigned char c;

    len = strlen(text);
    x_pos = 0;
    for(i = 0; i < len; i++) {
        c = text[i];

        if(c < 32 || font[c - 32] == 0)
            c = '?';

        if(text[i] == '\n' || x_offset + x_pos + font[c - 32][0] >= width)
            break;

        dispChar(x_offset + x_pos, y_offset, mask, c, buffer, font, surface_width);
        x_pos += font[c - 32][0];
    }
}
