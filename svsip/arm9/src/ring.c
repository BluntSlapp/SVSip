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

#include <pjsua-lib/pjsua.h>

#define THIS_FILE	"ring.c"

struct my_call_data
{
   pj_pool_t          *pool;
   pjmedia_port       *tonegen;
   pjsua_conf_port_id  toneslot;
};

void ring_init(pjsua_call_id call_id)
{
	struct my_call_data *cd;

	pj_pool_t *pool;
	//pjmedia_port       *tonegen;
    //pjsua_conf_port_id  toneslot;
	pjmedia_tone_desc   tones[1];
	
	pool = pjsua_pool_create("ring", 512, 512);
	cd = PJ_POOL_ZALLOC_T(pool, struct my_call_data);
	cd->pool = pool;
	
	pjmedia_tonegen_create(cd->pool, 8000, 1, /*8000 /  10*/64 /  10, 16, 0, &cd->tonegen);
	pjsua_conf_add_port(cd->pool, cd->tonegen, &cd->toneslot);
	pjsua_conf_connect(cd->toneslot, 0);
	
	tones[0].freq1 = 440;
	tones[0].freq2 = 480;
	tones[0].on_msec = 2000;
	tones[0].off_msec = 4000;
	tones[0].volume = 0;
	
	pjmedia_tonegen_play(cd->tonegen,  1, tones, PJMEDIA_TONEGEN_LOOP);
	pjsua_call_set_user_data(call_id, (void*) cd);
}

void ring_deinit(pjsua_call_id call_id)
{
  struct my_call_data *cd;

  cd = (struct my_call_data*) pjsua_call_get_user_data(call_id);
  if (!cd)
     return;

	if (cd->toneslot >= 0) 
	{
		pjsua_conf_remove_port(cd->toneslot);
	}
	if (cd->tonegen) 
	{
		pjmedia_tonegen_stop(cd->tonegen);
		pjmedia_port_destroy(cd->tonegen);
	}
	if (cd->pool) 
	{
		pj_pool_release(cd->pool);
	}
	pjsua_call_set_user_data(call_id, NULL);
}
