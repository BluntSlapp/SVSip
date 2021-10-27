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

#include <pjlib.h>

#include <netinet/in.h>

#include <nds.h>
#include <dswifi9.h>
#include "arm9_wifi.h"


/* For logging purpose. */
#define THIS_FILE   "arm9_wifi.c"

#define VCOUNT               (*((u16 volatile *) 0x04000006))
#define WifiTimerInterval_ms (50)
//#define WifiTimerInterval_ms (20)

#ifdef LOCAL_VBLANK
static void onVblank() {    
    //ticks++;
}
#endif

// Dswifi stub functions
//void *
//sgIP_malloc(int size) {
//  return malloc(size);
//}
//void
//sgIP_free(void * ptr) {
//  free(ptr);
//}

// sgIP_dbgprint only needed in debug version
void
sgIP_dbgprint(char * txt __attribute__((unused)), ...) {      
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
//    u32 Wifi_pass= Wifi_Init(WIFIINIT_OPTION_USELED|WIFIINIT_OPTION_USEHEAP_64);
    REG_IPC_FIFO_TX=0x12345678;
    REG_IPC_FIFO_TX=Wifi_pass;

    *((volatile u16 *)0x0400010E) = 0; // disable timer3

#ifdef LOCAL_VBLANK
    irqInit(); 
#endif
    irqSet(IRQ_TIMER3, arm9_wifiTimer); // setup timer IRQ
    irqEnable(IRQ_TIMER3);
    irqSet(IRQ_FIFO_NOT_EMPTY, arm9_fifo); // setup fifo IRQ
    irqEnable(IRQ_FIFO_NOT_EMPTY);
#ifdef LOCAL_VBLANK
    irqSet(IRQ_VBLANK, onVblank);
    irqEnable(IRQ_VBLANK);
#endif
    REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ; // enable FIFO IRQ

    Wifi_SetSyncHandler(arm9_synctoarm7); // tell wifi lib to use our handler to notify arm7

    // set timer3
//    *((volatile u16 *)0x0400010C) = -6553; // 6553.1 * 256 cycles = ~50ms;
    *((volatile u16 *)0x0400010C) = -(131062*WifiTimerInterval_ms/1000); // 131062 * 256 cycles = ~1000ms;
    *((volatile u16 *)0x0400010E) = 0x00C2; // enable, irq, 1/256 clock
    
    while(Wifi_CheckInit()==0) 
    { // wait for arm7 to be initted successfully
        while(VCOUNT>192); // wait for vblank
        while(VCOUNT<192);
    }
        
     // wifi init complete - wifi lib can now be used!
//    Wifi_ScanMode();
//    swiWaitForVBlank();
}

void arm9_wifiAutoconnect(void)
{
        Wifi_DisconnectAP();
//    Wifi_SetIP(0, 0, 0, 0, 0);
    Wifi_AutoConnect(); // request connect
    
//    int status = ASSOCSTATUS_DISCONNECTED;
    
    while(1){
        int i = Wifi_AssocStatus(); // check status
        if(i == ASSOCSTATUS_ASSOCIATED) {
            struct in_addr my_ip;
//            unsigned long my_ip = Wifi_GetIP();
            my_ip.s_addr = Wifi_GetIP();
            PJ_LOG(3,(THIS_FILE, "ip address %s", inet_ntoa(my_ip)));
            break;
            }
        if(i==ASSOCSTATUS_CANNOTCONNECT) {
            PJ_LOG(3,(THIS_FILE, "cannot connect"));
            break;
        }
        /*if(status != i)
        {
          status = i;
          switch(staus)
          {
            case ASSOCSTATUS_DISCONNECTED: _consolePrintf("ASSOCSTATUS_DISCONNECTED\n"); break;
            case ASSOCSTATUS_SEARCHING: _consolePrintf("ASSOCSTATUS_SEARCHING\n"); break;
            case ASSOCSTATUS_AUTHENTICATING: _consolePrintf("ASSOCSTATUS_AUTHENTICATING\n"); break;
            case ASSOCSTATUS_ASSOCIATING: _consolePrintf("ASSOCSTATUS_ASSOCIATING\n"); break;
            case ASSOCSTATUS_ACQUIRINGDHCP: _consolePrintf("ASSOCSTATUS_ACQUIRINGDHCP\n"); break;
            case ASSOCSTATUS_ASSOCIATED: _consolePrintf("ASSOCSTATUS_ASSOCIATED\n"); break;
            case ASSOCSTATUS_CANNOTCONNECT:
                _consolePrintf("ASSOCSTATUS_CANNOTCONNECT\n");
                _consolePrintf("\n");
                _consolePrintf("Fatal error!! can not connect.\n");
                ShowLogHalt();
                break;
          }
        }
        if(status == ASSOCSTATUS_ASSOCIATED)
        {
            _consolePrintf("Connected successfully!\n");
            break;
        }*/
    }
}

void arm9_wifiDisconnect(void)
{
    Wifi_DisconnectAP();
    while(1)
    {
        int i = Wifi_AssocStatus(); // check status
        if(i == ASSOCSTATUS_DISCONNECTED)
        {
            PJ_LOG(3,(THIS_FILE, "disconnect"));
            break;
        }
    }
    Wifi_DisableWifi();
}
