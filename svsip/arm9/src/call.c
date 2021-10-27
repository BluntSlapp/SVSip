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

#include <netinet/in.h>

#include <nds.h>
#include <dswifi9.h>
#include <fat.h>

#include "arm9_wifi.h"
#include "call.h"





#define THIS_FILE	"call.c"

#define PORT 5060
#define REALM "*"

char *sip_domain = NULL;

static pjsip_inv_state phone_state  = PJSIP_INV_STATE_NULL;
static pjsua_call_id   current_call = PJSUA_INVALID_ID;


static struct app_config
{
	pj_pool_t		      *pool;

	pjsua_config	       cfg;
	pjsua_logging_config   log_cfg;
	pjsua_media_config	   media_cfg;
	
	pjsua_transport_config udp_cfg;
	
	pjsua_acc_config       acc_cfg;
	
	float mic_level;
	float speaker_level;
} app_config;

static
void phone_set_state(pjsip_inv_state newstate)
{
	if (phone_state == newstate)
		return;
	
	phone_state = newstate;
}

pjsip_inv_state phone_getstate()
{
	return phone_state;
}

/* Callback called by the library upon receiving incoming call */
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,
			     pjsip_rx_data *rdata)
{
    pjsua_call_info ci;

    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(rdata);

    pjsua_call_get_info(call_id, &ci);
	//phone_set_state(ci.state);
    PJ_LOG(1,(THIS_FILE, "Incoming call from %.*s!!",
			 (int)ci.remote_info.slen,
			 ci.remote_info.ptr));

	printbtm(4, 0, "Incoming call from:");
	printbtm(5,0, ci.remote_info.ptr);
			 
	// Réveil l'écran si nécessaire
	screensaver(KEY_LID);
	
    /* Automatically answer incoming calls with 200/OK */
    //pjsua_call_answer(call_id, 200, NULL, NULL);
	pjsua_call_answer(call_id, 180, NULL, NULL);

	ring_init(call_id);
	/*PJ_LOG(3,(THIS_FILE,
		  "Incoming call for account %d!\n"
		  "From: %s\n"
		  "To: %s\n"
		  "Press a to answer or h to reject call",
		  acc_id,
		  ci.remote_info.ptr,
		  ci.local_info.ptr));*/
}

/* Callback called by the library when call's state has changed */
static void on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
    pjsua_call_info ci;

    PJ_UNUSED_ARG(e);

    pjsua_call_get_info(call_id, &ci);
	phone_set_state(ci.state);
    PJ_LOG(1,(THIS_FILE, "Call %d state=%.*s", call_id,
			 (int)ci.state_text.slen,
			 ci.state_text.ptr));
	printbtm(2,0, "state:");
	printbtm(2,8, ci.state_text.ptr);
	/* Déconnexion */
	if (ci.state == PJSIP_INV_STATE_DISCONNECTED) 
	{
	  /* Fin dtmf */
      call_deinit_tonegen(call_id);
	  current_call = PJSUA_INVALID_ID;
	  printbtm(4, 0, "");
	  printbtm(5, 0, "");
	  printbtm(6, 0, "");
    }
	else 
	{
	  if (current_call == PJSUA_INVALID_ID)
	    current_call = call_id;
	}
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
/*static void error_exit(const char *title, pj_status_t status)
{
    pjsua_perror(THIS_FILE, title, status);
    pjsua_destroy();
    exit(1);
}*/

