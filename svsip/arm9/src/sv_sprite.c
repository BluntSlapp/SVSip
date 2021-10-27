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
#include <pjlib.h>

#include "sv_sprite.h"

void sprite_init()
{
    int i;
    SpriteEntry *s;

    s = (SpriteEntry *) OAM;
    for(i = 0; i < SPRITE_COUNT; i++) 
    {
        s[i].attribute[0] = ATTR0_DISABLED;
        s[i].attribute[1] = 0;
        s[i].attribute[2] = 0;
    }
}

sv_sprite *sprite_new(pj_uint16_t *buffer, pj_int32_t width, pj_int32_t height,
                      pj_uint16_t xpos, pj_uint16_t ypos)
{
    static pj_int32_t num = 0;
    static pj_int32_t nextpos = 0;
    
    sv_sprite *spr;
    SpriteEntry *sprite;
    pj_uint16_t *icon;
    
    PJ_ASSERT_RETURN(buffer, NULL);
    
    spr = (sv_sprite*)malloc(sizeof(sv_sprite));
    PJ_ASSERT_RETURN(spr, NULL);
    
    sprite = ((SpriteEntry *) OAM) + num; num++;
    
    sprite->attribute[0] = ATTR0_BMP | ATTR0_COLOR_16 | ypos;
    sprite->attribute[2] = ATTR2_ALPHA(1) | nextpos / 128;
    icon = ((pj_uint16_t *) SPRITE_GFX) + nextpos / 2;
    switch(width) {
        case 8:
            sprite->attribute[1] = ATTR1_SIZE_8 | xpos;
            nextpos += 8*8 * 2;
            break;

        case 16:
            sprite->attribute[1] = ATTR1_SIZE_16 | xpos;
            nextpos += 16*16 * 2;
            break;

        case 32:
            sprite->attribute[1] = ATTR1_SIZE_32 | xpos;
            nextpos += 32*32 * 2;
            break;

        case 64:
            sprite->attribute[1] = ATTR1_SIZE_64 | xpos;
            nextpos += 64*64 * 2;
            break;

        default:
            break;
    }
   
    spr->icons         = buffer;
    spr->bytesperpixel = 2;
    spr->size          = width;
    spr->nb            = height / width;
    spr->xpos          = xpos;
    spr->ypos          = ypos;
    
    spr->sprite        = sprite;
    spr->icon          = icon;

    return spr;
}
