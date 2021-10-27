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

#include <pjlib.h>

#include <nds.h>
#include <dswifi9.h>


#include "sv_sprite.h"
#include "screen.h"
#include "text.h"
#include "myriad_lmp.h"

sv_sprite *battery;

void topbar_init()
{
}

static pj_bool_t topbar_update_battery(void)
{
    static pj_uint16_t bat = 0x7fff, aux = 0x7fff;

    if(bat == IPC->battery && aux == IPC->aux)
        return PJ_FALSE;

    bat = IPC->battery;
    aux = IPC->aux;

    if(bat == 0 && aux == 0) // Chargé
    {
        dmaCopy(battery->icons, battery->icon,
            battery->size * battery->size * 2);
    }
    else if(bat == 0 && aux != 0) // En charge
    {
        dmaCopy(battery->icons + 5 * battery->size * battery->size, battery->icon,
            battery->size * battery->size * 2);
    }
    else // Charger
    {
        dmaCopy(battery->icons + battery->size * battery->size, battery->icon, 
            battery->size * battery->size * 2);
    }

    return PJ_TRUE;
}

static pj_bool_t topbar_update_time()
{
    char txt[6];
    static pj_uint8_t hours = 0xff;
    static pj_uint8_t minutes = 0xff;

//    if (hours == IPC->time.rtc.hours && minutes == IPC->time.rtc.minutes)
//        return PJ_FALSE;
        
    hours = IPC->time.rtc.hours;
    minutes = IPC->time.rtc.minutes;
    
    pj_ansi_snprintf(txt, 6, "%02d:%02d", hours, minutes); 

    dispString2(screen_get_backbuffer(), 256,
                163 /* xpos */, 6 /* ypos */,
                0xffff /*currentskin.time_mask*/,
                txt, font_myriad_web_9,
                256 /*screen_width + screen_xpos - 40*/,
                16 /*screen_height + screen_ypos*/);

    screen_draw_rect(screen_get_backbuffer(), 256, 160, 3, 31, 15, 0xffff);
    
    return PJ_TRUE;
}

static void topbar_update_wifi(void)
{
#if 0    
    char txt[32];
    Wifi_AccessPoint ap;
    int i, num, l=8;
    num = Wifi_GetNumAP();
    pj_ansi_snprintf(txt, 32, "%d", num);
    printbtm(7,0, txt);
    for (i = 0; i < num; ++i)
    {
    Wifi_GetAPData(i, &ap);
    int quality = (ap.rssi*100)/0xD0;
    pj_ansi_snprintf(txt, 32, "%20s:%3d%%", ap.ssid, quality);
    l += i;
    printbtm(8+i,0, txt);
    if (l==12)
        l = 8;
    }
#endif    
}

static void topbar_update_sip(void)
{
}



void topbar_update()
{
    topbar_update_battery();
    topbar_update_time();
    topbar_update_wifi();
}
