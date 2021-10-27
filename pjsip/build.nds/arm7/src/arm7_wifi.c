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
#include <dswifi7.h>

#include "arm7_wifi.h"


/**
 * Callback to allow wifi library to notify arm9
 */
void arm7_synctoarm9() 
{
   // send fifo message
   REG_IPC_FIFO_TX = 0x87654321;
}

/**
 * Interrupt handler to allow incoming notifications from arm9
 */
void arm7_fifo() 
{ 
   // check incoming fifo messages
   u32 msg = REG_IPC_FIFO_RX;
   if(msg==0x87654321) Wifi_Sync();
}

/**
 */
void arm7_initWifi(void)
{
    irqSet(IRQ_WIFI, Wifi_Interrupt); // set up wifi interrupt
    irqEnable(IRQ_WIFI);

    { 
    // sync with arm9 and init wifi
    u32 fifo_temp;   

      while(1) 
      { 
        // wait for magic number
        while(REG_IPC_FIFO_CR&IPC_FIFO_RECV_EMPTY)
            swiWaitForVBlank();
        fifo_temp=REG_IPC_FIFO_RX;
        if(fifo_temp==0x12345678) 
            break;
      }
    while(REG_IPC_FIFO_CR&IPC_FIFO_RECV_EMPTY)
        swiWaitForVBlank();

    fifo_temp=REG_IPC_FIFO_RX; // give next value to wifi_init
    Wifi_Init(fifo_temp);
    
    irqSet(IRQ_FIFO_NOT_EMPTY,arm7_fifo); // set up fifo irq
    irqEnable(IRQ_FIFO_NOT_EMPTY);
    REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ;

    Wifi_SetSyncHandler(arm7_synctoarm9); // allow wifi lib to notify arm9
  } // arm7 wifi init complete
}
