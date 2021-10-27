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

 /*
  * Inspired by lmp-ng
  */

#include <string.h>

#include <nds.h>

#include "arm7_sound.h"
#include "ipcex.h"

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) < (b)) ? (b) : (a))

static char pcmL[4096], pcmR[4096];
static void *srcpcmL, *srcpcmR;
static u8 buffer; // Indice du buffer à utiliser, (3 en tout)
static u8 channels, bytes_per_sample;
static u32 freq, samples;
static u8 multiple;
static s16 llast, rlast;
static s16 *table;


/**
 */
void arm7_initSoundDevice(void)
{
//  //enable sound
//  powerON(POWER_SOUND);
//  SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
//  
//  swiChangeSoundBias(1,0x400);
//  {
//  //a7SetSoundAmplifier(true);
////    u8 control = PM_GetRegister(0) & ~PM_SOUND_PWR;
////    if(e==true) control|=PM_SOUND_PWR;
////    PM_SetRegister(0, control&255);
//    u8 control = readPowerManagement(0) & ~PM_SOUND_PWR;
//    control|=PM_SOUND_PWR;
//    writePowerManagement(0, control&255);
//  }
//
//  IPC->soundData = 0;
//  IPCEX->command = NOP;

    //enable sound
    powerON(POWER_SOUND);
    SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
}

/**
 */
static s8 InterruptHandler_TIMER3_Volume_calculate8(s8 sample) {
    u16 volume = IPC2->sound_volume;

    /* 8 bit gets the poor sound amplifier */

    return
    (s8) MIN(MAX((((s32) sample * (s32) volume) / (s32) 0x3fff), -128), 127);
}

/**
 */
static s16 InterruptHandler_TIMER3_Volume_calculate16(s16 sample) {
    unsigned index, sub;
    s32 fractional, high, low;

    index = (sample + 32768) >> 8;
    sub = sample & 0xff;
    high = table[index + 1];
    low = table[index];
    fractional = (high - low) * sub;

    return low + (fractional >> 8);
}

/**
 */
static void InterruptHandler_TIMER3_Volume(char *left, char *right) {
    int i;
    s8 *t8;
    s16 *t16;
    static u16 lastvol = 0;

    if(bytes_per_sample == 2 && lastvol != IPC2->sound_volume) {
        if(IPC2->sound_volume <= 0x3fff) {
            unsigned newvol;

            newvol = (0x7f * IPC2->sound_volume) / 0x3fff;

            SCHANNEL_CR(0) = SCHANNEL_ENABLE | SOUND_REPEAT | SOUND_VOL(newvol) | SOUND_PAN(0x0) | SOUND_16BIT;
            SCHANNEL_CR(1) = SCHANNEL_ENABLE | SOUND_REPEAT | SOUND_VOL(newvol) | SOUND_PAN(0x7f) | SOUND_16BIT;
        } else {
            if(lastvol < 0x3fff) {
                SCHANNEL_CR(0) = SCHANNEL_ENABLE | SOUND_REPEAT | SOUND_VOL(0x7f) | SOUND_PAN(0x0) | SOUND_16BIT;
                SCHANNEL_CR(1) = SCHANNEL_ENABLE | SOUND_REPEAT | SOUND_VOL(0x7f) | SOUND_PAN(0x7f) | SOUND_16BIT;
            }

            table = IPC2->sound_tables[((IPC2->sound_volume >> 8) - 64)];
        }

        lastvol = IPC2->sound_volume;
    }

    if(IPC2->sound_volume <= 0x3fff)
        return;

    if(bytes_per_sample == 1) {
        t8 = (s8 *) left;
        for(i = 0; i < samples; i++)
            t8[i] = InterruptHandler_TIMER3_Volume_calculate8(t8[i]);

        if(channels == 2) {
            t8 = (s8 *) right;
            for(i = 0; i < samples; i++)
                t8[i] = InterruptHandler_TIMER3_Volume_calculate8(t8[i]);
        }
    } else if(bytes_per_sample == 2) {
        t16 = (s16 *) left;
        for(i = 0; i < samples; i++)
            t16[i] = InterruptHandler_TIMER3_Volume_calculate16(t16[i]);

        if(channels == 2) {
            t16 = (s16 *) right;
            for(i = 0; i < samples; i++)
                t16[i] = InterruptHandler_TIMER3_Volume_calculate16(t16[i]);
        }
    }
}

/**
 */
