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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nds.h>

#include "ipcex.h"

static s16 *mic;

static s16* microphone_buffer = NULL;
static int microphone_buffer_length = 0;
static int microphone_current_length = 0;

static bool microphone_request_copy = false;
static bool microphone_first_part = true;

//---------------------------------------------------------------------------------
// Read a byte from the microphone. Code based on neimod's example and WifiVoiceChat 1.5
//---------------------------------------------------------------------------------
static s32 MIC_ReadData12() {
//---------------------------------------------------------------------------------
	s32 result, result2;
	s32 snddata;

	SerialWaitBusy();

	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_MICROPHONE | SPI_BAUD_2MHz | SPI_CONTINUOUS;
	REG_SPIDATA = 0xE4;// 0xE4=12bit 0xEC=8bit  // Touchscreen command format for AUX

	SerialWaitBusy();

	REG_SPIDATA = 0x00;

	SerialWaitBusy();

	result = REG_SPIDATA;
  	REG_SPICNT = SPI_ENABLE | SPI_DEVICE_TOUCH | SPI_BAUD_2MHz;
	REG_SPIDATA = 0x00;

	SerialWaitBusy();

	result2 = REG_SPIDATA;
	
	result = result & 0x7f;
	result2 = (result2>>3)&0x1f;
	
	result = (result<<5) | result2; // u12bit
	snddata=((s32)result)-0x7ff; // s12bit;
	return(snddata);
}

/**
 */
static void _ProcessMicrophoneTimerIRQ() 
{
	if(microphone_buffer && microphone_buffer_length > 0) 
	{
		u32 cnt = microphone_current_length;
		microphone_buffer[cnt] = MIC_ReadData12();
		cnt++;
		// Ask to copy, and flip the buffer.
		if(cnt == microphone_buffer_length)
		{
			microphone_request_copy = true;
		}
		else if (cnt == microphone_buffer_length * 2)
		{
			cnt=0;
			microphone_request_copy = true;
		}
		microphone_current_length = cnt;
	}
}

/**
 */
void arm7_microProcess(void)
{
    int idx;
    s16 *psrc, *pdst;
	if(microphone_request_copy == false)
	{
		return;
	}
	microphone_request_copy = false;
	
	pdst=IPC2->micro_buf;
	if (microphone_first_part == true)
	{
		microphone_first_part = false;
		psrc = microphone_buffer;
	}
	else
	{
		microphone_first_part = true;
		psrc = microphone_buffer + microphone_buffer_length;
	}
	
	for(idx = 0; idx < microphone_buffer_length; ++idx)
	{
	    s32 data=((s32)psrc[idx])<<2; // 12bit -> 14bit
	    if(data<-0x1f00) data=-0x1f00;
	    if(0x1eff<data) data=0x1eff;
	    pdst[idx]=(s16)data;
	}

	IPC2->micro_readrequest = 1;
    IPC_SendSync(IPC2_REQUEST_READ_SOUND);	
}

/**
 */
void arm7_microStart(void)
{
    u32 freq;

    IPC2->sound_control = IPC2_SOUND_NOP;

    microphone_request_copy = false;
    microphone_first_part = true;
    microphone_buffer_length = IPC2->sound_samples;
    microphone_current_length = 0;

    memset(IPC2->micro_buf, 0, 
        microphone_buffer_length * IPC2->sound_bytes_per_sample);
    freq = IPC2->sound_frequency;

    microphone_buffer = mic = malloc(microphone_buffer_length * 2 * 
        IPC2->sound_bytes_per_sample);

    MIC_On();

    TIMER0_DATA = (0x10000 - ((16777216*2) / freq));
    TIMER0_CR = TIMER_DIV_1 | TIMER_IRQ_REQ | TIMER_ENABLE;

    irqEnable(IRQ_TIMER0);
    swiIntrWait(1, IRQ_TIMER0);
    irqSet(IRQ_TIMER0,_ProcessMicrophoneTimerIRQ);

    IPC2->sound_state = IPC2_RECORDING;
}

/**
 */
void arm7_microStop(void)
{
    IPC2->sound_control = IPC2_SOUND_NOP;
          
    irqClear(IRQ_TIMER0);

    TIMER0_CR = 0;

    MIC_Off();
    microphone_buffer = NULL;
    free(mic);
    IPC2->sound_state = IPC2_STOPPED;
}