/* Set default config. */
static void default_config(struct app_config *cfg)
{
    //char tmp[80];
//	unsigned long dns1 = 0, dns2 = 0;
//	pj_in_addr in;
	
	pjsua_config_default(&cfg->cfg);
	cfg->cfg.thread_cnt = 0;
	//pj_ansi_snprintf(tmp, 80, "SvSip v%s/%s", pj_get_version(), PJ_OS_NAME);
    //pj_strdup2_with_null(app_config.pool, &cfg->cfg.user_agent, tmp);

	/* if (Wifi_GetIPInfo(NULL, NULL, &dns1, &dns2) != 0)
	 {
		 cfg->cfg.nameserver_count = 0;
		 if (dns1 != 0)
		 {
			 cfg->cfg.nameserver_count = 1;
			 in.s_addr = dns1;
			 pj_strdup2_with_null(app_config.pool, &(cfg->cfg.nameserver[0]), pj_inet_ntoa(in));
			 if (dns2 != 0)
			 {
				cfg->cfg.nameserver_count = 2;
				in.s_addr = dns2;
				pj_strdup2_with_null(app_config.pool, &(cfg->cfg.nameserver[1]), pj_inet_ntoa(in));
			 }
		 }
	 }*/

	pjsua_logging_config_default(&cfg->log_cfg);
	cfg->log_cfg.console_level = 0;

	pjsua_media_config_default(&cfg->media_cfg);
	cfg->media_cfg.thread_cnt = 0;
	cfg->media_cfg.has_ioqueue = PJ_FALSE;
    cfg->media_cfg.clock_rate = 8000;

	pjsua_transport_config_default(&cfg->udp_cfg);
	cfg->udp_cfg.port = PORT;
	
	pjsua_acc_config_default(&cfg->acc_cfg);
	cfg->acc_cfg.cred_count = 1;	

	cfg->mic_level = 3.0;
	cfg->speaker_level = 1.0;	
}


