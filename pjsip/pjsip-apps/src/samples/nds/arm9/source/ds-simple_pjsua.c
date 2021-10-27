/* $Id$ */
/* 
 * Copyright (C) 2003-2007 Benny Prijono <benny@prijono.org>
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

/**
 * simple_pjsua.c
 *
 * This is a very simple but fully featured SIP user agent, with the 
 * following capabilities:
 *  - SIP registration
 *  - Making and receiving call
 *  - Audio/media to sound device.
 *
 * Usage:
 *  - To make outgoing call, start simple_pjsua with the URL of remote
 *    destination to contact.
 *    E.g.:
 *	 simpleua sip:user@remote
 *
 *  - Incoming calls will automatically be answered with 200.
 *
 * This program will quit once it has completed a single call.
 */

#include <pjsua-lib/pjsua.h>

#include <nds.h>
#include <fat.h>
#include "arm9_console.h"
#include "arm9_wifi.h"

#define THIS_FILE	"APP"

#define SIP_DOMAIN	""
#define SIP_USER	""
#define SIP_PASSWD	""


/* Callback called by the library upon receiving incoming call */
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,
			     pjsip_rx_data *rdata)
{
    pjsua_call_info ci;

    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(rdata);

    pjsua_call_get_info(call_id, &ci);

    PJ_LOG(3,(THIS_FILE, "Incoming call from %.*s!!",
			 (int)ci.remote_info.slen,
			 ci.remote_info.ptr));

    /* Automatically answer incoming calls with 200/OK */
    pjsua_call_answer(call_id, 200, NULL, NULL);
}

/* Callback called by the library when call's state has changed */
static void on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
    pjsua_call_info ci;

    PJ_UNUSED_ARG(e);

    pjsua_call_get_info(call_id, &ci);
    PJ_LOG(3,(THIS_FILE, "Call %d state=%.*s", call_id,
			 (int)ci.state_text.slen,
			 ci.state_text.ptr));
}

/* Callback called by the library when call's media state has changed */
static void on_call_media_state(pjsua_call_id call_id)
{
    pjsua_call_info ci;

    pjsua_call_get_info(call_id, &ci);

    if (ci.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
	// When media is active, connect call to sound device.
	pjsua_conf_connect(ci.conf_slot, 0);
	pjsua_conf_connect(0, ci.conf_slot);
    }
}

/* Display error and exit application */
static void error_exit(const char *title, pj_status_t status)
{
    pjsua_perror(THIS_FILE, title, status);
    pjsua_destroy();
    exit(1);
}

/*
 * main()
 *
 * argv[1] may contain URL to call.
 */
int main(int argc, char *argv[])
{
    pjsua_acc_id acc_id;
    pj_status_t status;
    
	//char *number = argv[1];
	//int argcnt  = argc;
	char *number = "sip:numbre@registrar";
	int argcnt  = 2;
	
    powerON(/*POWER_ALL*/POWER_LCD);
	// install the default exception handler
    defaultExceptionHandler();
//    initSystem();
    consoleDemoInit();
    irqInit();
irqEnable(IRQ_VBLANK);
    fatInitDefault();
    arm9_wifiInit();
    arm9_wifiAutoconnect();

    /* Create pjsua first! */
    status = pjsua_create();
    if (status != PJ_SUCCESS) error_exit("Error in pjsua_create()", status);

    /* If argument is specified, it's got to be a valid SIP URL */
    if (argcnt > 1) {
	status = pjsua_verify_sip_url(number);
	if (status != PJ_SUCCESS) error_exit("Invalid URL in argv", status);
    }

    /* Init pjsua */
    {
	pjsua_config cfg;
	pjsua_logging_config log_cfg;
	pjsua_media_config	 media_cfg;

	pjsua_config_default(&cfg);
	cfg.thread_cnt = 0;
	cfg.cb.on_incoming_call = &on_incoming_call;
	cfg.cb.on_call_media_state = &on_call_media_state;
	cfg.cb.on_call_state = &on_call_state;
//    cfg.user_agent = pj_str("PJSUA v0.7.0-trunk/arm-nds");

	pjsua_logging_config_default(&log_cfg);
	log_cfg.console_level = 4;
    log_cfg.log_filename = pj_str("logsimple-pjsua.txt");

	pjsua_media_config_default(&media_cfg);
	media_cfg.thread_cnt = 0;
	media_cfg.has_ioqueue = PJ_FALSE;
    media_cfg.clock_rate = 8000;
	
	status = pjsua_init(&cfg, &log_cfg, &media_cfg);
	if (status != PJ_SUCCESS) error_exit("Error in pjsua_init()", status);
    }

    /* Add UDP transport. */
    {
	pjsua_transport_config cfg;

	pjsua_transport_config_default(&cfg);
	cfg.port = 5060;
	status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &cfg, NULL);
	if (status != PJ_SUCCESS) error_exit("Error creating transport", status);
    }

    /* Initialization is done, now start pjsua */
    status = pjsua_start();
    if (status != PJ_SUCCESS) error_exit("Error starting pjsua", status);
//pjsua_handle_events(100);
    /* Register to SIP server by creating SIP account. */
    {
	pjsua_acc_config cfg;

	pjsua_acc_config_default(&cfg);
	cfg.id = pj_str("sip:" SIP_USER "@" SIP_DOMAIN);
	cfg.reg_uri = pj_str("sip:" SIP_DOMAIN);
	cfg.cred_count = 1;
    cfg.reg_timeout = 1800;
    //cfg.force_contact = pj_str("<sip:number@address:5060;transport=UDP>");
    
	cfg.cred_info[0].realm = pj_str(SIP_DOMAIN);
	cfg.cred_info[0].scheme = pj_str("digest");
	cfg.cred_info[0].username = pj_str(SIP_USER);
	cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
	cfg.cred_info[0].data = pj_str(SIP_PASSWD);

	status = pjsua_acc_add(&cfg, PJ_TRUE, &acc_id);
	if (status != PJ_SUCCESS) error_exit("Error adding account", status);
    }
//pjsua_handle_events(100);

    /* Wait until user press "A" to quit. */
while(1) {
	 scanKeys();
	 uint32 key = keysDown(); 
	 if (key & KEY_A)
	 {
		break;
	 }
     if (key & KEY_B)
     {
        pjsua_call_hangup_all();
        //break;
     }

     if (key & KEY_Y)
     {
        PJ_LOG(3,(THIS_FILE, "Make call immediately: %s...", number));
        if (argcnt > 1) 
        {
            pj_str_t uri = pj_str(number);
            status = pjsua_call_make_call(acc_id, &uri, 0, NULL, NULL, NULL);
            if (status != PJ_SUCCESS) 
                error_exit("Error making call", status);
        }
     }

    pjsua_handle_events(1);
//    swiWaitForVBlank();
    swiWaitForIRQ();
}	

    /* Destroy pjsua */
    pjsua_destroy();

    return 0;
}
