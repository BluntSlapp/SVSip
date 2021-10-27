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

/*
 * Code inspiré de DSFtp et de LMP-ng.
 */
 
#include <nds.h>
#include "ipcex.h" 
#include "screensaver.h"


static pj_bool_t  ssaverOn = PJ_FALSE;
static pj_uint32_t ssaverTimeout = 10;

/*
 * Demande l'extinction des écrans.
  */
PJ_DEF(void) screensaver_start()
{
	if (ssaverOn != PJ_TRUE)
	{
		IPC_SendSync(IPC2_REQUEST_SET_BACKLIGHTS_OFF);
		ssaverOn = PJ_TRUE;
		swiWaitForVBlank(); /* HACK */	
	}
}

/*
 * Demande l'allumage des écrans.
 */
PJ_DEF(void) screensaver_stop()
{
	if (ssaverOn != PJ_FALSE)
	{
		IPC_SendSync(IPC2_REQUEST_SET_BACKLIGHTS_ON);
		ssaverOn = PJ_FALSE;
		swiWaitForVBlank(); /* HACK */
	}
}

/*
 * Définit le délai avant que l'économiseur d'écran s'active.
 * @param seconds délai avant que l'économiseur d'écran s'active. 0 désactive l'économiseur d'écran
 */
PJ_DEF(void) screensaver_set_timeout(pj_uint32_t seconds)
{
	ssaverTimeout = seconds;
}

PJ_DEF(void) screensaver_reset()
{
}
