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

#ifndef IPCEX_H_
#define IPCEX_H_

//#if defined(ARM9)
//extern volatile int tick9;
//extern volatile int tick7;
//#endif
//
///*
//  Structures and functions to allow the ARM9 to send commands to the
//  ARM7. Based on code from the "Microphone Demo" and "Wifi voice Chat"
//*/
//
///* Enumeration of commands that the ARM9 can send to the ARM7 */
//enum CommandType {
//  NOP,
//  PLAY_ONE_SHOT_SAMPLE,
//  START_PLAYING,
//  STOP_PLAYING,
//  START_RECORDING,
//  STOP_RECORDING
//};
//
//enum IrType {
//    IR_NULL,
//    IR_audio,
//    IR_mic
//};
//
//typedef struct sTransferRegionEX 
//{
///*  u32 IR;
//    vu32 strpcmControl;*/
///*  IrType IR;
//  CommandType command;*/
//  u32 IR;
//  vu32 command;
//
//  s16 *pmicBuf;
//  int  micBufLength;
//
//  vu32 strpcmFreq,strpcmSamples;
//  s16 *pstrpcmBuf;
////  int audioBufLength;
//  
//} TransferRegionEX, * pTransferRegionEX;

/* from ARM7 to ARM9 */
#define IPC2_REQUEST_WRITE_SOUND 1
#define IPC2_REQUEST_READ_SOUND  2

/* from ARM9 to ARM7 */
#define IPC2_REQUEST_START_PLAYING   1
#define IPC2_REQUEST_STOP_PLAYING    2
#define IPC2_REQUEST_START_RECORDING 3
#define IPC2_REQUEST_STOP_RECORDING  4
#define IPC2_REQUEST_SET_BACKLIGHTS_OFF 5
#define IPC2_REQUEST_SET_BACKLIGHTS_ON  6
#define IPC2_REQUEST_LEDBLINK_OFF       7
#define IPC2_REQUEST_LEDBLINK_ON        8

#define IPC2_SOUND_NOP                  0
#define IPC2_SOUND_START                1
#define IPC2_SOUND_STOP                 2
#define IPC2_SOUND_CLICK                3
#define IPC2_RECORD_START               4
#define IPC2_RECORD_STOP                5



#define IPC2_STOPPED 1
#define IPC2_PLAYING 2
#define IPC2_RECORDING 3

#define IPC2_SOUND_MODE_PCM_OVERSAMPLING4x 1
#define IPC2_SOUND_MODE_PCM_OVERSAMPLING2x 2
#define IPC2_SOUND_MODE_PCM_NORMAL 3


typedef struct sTransferRegionEX 
{
//    u16 ms;
    u8 micro_readrequest;
    void *micro_buf;
    
    void *sound_lbuf, *sound_rbuf;

    u8 sound_control;
    u8 sound_mode;
    u8 sound_state;
    
    u8  sound_channels;
    u16 sound_frequency;
    u8  sound_bytes_per_sample;
    u16 sound_samples;

    u8 sound_writerequest;

    u16 sound_volume;
    s16 **sound_tables;

    u8 sound_arm7_mode;
} TransferRegionEX, * pTransferRegionEX;

/* Address of the shared CommandControl structure */
#define IPC2 ((TransferRegionEX volatile*)((uint32)(IPC) + sizeof(TransferRegion)))

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*IPCEX_H_*/
