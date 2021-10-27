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

#ifdef REBOOT
#include <reboot.h>
#endif

#include "arm7_wifi.h"
#include "arm7_sound.h"
#include "arm7_microphone.h"
#include "ipcex.h"

void commandControl()
{
    u8 control = IPC2->sound_control;
    if (control != IPC2_SOUND_NOP)
    {
        switch (control)
        {
            case IPC2_SOUND_START: arm7_pcmplay(); break;
            case IPC2_SOUND_STOP: arm7_pcmstop(); break;
            case IPC2_SOUND_CLICK: arm7_playclick(); break;
            case IPC2_RECORD_START: arm7_microStart(); break;
            case IPC2_RECORD_STOP: arm7_microStop(); break;
        }
        IPC2->sound_control = IPC2_SOUND_NOP;
    }
}

void InterruptHandler_IPC_SYNC(void) 
{
    u8 sync;
	int level;

    sync = IPC_GetSync();

    {
        int oldval, newval;
        
        newval = oldval = readPowerManagement(PM_CONTROL_REG);

		switch(sync)
		{
		 case IPC2_REQUEST_SET_BACKLIGHTS_OFF:
            newval = oldval & ~PM_BACKLIGHT_TOP & ~PM_BACKLIGHT_BOTTOM;
			break;
         case IPC2_REQUEST_SET_BACKLIGHTS_ON:
            newval = oldval | PM_BACKLIGHT_TOP | PM_BACKLIGHT_BOTTOM;
			break;
         case IPC2_REQUEST_LEDBLINK_OFF:
            newval = oldval & ~PM_LED_BLINK;
			break;
         case IPC2_REQUEST_LEDBLINK_ON:
            newval = oldval | PM_LED_BLINK;
			break;
		 case IPC2_REQUEST_POWER_OFF:
			newval = oldval | PM_POWER_DOWN;
			break;
		case IPC2_REQUEST_BRIGHTNESS:
			level = readPowerManagement(4/*PM_DSLITE_REG*/) - 64;
			level++; level = level == 4 ? 0 : level; // level = level % 4;
			writePowerManagement(4/*PM_DSLITE_REG*/, level);
			return;
		 default:
			break;
		}

        writePowerManagement(PM_CONTROL_REG, newval);
    }
}

touchPosition first,tempPos;

//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
	static int lastbut = -1;
	
	uint16 but=0, x=0, y=0, xpx=0, ypx=0, z1=0, z2=0;

	but = REG_KEYXY;

	if (!( (but ^ lastbut) & (1<<6))) {
 
		tempPos = touchReadXY();

		if ( tempPos.x == 0 || tempPos.y == 0 ) {
			but |= (1 <<6);
			lastbut = but;
		} else {
			x = tempPos.x;
			y = tempPos.y;
			xpx = tempPos.px;
			ypx = tempPos.py;
			z1 = tempPos.z1;
			z2 = tempPos.z2;
		}
		
	} else {
		lastbut = but;
		but |= (1 <<6);
	}

	IPC->touchX			= x;
	IPC->touchY			= y;
	IPC->touchXpx		= xpx;
	IPC->touchYpx		= ypx;
	IPC->touchZ1		= z1;
	IPC->touchZ2		= z2;
	IPC->buttons		= but;
    
    IPC->battery  = (u16)(readPowerManagement(PM_BATTERY_REG) & 0x01);
    if(readPowerManagement(4) & (1<<6))
        IPC->aux    = (u16)(readPowerManagement(4) & 0x08);
    else
        IPC->aux    = 0;

}

//---------------------------------------------------------------------------------
void VblankHandler(void) 
{
	Wifi_Update(); // update wireless in vblank
}

//---------------------------------------------------------------------------------
int main(int argc, char ** argv) {
//---------------------------------------------------------------------------------

	// enable & prepare fifo asap
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR; 
	
	//enable sound
	//powerON(POWER_SOUND);
	//SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);

	irqInit();

	// Start the RTC tracking IRQ
    initClockIRQ();
	
	irqSet(IRQ_VBLANK, VblankHandler);

	//enable sound
    arm7_initSoundDevice();
	
	SetYtrigger(80);
	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqSet(IRQ_IPC_SYNC, InterruptHandler_IPC_SYNC);
    REG_IPC_SYNC = IPC_SYNC_IRQ_ENABLE;
	
	irqEnable( IRQ_VBLANK | IRQ_VCOUNT);

	IPC->mailBusy = 0;

	arm7_initWifi();
	
	// Keep the ARM7 out of main RAM
	while (1) 
	{
		arm7_microProcess();
        commandControl();
#ifdef REBOOT		
		if(need_reboot())
			reboot();
#endif // REBOOT			
		swiWaitForVBlank();
	}
}


