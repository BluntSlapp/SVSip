/*
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

#define WIFI

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

    sync = IPC_GetSync();

    {
        int oldval, newval;
        
        newval = oldval = readPowerManagement(PM_CONTROL_REG);

        if(sync == IPC2_REQUEST_SET_BACKLIGHTS_OFF)
            newval = oldval & ~PM_BACKLIGHT_TOP & ~PM_BACKLIGHT_BOTTOM;

        else if(sync == IPC2_REQUEST_SET_BACKLIGHTS_ON)
            newval = oldval | PM_BACKLIGHT_TOP | PM_BACKLIGHT_BOTTOM;

        else if(sync == IPC2_REQUEST_LEDBLINK_OFF)
            newval = oldval & ~PM_LED_BLINK;

        else if(sync == IPC2_REQUEST_LEDBLINK_ON)
            newval = oldval | PM_LED_BLINK;

        writePowerManagement(PM_CONTROL_REG, newval);
    }
}
touchPosition tempPos;
//---------------------------------------------------------------------------------
void VblankHandler(void) 
{
    static int lastbut = -1;
    
    uint16 but=0, x=0, y=0, xpx=0, ypx=0, z1=0, z2=0;
    uint16 batt=0, aux=0;

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

    batt = touchRead(TSC_MEASURE_BATTERY);
    aux  = touchRead(TSC_MEASURE_AUX);

    IPC->touchX         = x;
    IPC->touchY         = y;
    IPC->touchXpx       = xpx;
    IPC->touchYpx       = ypx;
    IPC->touchZ1        = z1;
    IPC->touchZ2        = z2;
    IPC->buttons        = but;
    IPC->battery        = batt;
    IPC->aux            = aux;

#ifdef WIFI
	Wifi_Update(); // update wireless in vblank
#endif
}

//---------------------------------------------------------------------------------
int main(int argc, char ** argv) 
{
    // Reset the clock if needed
//    rtcReset();

    //---------------------------------------------------------------------------------
#ifdef WIFI
    REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR; // enable & prepare fifo asap
#endif
 
    // Set up the interrupt handler
	irqInit();

    // Start the RTC tracking IRQ
    initClockIRQ();

	irqSet(IRQ_VBLANK, VblankHandler);
	irqEnable(IRQ_VBLANK);
    
    //enable sound
    arm7_initSoundDevice();
    
    irqSet(IRQ_IPC_SYNC, InterruptHandler_IPC_SYNC);
    REG_IPC_SYNC = IPC_SYNC_IRQ_ENABLE;
    
#ifdef WIFI
    arm7_initWifi();
#endif

 
	// Keep the ARM7 out of main RAM
	while (1) 
    {
        swiWaitForVBlank(); // FIXME: réveiller sur d'autres interruptions ?
        arm7_microProcess();

        commandControl();
    }

    return 0;
}