//int init_pjsua_acc(pjsua_acc_config *cfg, const char *config_file)
int parse_args(struct app_config *cfg, const char *config_file)
{
	int argc = 0;
	char **argv;
	pj_status_t status;
	int c;
	int option_index;
	char *sip_username = NULL;

enum { OPT_LOG_FILE=127, OPT_LOG_LEVEL, OPT_DOMAIN, OPT_REG_TIMEOUT, OPT_REALM,
	OPT_USERNAME, OPT_PASSWORD, OPT_MIC_LEVEL, OPT_SPEAKER_LEVEL,
    OPT_NAMESERVER, OPT_USE_ICE};

struct pj_getopt_option long_options[] = {
	{ "log-file",	1, 0, OPT_LOG_FILE},
	{ "log-level",	1, 0, OPT_LOG_LEVEL},
	{ "domain", 1, 0, OPT_DOMAIN},
	{ "reg-timeout", 1, 0, OPT_REG_TIMEOUT},
	{ "realm", 1, 0, OPT_REALM},
	{ "username",	1, 0, OPT_USERNAME},
	{ "password",	1, 0, OPT_PASSWORD},
	{ "mic-level", 1, 0, OPT_MIC_LEVEL},
	{ "speaker-level", 1, 0, OPT_SPEAKER_LEVEL},
    { "nameserver", 1, 0, OPT_NAMESERVER},
    { "use-ice",    0, 0, OPT_USE_ICE},
	{ NULL, 0, 0, 0}
    };

	status = read_config_file(config_file, &argc, &argv);
	if (status != 0)
	    return status;

	 while((c=pj_getopt_long(argc,argv, "", long_options,&option_index))!=-1) 
	 {
		pj_str_t tmp;

		switch (c) 
		{
			case OPT_LOG_FILE:
				cfg->log_cfg.log_filename = pj_str(pj_optarg);
				break;

			case OPT_LOG_LEVEL:
				c = pj_strtoul(pj_cstr(&tmp, pj_optarg));
				if (c < 0 || c > 6) 
				{
					PJ_LOG(1,(THIS_FILE, "Error: expecting integer value 0-6 for --log-level"));
					return PJ_EINVAL;
				}
				cfg->log_cfg.level = c;
				pj_log_set_level( c );
				break;
		
			/*case OPT_REGISTRAR:
				if (pjsua_verify_sip_url(pj_optarg) != 0) 
				{
					PJ_LOG(1,(THIS_FILE,  "Error: invalid SIP URL '%s' in registrar argument", pj_optarg));
					return PJ_EINVAL;
				}
				cfg->reg_uri = pj_str(pj_optarg);
				break;*/
			case OPT_DOMAIN:
				sip_domain = pj_optarg;
				break;
			case OPT_REG_TIMEOUT:
				cfg->acc_cfg.reg_timeout = pj_strtoul(pj_cstr(&tmp,pj_optarg));
				if (cfg->acc_cfg.reg_timeout < 1 || cfg->acc_cfg.reg_timeout > 3600) 
				{
					PJ_LOG(1,(THIS_FILE, "Error: invalid value for --reg-timeout (expecting 1-3600)"));
					return PJ_EINVAL;
				}
				break;
			/*case OPT_ID:
				if (pjsua_verify_sip_url(pj_optarg) != 0) 
				{
					PJ_LOG(1,(THIS_FILE, "Error: invalid SIP URL '%s' in local id argument", pj_optarg));
					return PJ_EINVAL;
				}
				cfg->id = pj_str(pj_optarg);
				break;*/
			/*case OPT_CONTACT:
				if (pjsua_verify_sip_url(pj_optarg) != 0) 
				{
					PJ_LOG(1,(THIS_FILE, "Error: invalid SIP URL '%s' in contact argument", pj_optarg));
					return PJ_EINVAL;
				}
				cfg->force_contact = pj_str(pj_optarg);
				break;*/
			case OPT_REALM:
				cfg->acc_cfg.cred_info[0].realm = pj_str(pj_optarg);
				break;
			case OPT_USERNAME:
				sip_username = pj_optarg;
				cfg->acc_cfg.cred_info[0].username = pj_str(pj_optarg);
				cfg->acc_cfg.cred_info[0].scheme = pj_str("Digest");
				break;
			case OPT_PASSWORD:
				cfg->acc_cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
				cfg->acc_cfg.cred_info[0].data = pj_str(pj_optarg);
				break;
			case OPT_MIC_LEVEL:
				cfg->mic_level = (float)atof(pj_optarg);
				if (cfg->mic_level < 0. || cfg->mic_level > 10.) 
				{
					PJ_LOG(1,(THIS_FILE, "Error: invalid value for --mic-level (expecting 0-10)"));
					return PJ_EINVAL;
				}
				break;
			case OPT_SPEAKER_LEVEL:
				cfg->speaker_level = (float)atof(pj_optarg);
				if (cfg->speaker_level < 0. || cfg->speaker_level > 10.) 
				{
					PJ_LOG(1,(THIS_FILE, "Error: invalid value for --speaker-level (expecting 0-10)"));
					return PJ_EINVAL;
				}
				break;
            case OPT_NAMESERVER: /* nameserver */
                cfg->cfg.nameserver[cfg->cfg.nameserver_count++] = pj_str(pj_optarg);
                if (cfg->cfg.nameserver_count > PJ_ARRAY_SIZE(cfg->cfg.nameserver)) 
                {
                    PJ_LOG(1,(THIS_FILE, "Error: too many nameservers"));
                    return PJ_ETOOMANY;
                }
                break;
            case OPT_USE_ICE:
                cfg->media_cfg.enable_ice = PJ_TRUE;
                break;
			default:
				PJ_LOG(1,(THIS_FILE,  "Argument \"%s\" is not valid.", argv[pj_optind-1]));
				return -1;
		}
	 }
	 
	 if (sip_username == NULL || sip_domain == NULL)
	 {
		PJ_LOG(1,(THIS_FILE, "Username or domain is undefined"));
		return -1;
	 }

	// ID
	cfg->acc_cfg.id.ptr = (char*) pj_pool_alloc(cfg->pool, PJSIP_MAX_URL_SIZE);
	cfg->acc_cfg.id.slen = pj_ansi_snprintf(cfg->acc_cfg.id.ptr, PJSIP_MAX_URL_SIZE, "sip:%s@%s", sip_username, sip_domain);
	if (pjsua_verify_sip_url(cfg->acc_cfg.id.ptr) != 0) 
	{
		PJ_LOG(1,(THIS_FILE, "Error: invalid SIP URL '%s' in local id argument", cfg->acc_cfg.id));
		return PJ_EINVAL;
	}
	
	// Registar
	cfg->acc_cfg.reg_uri.ptr = (char*) pj_pool_alloc(cfg->pool, PJSIP_MAX_URL_SIZE);
	cfg->acc_cfg.reg_uri.slen = pj_ansi_snprintf(cfg->acc_cfg.reg_uri.ptr, PJSIP_MAX_URL_SIZE, "sip:%s", sip_domain);
	if (pjsua_verify_sip_url(cfg->acc_cfg.reg_uri.ptr) != 0) 
	{
		PJ_LOG(1,(THIS_FILE,  "Error: invalid SIP URL '%s' in registrar argument", cfg->acc_cfg.reg_uri));
		return PJ_EINVAL;
	}
	
	// Realm
	if (cfg->acc_cfg.cred_info[0].realm.slen == 0)
	//cfg->cred_info[0].realm = pj_str(sip_domain);
		cfg->acc_cfg.cred_info[0].realm = pj_str(REALM);
	
	return PJ_SUCCESS;
}