static void InterruptHandler_TIMER3_OverSampling8_2x(u32 samples, s8 *lbuf, s8 *rbuf, s8 llast, s8 rlast) {
    int i;
    s8 *lsrc, *rsrc;

    lsrc = lbuf + samples;
    rsrc = rbuf + samples;

    if(channels == 1) {
        lbuf[0] = llast/2 + lsrc[0]/2;
        lbuf[1] = lsrc[0];

        for(i=1; i<samples; i++) {
            lbuf[2 * i] = lsrc[i-1]/2 + lsrc[i]/2;
            lbuf[2 * i + 1] = lsrc[i];
        }

    } else if(channels == 2) {
        lbuf[0] = llast/2 + lsrc[0]/2;
        lbuf[1] = lsrc[0];

        rbuf[0] = rlast/2 + rsrc[0]/2;
        rbuf[1] = rsrc[0];

        for(i=1; i<samples; i++) {
            lbuf[2 * i] = lsrc[i-1]/2 + lsrc[i]/2;
            lbuf[2 * i + 1] = lsrc[i];
            rbuf[2 * i] = rsrc[i-1]/2 + rsrc[i]/2;
            rbuf[2 * i + 1] = rsrc[i];
        }
    }
}

/**
 */
static void InterruptHandler_TIMER3_OverSampling8(s8 *lbuf, s8 *rbuf) {
    switch(multiple) {
        case 1:
            break;

        case 2:
            InterruptHandler_TIMER3_OverSampling8_2x(samples, lbuf, rbuf, llast, rlast);
            break;

        case 4:
            InterruptHandler_TIMER3_OverSampling8_2x(samples, lbuf + 2 * samples, rbuf + 2 * samples, llast, rlast);
            InterruptHandler_TIMER3_OverSampling8_2x(2 * samples, lbuf, rbuf, llast, rlast);
            break;

        default:
            break;
    }

    if(channels == 1)
        llast = lbuf[samples - 1];
    else if(channels == 2) {
        llast = lbuf[multiple * samples - 1];
        rlast = rbuf[multiple * samples - 1];
    }
}

/**
 */
static void InterruptHandler_TIMER3_OverSampling16_2x(u32 samples, s16 *lbuf, s16 *rbuf, s16 llast, s16 rlast) {
    int i;
    s16 *lsrc, *rsrc;

    lsrc = lbuf + samples;
    rsrc = rbuf + samples;

    if(channels == 1) {
        lbuf[0] = llast/2 + lsrc[0]/2;
        lbuf[1] = lsrc[0];

        for(i=1; i<samples; i++) {
            lbuf[2 * i] = lsrc[i-1]/2 + lsrc[i]/2;
            lbuf[2 * i + 1] = lsrc[i];
        }

    } else if(channels == 2) {
        lbuf[0] = llast/2 + lsrc[0]/2;
        lbuf[1] = lsrc[0];

        rbuf[0] = rlast/2 + rsrc[0]/2;
        rbuf[1] = rsrc[0];

        for(i=1; i<samples; i++) {
            lbuf[2 * i] = lsrc[i-1]/2 + lsrc[i]/2;
            lbuf[2 * i + 1] = lsrc[i];
            rbuf[2 * i] = rsrc[i-1]/2 + rsrc[i]/2;
            rbuf[2 * i + 1] = rsrc[i];
        }
    }
}

/**
 */
static void InterruptHandler_TIMER3_OverSampling16(s16 *lbuf, s16 *rbuf) {
    switch(multiple) {
        case 1:
            break;

        case 2:
            InterruptHandler_TIMER3_OverSampling16_2x(samples, lbuf, rbuf, llast, rlast);
            break;

        case 4:
            InterruptHandler_TIMER3_OverSampling16_2x(samples, lbuf + 2 * samples, rbuf + 2 * samples, llast, rlast);
            InterruptHandler_TIMER3_OverSampling16_2x(2 * samples, lbuf, rbuf, llast, rlast);
            break;

        default:
            break;
    }

    if(channels == 1)
        llast = lbuf[multiple * samples - 1];
    else if(channels == 2) {
        llast = lbuf[multiple * samples - 1];
        rlast = rbuf[multiple * samples - 1];
    }
}

/**
 */
static void InterruptHandler_TIMER3(void) 
{
    if(IPC2->sound_writerequest == 1) 
    {
//        if(IPC2->messageflag2 < IPC2_MAX_MESSAGES)
//            strcpy(IPC2->message2[IPC2->messageflag2++], "error: buffer underrun");

        memset(&pcmL[(multiple - 1) * samples * bytes_per_sample + 
            buffer * multiple * samples * bytes_per_sample], 
            0, samples * bytes_per_sample);
        memset(&pcmR[(multiple - 1) * samples * bytes_per_sample + 
            buffer * multiple * samples * bytes_per_sample], 
            0, samples * bytes_per_sample);

        IPC_SendSync(IPC2_REQUEST_WRITE_SOUND);
    }

    if(IPC2->sound_writerequest == 0) 
    {
        memcpy(&pcmL[(multiple - 1) * samples * bytes_per_sample + 
            buffer * multiple * samples * bytes_per_sample], 
            srcpcmL, samples * bytes_per_sample);
        if(channels == 2)
            memcpy(&pcmR[(multiple - 1) * samples * bytes_per_sample + 
                buffer * multiple * samples * bytes_per_sample], 
                srcpcmR, samples * bytes_per_sample);

        IPC2->sound_writerequest = 1;
        IPC_SendSync(IPC2_REQUEST_WRITE_SOUND);

//        InterruptHandler_TIMER3_Volume(&pcmL[(multiple - 1) * samples * bytes_per_sample + buffer * multiple * samples * bytes_per_sample], &pcmR[(multiple - 1) * samples * bytes_per_sample + buffer * multiple * samples * bytes_per_sample]);

        if(bytes_per_sample == 1)
            InterruptHandler_TIMER3_OverSampling8((s8 *) (&pcmL[buffer * multiple * samples * bytes_per_sample]), (s8 *) (&pcmR[buffer * multiple * samples * bytes_per_sample]));
        else if(bytes_per_sample == 2)
            InterruptHandler_TIMER3_OverSampling16((s16 *) (&pcmL[buffer * multiple * samples * bytes_per_sample]), (s16 *) (&pcmR[buffer * multiple * samples * bytes_per_sample]));

    }
    buffer = (1 + buffer) % 3; /* Changement de buffer */
}

