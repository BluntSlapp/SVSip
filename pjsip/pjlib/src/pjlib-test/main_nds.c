/* 
 * Copyright (C) 2007-2009  Samuel Vinson <samuelv0304@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */
#include "test.h"
#include <pj/log.h>

#include <nds.h>
#include <dswifi9.h>
#include <fat.h>

#define THIS_FILE   "main_nds.c"

#define WifiTimerInterval_ms (50)

// sgIP_dbgprint only needed in debug version
void
sgIP_dbgprint(char * txt __attribute__((unused)), ...) {      
}

static void WaitVbl() 
{
    while(REG_VCOUNT>192);
    while(REG_VCOUNT<192);
}

// wifi timer function, to update internals of sgIP
static void arm9_wifiTimer(void) {
    Wifi_Timer(WifiTimerInterval_ms);
}

// notification function to send fifo message to arm7
void arm9_synctoarm7() { // send fifo message
    REG_IPC_FIFO_TX=0x87654321;
}

// interrupt handler to receive fifo messages from arm7
void arm9_fifo() { // check incoming fifo messages
    u32 value = REG_IPC_FIFO_RX;
    if(value == 0x87654321) {
        Wifi_Sync();
    }
}

void arm9_wifiInit()
{
    // send fifo message to initialize the arm7 wifi
    REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR; // enable & clear FIFO
    
    u32 Wifi_pass= Wifi_Init(WIFIINIT_OPTION_USELED);
    REG_IPC_FIFO_TX=0x12345678;
    REG_IPC_FIFO_TX=Wifi_pass;

    *((volatile u16 *)0x0400010E) = 0; // disable timer3

    irqSet(IRQ_TIMER3, arm9_wifiTimer); // setup timer IRQ
    irqEnable(IRQ_TIMER3);
    irqSet(IRQ_FIFO_NOT_EMPTY, arm9_fifo); // setup fifo IRQ
    irqEnable(IRQ_FIFO_NOT_EMPTY);

    REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ; // enable FIFO IRQ

    Wifi_SetSyncHandler(arm9_synctoarm7); // tell wifi lib to use our handler to notify arm7

    // set timer3
    *((volatile u16 *)0x0400010C) = -(131062*WifiTimerInterval_ms/1000); // 131062 * 256 cycles = ~1000ms;
    *((volatile u16 *)0x0400010E) = 0x00C2; // enable, irq, 1/256 clock
    
    while(Wifi_CheckInit()==0) 
    { // wait for arm7 to be initted successfully
        WaitVbl(); // wait for vblank
    }
        
     // wifi init complete - wifi lib can now be used!
}

static int arm9_wifiConnect()
{
    int j = 0, state = 5, delay;
    arm9_wifiInit();
    
    PJ_LOG(3,(THIS_FILE, "Connecting to AP..."));
    while(1) 
    {
        WaitVbl();
        switch(state) 
        {
            case 5: // connect to AP
                state=6;
                // firmware
                Wifi_AutoConnect();
                PJ_LOG(3,(THIS_FILE, "Connecting to Access Point..."));
            case 6:
                j=Wifi_AssocStatus();
                if(j == ASSOCSTATUS_ASSOCIATED) state=30;
                if(j == ASSOCSTATUS_CANNOTCONNECT) state=20;
                break;
            case 20:
                PJ_LOG(3,(THIS_FILE, "Cannot connect to Access Point."));
                return 0;
            case 30:
                delay=60;
                state=31;
            case 31:
                PJ_LOG(3,(THIS_FILE, "Connected!"));
                if(!(delay--)) 
                    return 1;
                break;
        }
    }
   
   return 0;
}

int main(int argc, char *argv[])
{
    int rc;
    
    defaultExceptionHandler();
    consoleDemoInit();
    irqInit();
    irqEnable(IRQ_VBLANK);
    
    if(fatInitDefault() != 1)
    {
        return -1;
    }

    arm9_wifiConnect();
    
    rc = test_main();
    
    Wifi_DisconnectAP();
    Wifi_DisableWifi();
    
    return rc;
}