/**
 */
int initCaller(pjsua_acc_id *acc_id, const char *filename)
{
	pj_status_t status;

    arm9_wifiInit();
	arm9_wifiConnect();

    /* Create pjsua first! */
    status = pjsua_create();
    if (status != PJ_SUCCESS)
	{	
		printf("Error in pjsua_create()");
		return status;
	}

	/* Create pool for application */
    app_config.pool = pjsua_pool_create("pjsua", 1000, 1000);
	
	/* Initialize default config */
    default_config(&app_config);
	
	/* Parse the arguments */
	status = parse_args(&app_config, filename);
    if (status != PJ_SUCCESS)
		return status;
	
	app_config.cfg.cb.on_incoming_call = &on_incoming_call;
	app_config.cfg.cb.on_call_media_state = &on_call_media_state;
	app_config.cfg.cb.on_call_state = &on_call_state;

	/* Initialize pjsua */
	status = pjsua_init(&app_config.cfg, &app_config.log_cfg, &app_config.media_cfg);
	if (status != PJ_SUCCESS) 
	{
		PJ_LOG(1,(THIS_FILE,  "Error: Error in pjsua_init"));
		return status;
	}

    /* Add UDP transport. */
	status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &app_config.udp_cfg, NULL);
	if (status != PJ_SUCCESS) 
	{
		pjsua_perror(THIS_FILE, "Error creating transport", status);
		return status;
	}

    /* Initialization is done, now start pjsua */
    status = pjsua_start();
    if (status != PJ_SUCCESS) 
	{
		pjsua_perror(THIS_FILE, "Error: Error starting pjsua", status);
		return status;
	}
pjsua_handle_events(100);
    /* Register to SIP server by creating SIP account. */
	status = pjsua_acc_add(&app_config.acc_cfg, PJ_TRUE, acc_id);
	if (status != PJ_SUCCESS) 
	{
		pjsua_perror(THIS_FILE, "Error: Error adding account", status);
		return status;
	}
	pjsua_handle_events(100);

	// Ajuste le niveau du micro et du haut parleur
	pjsua_conf_adjust_rx_level(0, app_config.mic_level);
	pjsua_conf_adjust_tx_level(0, app_config.speaker_level);
	
	return 1;
}

/**
 */
pj_status_t answer()
{
	//pj_status_t status;
	
	ring_deinit(current_call);
	/*if (current_call == -1) 
	{
		
	}*/
	return pjsua_call_answer(current_call, 200, NULL, NULL);
	
	//return status;
}

/**
 */
pj_status_t call(pjsua_acc_id *acc_id, char *number, pjsua_call_id *call_id)
{
	pj_status_t status;
	char uri[256];
	

//	sprintf(uri, "sip:%s@%s", number, sip_domain);
    pj_ansi_snprintf(uri, 256, "sip:%s@%s", number, sip_domain);
    PJ_LOG(5,(THIS_FILE,  "Calling URI \"%s\".", uri));
	//printf("URI %s\n", uri);

	status = pjsua_verify_sip_url(uri);
	if (status != PJ_SUCCESS) 
	{
//		printf("Invalid URL %s\n", uri);
        PJ_LOG(1,(THIS_FILE,  "Invalid URL \"%s\".", uri));
        pjsua_perror(THIS_FILE, "Invalid URL", status);
		return 0;
	}
	
	pj_str_t pj_uri = pj_str(uri);
	
	status = pjsua_call_make_call(*acc_id, &pj_uri, 0, NULL, NULL, call_id);
    if (status != PJ_SUCCESS)
	{
		pjsua_perror(THIS_FILE, "Error making call", status);
		return status;
	}
	
	return PJ_SUCCESS;
}

/**
 */
pj_status_t cleanupCaller()
{
	pj_status_t status;

	if (app_config.pool) {
	pj_pool_release(app_config.pool);
	app_config.pool = NULL;
    }

	/* Destroy pjsua */
    status = pjsua_destroy();
	
	arm9_wifiDisconnect();
	
	return status;
}
