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

#include "ipcex.h"
#include "splashscreen_bin.h"
#include "sv_sprite.h"

#define POWER_OFF_VBLANKS 1200

/**
 * Cette fonction doit être appelée après chaque interruption VBLANK. Après POWER_OFF_VBLANKS interruption Vblank sans action
 * l'écran est éteint.
 * Pour réveiller l'écran il suffit d'appuyer sur les touches L ou R ou toucher l'écran.
 * Cette fonction provient du programme lmp-ng.
 */
void screensaver(int key)
{
	static unsigned power_state = 0, power_vblanks = POWER_OFF_VBLANKS;
	static int lastkeys = 0; 

	if(key != lastkeys
			|| key & KEY_TOUCH
			|| key & KEY_R
			|| key & KEY_L) 
	{
		lastkeys = key;
		power_vblanks = POWER_OFF_VBLANKS;
	}
	else if(power_vblanks > 0)
	{
		power_vblanks--;
		
		if((power_vblanks == 0 || (key & KEY_LID)) && power_state == 0) 
		{
			power_state = 1;

			IPC_SendSync(IPC2_REQUEST_SET_BACKLIGHTS_OFF);
			swiWaitForVBlank(); /* HACK */
		} 
		else if(power_vblanks > 0 && (key & KEY_LID) == 0 && power_state == 1) 
		{
			power_state = 0;

			IPC_SendSync(IPC2_REQUEST_SET_BACKLIGHTS_ON);
			swiWaitForVBlank(); /* HACK */
		}
	}
}

static pj_uint16_t *frontbuffer = (pj_uint16_t *)0x06020000;
static pj_uint16_t *backbuffer = (pj_uint16_t *) 0x06040000;
static pj_uint16_t *top_bg, *bottom_bg;

//extern u16* font_myriad_web_9[];

//void printbtm(int x, int y, const char * str) {
//    // Google: ANSI ESCAPE sequences
//    // http://isthe.com/chongo/tech/comp/ansi_escapes.html
//    printf("\x1b[%d;%dH%s\x1b[K", x, y, str);
//    
////    dispString(backbuffer, 256,
////    x * 8, y * 8, 0xffff, str, 
////    font_myriad_web_9, 256, 32);
//}

static void screen_reset_backbuffer(void) 
{
    dmaCopy(top_bg, backbuffer, 256 * 192 * 2);
}

static void screen_switchbuffers(void) 
{
    pj_uint16_t *t;

    t = backbuffer;
    backbuffer = frontbuffer;
    frontbuffer = t;

    BG2_CR ^= BG_PRIORITY_1;
    BG2_CR ^= BG_PRIORITY_2;
    BG3_CR ^= BG_PRIORITY_1;
    BG3_CR ^= BG_PRIORITY_2;
}

pj_uint16_t *screen_get_backbuffer()
{
    return backbuffer;
}

/**
 * Rafraichit l'écran ainsi que la barre d'état supèrieur (heure ,batterie...)
 */
void screen_update()
{
//    dmaCopy(bottom_bg, BG_GFX_SUB, 256*192*2);
//    dmaCopy(top_bg, frontbuffer, 256*192*2);
    dmaCopy(top_bg, backbuffer, 256*192*2);
    
    topbar_update();

    screen_switchbuffers();
}

void screen_init()
{
    powerON(POWER_ALL_2D);
    
    /* setup main display */
    vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
    vramSetBankB(VRAM_B_MAIN_BG_0x06020000);
    vramSetBankD(VRAM_D_MAIN_BG_0x06040000);
    vramSetBankE(VRAM_E_MAIN_SPRITE);

    /* background */
    BG2_CR = BG_BMP16_256x256 | BG_BMP_BASE(128 / 16) | BG_PRIORITY_1;
    BG2_XDX = 1 << 8;
    BG2_XDY = 0;
    BG2_YDX = 0;
    BG2_YDY = 1 << 8;
    BG2_CX = 0;
    BG2_CY = 0;

    /* screen */
    BG3_CR = BG_BMP16_256x256 | BG_BMP_BASE(256 / 16) | BG_PRIORITY_2;
    BG3_XDX = 1 << 8;
    BG3_XDY = 0;
    BG3_YDX = 0;
    BG3_YDY = 1 << 8;
    BG3_CX = 8;
    BG3_CY = 8;

    /* setup sub display */
    videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE 
        | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT | DISPLAY_SPR_1D_BMP);
    vramSetBankC(VRAM_C_SUB_BG);
    vramSetBankI(VRAM_I_SUB_SPRITE);
    SUB_BG3_CR = BG_BMP16_256x256  | BG_PRIORITY_1;

    /* scaling and rotating */
    SUB_BG3_XDX = 1 << 8;
    SUB_BG3_XDY = 0;
    SUB_BG3_YDX = 0;
    SUB_BG3_YDY = 1 << 8;

    /* translation */
    SUB_BG3_CX = 0;
    SUB_BG3_CY = 0;
    
    dmaCopy(splashscreen_bin, frontbuffer, splashscreen_bin_size);
