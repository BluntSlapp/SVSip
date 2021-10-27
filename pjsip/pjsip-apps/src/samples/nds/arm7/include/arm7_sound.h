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

#ifndef ARM7_SOUND_H_
#define ARM7_SOUND_H_

#include <nds.h>
#include <nds/arm7/serial.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void arm7_initSoundDevice(void);
//void arm7_soundStart(void);
//void arm7_soundStop(void);
//
//s32 getFreeSoundChannel();
//void startSound(int sampleRate, const void* data, u32 bytes, u8 channel, u8 vol,  u8 pan, u8 format);
void arm7_pcmplay(void);
void arm7_pcmstop(void);
void arm7_playclick(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*ARM7_SOUND_H_*/