/**
 */
void arm7_pcmplay(void) 
{
    int i;

    /* On devrait allouer pcmL et pcmR en utilisant comme taille :
     * samples * bytes_per_sample * (bufferMax = 3)
     */

    IPC2->sound_control = IPC2_SOUND_NOP;
    channels = IPC2->sound_channels;
    freq = IPC2->sound_frequency;
    samples = IPC2->sound_samples;
    bytes_per_sample = IPC2->sound_bytes_per_sample;
    llast = rlast = 0;
    buffer = 0;
    IPC2->sound_arm7_mode = IPC2->sound_mode;

    switch(IPC2->sound_mode) {
        default:
        case IPC2_SOUND_MODE_PCM_NORMAL:
            multiple = 1;
            break;

        case IPC2_SOUND_MODE_PCM_OVERSAMPLING2x:
            multiple = 2;
            break;

        case IPC2_SOUND_MODE_PCM_OVERSAMPLING4x:
            multiple = 4;
            break;
    }

    srcpcmL = IPC2->sound_lbuf;
    srcpcmR = IPC2->sound_rbuf;

    TIMER2_DATA = SOUND_FREQ((int) (multiple * freq));
    TIMER2_CR = TIMER_DIV_1 | TIMER_ENABLE;

    TIMER3_DATA = 0x10000 - (multiple * samples * 2);
    TIMER3_CR = TIMER_CASCADE | TIMER_IRQ_REQ | TIMER_ENABLE;

    for(i = 0; i < 2; i++) 
    {
        SCHANNEL_CR(i) = 0;
        SCHANNEL_TIMER(i) = SOUND_FREQ((int) (multiple * freq));
        SCHANNEL_LENGTH(i) = (multiple * 3 * samples * bytes_per_sample) >> 2;
        SCHANNEL_REPEAT_POINT(i) = 0;
    }

//    IPC2->sound_writerequest = 1;
//    IPC_SendSync(IPC2_REQUEST_WRITE_SOUND);
//    while(IPC2->sound_writerequest == 1);
//    swiWaitForVBlank();
//    InterruptHandler_TIMER3();
//    while(IPC2->sound_writerequest == 1);
//    swiWaitForVBlank();
//    InterruptHandler_TIMER3();
//    while(IPC2->sound_writerequest == 1);
//    swiWaitForVBlank();

    irqEnable(IRQ_TIMER3);
    swiIntrWait(1, IRQ_TIMER3);
    irqSet(IRQ_TIMER3, InterruptHandler_TIMER3);

    SCHANNEL_CR(0) = 0;
    SCHANNEL_SOURCE(0) = (u32) pcmL;
    SCHANNEL_CR(1) = 0;
    SCHANNEL_SOURCE(1) = (u32) ((channels == 2) ? pcmR : pcmL);
    SCHANNEL_CR(0) = SCHANNEL_ENABLE | SOUND_REPEAT | SOUND_VOL(0x7F) | SOUND_PAN(0x0) | ((bytes_per_sample == 2) ? SOUND_16BIT : SOUND_8BIT);
    SCHANNEL_CR(1) = SCHANNEL_ENABLE | SOUND_REPEAT | SOUND_VOL(0x7F) | SOUND_PAN(0x7F) | ((bytes_per_sample == 2) ? SOUND_16BIT : SOUND_8BIT);

    IPC2->sound_state = IPC2_PLAYING;
}

/**
 */
void arm7_pcmstop(void) 
{
    IPC2->sound_control = IPC2_SOUND_NOP;

    irqClear(IRQ_TIMER3);

    TIMER2_CR = 0;
    TIMER3_CR = 0;

    SCHANNEL_CR(0) = 0;
    SCHANNEL_CR(1) = 0;

    IPC2->sound_state = IPC2_STOPPED;
}