//    dmaCopy(splashscreen_bin, BG_GFX_SUB, splashscreen_bin_size);
}

void screen_display()
{
    videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE | DISPLAY_BG3_ACTIVE
        | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT | DISPLAY_SPR_1D_BMP);
}

extern sv_sprite *battery;

void screen_load_skin(const char *pathname)
{
    pj_uint16_t *buffer;
    pj_int32_t width, height;
    char filename[64]; 
   
    pj_ansi_snprintf(filename, 64, "%s/numpad.png", pathname);
    if (png2bmp(filename, &buffer, &width, &height))
    {
        if (bottom_bg) free(bottom_bg);
        bottom_bg = buffer;
    }
    
    pj_ansi_snprintf(filename, 64, "%s/bg-top.png", pathname);
    if (png2bmp(filename, &buffer, &width, &height))
    { 
        if (top_bg) free(top_bg);
        top_bg = buffer;
    }
    
    sprite_init();
    
    pj_ansi_snprintf(filename, 64, "%s/battery.png", pathname);
    if (png2bmp(filename, &buffer, &width, &height))
    {
        battery = sprite_new(buffer, width, height, 240, 3);
    }
    
    DC_FlushAll();
    
    screen_reset_backbuffer();
    screen_switchbuffers();
    screen_reset_backbuffer();
    screen_switchbuffers();
    dmaCopy(bottom_bg, BG_GFX_SUB, 256*192*2);
}

/* Draw horizontal line from x1,y to x2,y including final point*/
void
screen_draw_hline(pj_uint16_t *buffer, pj_uint16_t surface_width, 
    pj_uint16_t x1, pj_uint16_t x2, pj_uint16_t y, pj_uint16_t c)
{
//    assert (addr != 0);
//    assert (x1 >= 0 && x1 < psd->xres);
//    assert (x2 >= 0 && x2 < psd->xres);
//    assert (x2 >= x1);
//    assert (y >= 0 && y < psd->yres);
//    assert (c < psd->ncolors);

    buffer += x1 + y * surface_width;

    int count = x2 - x1 + 1;
    asm ( "   tst    %0, #2\n"      // odd 16bit address?
          "   strneh %2, [%0], #2\n"    // yes: store one pixel
          "   subne  %1, %1, #1\n"          // count--
          "   orr    %2, %2, %2, lsl #16\n" // extend pixelvalue to 32 bit
          "   subs   %1, %1, #2\n"      // count -= 2     
          "1: strhs  %2, [%0], #4\n"    // draw 2 pixels
          "   subhss %1, %1, #2\n"      // count -= 2     
          "   strhs  %2, [%0], #4\n"    // draw 2 pixels
          "   subhss %1, %1, #2\n"      // count -= 2     
          "   bhs    1b\n"          // and again
          "   adds   %1, %1, #1\n"      // count++
          "   streqh %2, [%0]\n"        // draw last byte
        : /* no output registers */
        : "r" (buffer), "r" (count), "r" (c) 
                : "cc" /* no memory, because we know GCC does not read it here */
                );
}

/* Draw a vertical line from x,y1 to x,y2 including final point*/
void
screen_draw_vline(pj_uint16_t *buffer, pj_uint16_t surface_width,
    pj_uint16_t x, pj_uint16_t y1, pj_uint16_t y2, pj_uint16_t c)
{
//    assert (addr != 0);
//    assert (x >= 0 && x < psd->xres);
//    assert (y1 >= 0 && y1 < psd->yres);
//    assert (y2 >= 0 && y2 < psd->yres);
//    assert (y2 >= y1);
//    assert (c < psd->ncolors);

    buffer += x + y1 * surface_width;
    int count = y2 - y1 + 1;
    asm ( "   mov    %3, %3, lsl #1\n"      // linelen * 2
          "   subs   %1, %1, #1\n"          // count--
          "1: strhsh %2, [%0], %3\n"    // draw 1 pixel
          "   subhss %1, %1, #1\n"      // count--
          "   strhsh %2, [%0], %3\n"    // draw 1 pixel
          "   subhss %1, %1, #1\n"      // count--
          "   bhs    1b\n"          // and again
        : /* no output registers */
        : "r" (buffer), "r" (count), "r" (c), "r" (surface_width) 
                : "cc" /* no memory, because we know GCC does not read it here */
                );
}

void
screen_draw_rect(pj_uint16_t *buffer, pj_uint16_t surface_width,
    pj_uint16_t x, pj_uint16_t y, pj_int16_t width, pj_int16_t height,
    pj_uint16_t c)
{
    screen_draw_hline(buffer, surface_width, x, x+width, y, c);
    screen_draw_vline(buffer, surface_width, x, y, y+height, c);
    screen_draw_hline(buffer, surface_width, x, x+width, y + height, c);
    screen_draw_vline(buffer, surface_width, x+width, y, y+height, c);
}
