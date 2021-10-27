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

#include "sv_config.h" 

#if CONSOLE_DEBUG

#include <nds.h>
#include <nds/arm9/console.h>
#include "sv_console.h"

void console_init()
{   
    vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
    
    /* console */
    BG0_CR = BG_MAP_BASE(31);
    BG_PALETTE[255] = RGB15(31,31,31);
    consoleInitDefault((u16*)SCREEN_BASE_BLOCK(31), (u16*)CHAR_BASE_BLOCK(0), 16);
}

void console_display()
{
    videoSetMode(MODE_5_2D | DISPLAY_BG0_ACTIVE);
}

#endif /* CONSOLE_DEBUG */
