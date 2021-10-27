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

#include <pjsua-lib/pjsua.h>

#include <stdio.h>
#include <nds.h>
#include <fat.h>

#ifdef REBOOT
#include <reboot.h>
#endif

#include "sv_config.h"

#include "ipcex.h"
#include "sv_console.h"
#include "screen.h"



char keys[4][3] = {{'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}, {'*', '0', '#'}};

#define X0_NUMPAD 6
#define Y0_NUMPAD 8
#define WIDTH_PAD  186
#define HEIGHT_PAD 179
#define HEIGHT_BUT 42
#define WIDTH_BUT  58
#define EX 6
#define EY 4

#define NONE    0
#define CALLING 1
#define HANGUP  2
#define DELETE  3

typedef struct Point 
{
	int x, y;
} Point;


char readKeyboard()
{
	int x,y;
	
	touchPosition touchXY = touchReadXY();

	x = touchXY.px;
	y = touchXY.py;
	//printf("\x1b[10;0H");
   // printf("Touch x = %d   \n", x);
    //printf("Touch y = %d   \n", y);
	
	if (x > X0_NUMPAD && x < WIDTH_PAD + X0_NUMPAD &&
		y > Y0_NUMPAD && y < HEIGHT_PAD + Y0_NUMPAD)
	{
		x -= (X0_NUMPAD + EX / 2);
		y -= (X0_NUMPAD + EY / 2);
		
		x /= (WIDTH_BUT + EX);
		y /= (HEIGHT_BUT + EY);
		
		//printf("pos x = %d   \n", x);
	    //printf("pos y = %d   \n", y);
		//printf("key   = %c   \n", keys[y][x]);
		return keys[y][x];
	}
	
	return '\0';
}

char readControl()
{
	int x,y;
	
	touchPosition touchXY = touchReadXY();

	x = touchXY.px;
	y = touchXY.py;
	
	if (x > 205 && x < 246)
	{
		if (y > 8 && y < 49)
		{
			return CALLING;
		}
		if (y > 52 && y < 93)
		{
			return HANGUP;
		}
		if (y > 140 && y < 181)
		{
			return DELETE;
		}
	}
	return NONE;
}

int main(void) 
{
    static enum { CONSOLE, GRAPHICS } mode = GRAPHICS;
    
	int i;
	//char *file_name = "svsip/bg_keypad.png";
	//char *file_name = "bg_keypad.png";
	char *file_name = "svsip/numpad.png";
	//char *file_name = "dsip.png";
	char *config_file= "svsip/config.txt";
	ushort *bmp; 
	int width, height;
	int delta = 0;
	
	char number[32];
	char oldKey = 0, newKey;
	char oldCtrl = 0, newCtrl;

	pjsua_acc_id  acc_id;
	pjsua_call_id call_id = PJSUA_INVALID_ID;

	memset(number, 0, strlen(number));

	defaultExceptionHandler();
    irqInit();
    irqEnable(IRQ_VBLANK);
    
    screen_init();
    screen_display();
    
#if CONSOLE_DEBUG
    console_init();
    scanKeys();
    if(keysHeld() & KEY_START)
    {
        mode = CONSOLE;    
        console_display();
    }
#endif /* CONSOLE_DEBUG */
    
    // starting fat...
	if(fatInitDefault() != 1) 
	{
		consoleClear();
		printf("fatInitDefault() failed!\n");
		printf("Press [B] to quit");
		while(1)
		{   
			scanKeys();
			if(keysHeld() & KEY_B)
#ifdef REBOOT		
				if (can_reboot())
					reboot();
#else
				return 0;
#endif
			swiWaitForVBlank();
		}
	}

	if (initCaller(&acc_id, config_file) == 0)
	{
		return 1;
	}

    screen_load_skin("svsip");
    screen_update();

    mode = CONSOLE;
    console_display();
	consoleClear();
		
	while(1) 
	{
		uint32 key;
		scanKeys();

#if CONSOLE_DEBUG        
        if((keysDown() & KEY_START) != 0) 
        {
            if(mode == GRAPHICS) 
            {
                console_display();
                mode = CONSOLE;
            }
            else 
            {
                screen_display();
                mode = GRAPHICS;
            }
        }
#endif /* CONSOLE_DEBUG */
		
		key = keysHeld();
		if((key & KEY_X) && (key & KEY_Y) && (key & KEY_A) && (key & KEY_B))
		{
			cleanupCaller();
			IPC_SendSync(IPC2_REQUEST_POWER_OFF);
		}
		if ((key & KEY_SELECT) && (key & KEY_START)) 
		{
			break;
		}
		if (keysDown() & KEY_Y)
		{
			IPC_SendSync(IPC2_REQUEST_BRIGHTNESS);
		}
		if (key & KEY_TOUCH)
		//if (keysDown() & KEY_TOUCH)
		{
			newKey = readKeyboard();
			newCtrl = readControl();
		}
		else
		{
			newKey = '\0';
			newCtrl = 0;
		}
		if (key & KEY_B)
			newCtrl = HANGUP;
		if (key & KEY_A)
			newCtrl = CALLING;
		
		if (newKey == '\0')
		{
			oldKey = newKey;
		}
		else if (newKey != '\0' && newKey != oldKey)
		{
			IPC2->sound_control = IPC2_SOUND_CLICK;
			if (phone_getstate() == PJSIP_INV_STATE_NULL || 
				phone_getstate() == PJSIP_INV_STATE_DISCONNECTED)
			{
				sprintf(number, "%s%c", number, newKey);
				printbtm(6,0, "Number:");
				printbtm(6,9, number);
			}
			else
			{
				call_play_digit(call_id, newKey);		
			}
			oldKey = newKey;
		}
		if (newCtrl != oldCtrl)
		{
			IPC2->sound_control = IPC2_SOUND_CLICK;
			switch(newCtrl)
			{
				case CALLING:
					if ((phone_getstate() == PJSIP_INV_STATE_NULL || 
						phone_getstate() == PJSIP_INV_STATE_DISCONNECTED) && 
							strlen(number) > 0)
					{
						call(&acc_id, number, &call_id);
					}
					else if ((call_id == PJSUA_INVALID_ID) && 
						(phone_getstate() == PJSIP_INV_STATE_EARLY ||
						 phone_getstate() == PJSIP_INV_STATE_INCOMING))
					{
						answer();
					}
					oldCtrl = NONE;
					break;
				case HANGUP:
					if (phone_getstate() != PJSIP_INV_STATE_NULL && 
						phone_getstate() != PJSIP_INV_STATE_DISCONNECTED)
					{
						pjsua_call_hangup_all();
						/* Hangup current calls */
						//pjsua_call_hangup(call_id, 0, NULL, NULL);
						call_id = PJSUA_INVALID_ID;
						number[0] = '\0';
					}
					oldCtrl = NONE;
					break;
				case DELETE:
					if (strlen(number) > 0)
					{
						number[strlen(number)-1] = '\0';
						printbtm(6,0, "Number:");
				        printbtm(6,9, number);
					}
					oldCtrl = newCtrl;
					break;
				default: oldCtrl = NONE; break;
			}
			//oldCtrl = newCtrl;
			//oldCtrl = NONE;
		}
		
		// Economiseur d'écran
		screensaver(key);
		screen_update();
		
		pjsua_handle_events(1);
		
		swiWaitForVBlank();
		//swiWaitForIRQ();
	}
	
	cleanupCaller();

	// Réveille l'écran
	IPC_SendSync(IPC2_REQUEST_SET_BACKLIGHTS_ON);
	
	for (i = 0; i < 50; ++i)
		swiWaitForVBlank();

#ifdef REBOOT		
	if (can_reboot())
		reboot();
#endif
	
	return 0;
}
