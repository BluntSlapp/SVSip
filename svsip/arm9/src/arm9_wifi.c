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
#include <pjlib-util.h>
#include <pjsua-lib/pjsua.h>

#include <netinet/in.h>

#include <nds.h>
#include <dswifi9.h>
#include "arm9_wifi.h"


/* For logging purpose. */
#define THIS_FILE   "arm9_wifi.c"

static void WaitVbl() ;

void printbtm(int x, int y, const char * str) {
	// Google: ANSI ESCAPE sequences
	// http://isthe.com/chongo/tech/comp/ansi_escapes.html
	printf("\x1b[%d;%dH%s\x1b[K", x, y, str);
}

#define WifiTimerInterval_ms (50)
//#define WifiTimerInterval_ms (20)

// Dswifi stub functions
//void *
//sgIP_malloc(int size) {
//  return malloc(size);
//}
//void
//sgIP_free(void * ptr) {
//  free(ptr);
//}

// sgIP_dbgprint only needed in debug version
void
sgIP_dbgprint(char * txt __attribute__((unused)), ...) {      
}

// wifi timer function, to update internals of sgIP
static void arm9_wifiTimer(void) {
    Wifi_Timer(WifiTimerInterval_ms);
}

// notification function to send fifo message to arm7
void arm9_synctoarm7() { // send fifo message
    REG_IPC_FIFO_TX=0x87654321;
}

// interrupt handler to receive fifo messages from arm7
void arm9_fifo() { // check incoming fifo messages
    u32 value = REG_IPC_FIFO_RX;
    if(value == 0x87654321) {
        Wifi_Sync();
    }
}

void arm9_wifiInit()
{
    // send fifo message to initialize the arm7 wifi
    REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR; // enable & clear FIFO
    
    u32 Wifi_pass= Wifi_Init(WIFIINIT_OPTION_USELED);
//    u32 Wifi_pass= Wifi_Init(WIFIINIT_OPTION_USELED|WIFIINIT_OPTION_USEHEAP_64);
    REG_IPC_FIFO_TX=0x12345678;
    REG_IPC_FIFO_TX=Wifi_pass;

    *((volatile u16 *)0x0400010E) = 0; // disable timer3

    irqSet(IRQ_TIMER3, arm9_wifiTimer); // setup timer IRQ
    irqEnable(IRQ_TIMER3);
    irqSet(IRQ_FIFO_NOT_EMPTY, arm9_fifo); // setup fifo IRQ
    irqEnable(IRQ_FIFO_NOT_EMPTY);

    REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ; // enable FIFO IRQ

    Wifi_SetSyncHandler(arm9_synctoarm7); // tell wifi lib to use our handler to notify arm7

    // set timer3
//    *((volatile u16 *)0x0400010C) = -6553; // 6553.1 * 256 cycles = ~50ms;
    *((volatile u16 *)0x0400010C) = -(131062*WifiTimerInterval_ms/1000); // 131062 * 256 cycles = ~1000ms;
    *((volatile u16 *)0x0400010E) = 0x00C2; // enable, irq, 1/256 clock
    
    while(Wifi_CheckInit()==0) 
    { // wait for arm7 to be initted successfully
        WaitVbl(); // wait for vblank
    }
        
     // wifi init complete - wifi lib can now be used!
//    Wifi_ScanMode();
//    swiWaitForVBlank();
}

void arm9_wifiDisconnect(void)
{
    Wifi_DisconnectAP();
    /*while(1)
    {
        int i = Wifi_AssocStatus(); // check status
		printf("Wifi_AssocStatus %d\n", i);
        if(i == ASSOCSTATUS_DISCONNECTED)
        {
            PJ_LOG(3,(THIS_FILE, "disconnect"));
			printf("disconnect\n");
            break;
        }
    }*/
    Wifi_DisableWifi();
}

Wifi_AccessPoint global_connectAP;
int firmware;
unsigned char global_wepkeys[4][32];
int global_wepkeyid, global_wepmode;
int global_dhcp; // 0=none, 1=get IP&dns, 2=get IP&not dns
unsigned long global_ipaddr, global_snmask, global_gateway, global_dns1,global_dns2;



static
int arm9_readWifiParameters(const char* config_file)
{
	pj_pool_t *pool;

	int argc = 0;
	char **argv;
	pj_status_t status = PJ_SUCCESS;
	int option_index;
	int c;
	struct in_addr in;
	
	int i;
	int len;

enum { OPT_CONFIG=256, OPT_SSID, OPT_CHANNEL, OPT_WEP_KEY,
	OPT_IP, OPT_MASK, OPT_GATEWAY, OPT_DNS1, OPT_DNS2};

struct pj_getopt_option long_options[] = {
	{ "config",	1, 0, OPT_CONFIG},
	{ "ssid",	1, 0, OPT_SSID},
	{ "channel", 1, 0, OPT_CHANNEL},
	{ "wep-key", 1, 0, OPT_WEP_KEY},
	{ "ip", 1, 0, OPT_IP},
	{ "mask",	1, 0, OPT_MASK},
	{ "gateway",	1, 0, OPT_GATEWAY},
	{ "dns1", 1, 0, OPT_DNS1},
	{ "dns2", 1, 0, OPT_DNS2},
	{ NULL, 0, 0, 0}
    };

	firmware = 1;
	global_dhcp = 1;
	global_wepkeyid = 0;
	global_wepmode = WEPMODE_NONE;
	memset(&global_connectAP, 0, sizeof(global_connectAP));
	
	status = read_config_file(config_file, &argc, &argv);
	if (status != 0)
	    return status;
	
	while((c=pj_getopt_long(argc,argv, "", long_options,&option_index))!=-1) 
	{
		switch(c)
		{
			case OPT_CONFIG:
				if (stricmp( "firmware", pj_optarg) == 0)
				{
					firmware = 1;
					return status;
				}
				firmware = 0;
				break;
			case OPT_SSID:
				strncpy(global_connectAP.ssid, pj_optarg, sizeof(global_connectAP.ssid));
				global_connectAP.ssid_len = strlen(global_connectAP.ssid);
				break;
			case OPT_CHANNEL:
				global_connectAP.channel = atol(pj_optarg);
				break;
			case OPT_WEP_KEY:
				len = strlen(pj_optarg);
				char tmp[3] = {0};

				switch(len)
				{
					case 0: global_wepmode=0; break;
					case 10: global_wepmode=1; break;
					case 26: global_wepmode=2; break;
					default: 
						PJ_LOG(1,(THIS_FILE,  "WEP key \"%s\" is not valid.", pj_optarg));
						return -1;
				}
				
				for (i = 0; i < len/2; i++)
				{
					int var;
					char *end;
					tmp[0] = pj_optarg[2*i];
					tmp[1] = pj_optarg[2*i+1];
					var = strtol(tmp, &end, 16);
					if (*end != '\0')
					{
						PJ_LOG(1,(THIS_FILE,  "WEP key character \"%s\" is not valid.", pj_optarg));
						return -1;
					}
					global_wepkeys[0][i] = (unsigned char)(0xFF & var);
				}
				
				break;
			case OPT_IP:
				if (inet_aton(pj_optarg, &in) == 0)
				{
					PJ_LOG(1,(THIS_FILE,  "IP address \"%s\" is not valid.", pj_optarg));
					return -1;
				}
				global_ipaddr = in.s_addr;
				if (global_dhcp != 0)
					global_dhcp = 0;
				break;
			case OPT_MASK:
				if (inet_aton(pj_optarg, &in) == 0)
				{
					PJ_LOG(1,(THIS_FILE,  "Mask \"%s\" is not valid.", pj_optarg));
					return -1;
				}
				global_snmask = in.s_addr;
				break;
			case OPT_GATEWAY:
				if (inet_aton(pj_optarg, &in) == 0)
				{
					PJ_LOG(1,(THIS_FILE,  "IP gateway \"%s\" is not valid.", pj_optarg));
					return -1;
				}
				global_gateway = in.s_addr;
				break;
			case OPT_DNS1:
				if (inet_aton(pj_optarg, &in) == 0)
				{
					PJ_LOG(1,(THIS_FILE,  "Primary DNS Address \"%s\" is not valid.", pj_optarg));
					return -1;
				}
				global_dns1 = in.s_addr;
				if (global_dhcp == 1)
					global_dhcp = 2;
				break;
			case OPT_DNS2:
				if (inet_aton(pj_optarg, &in) == 0)
				{
					PJ_LOG(1,(THIS_FILE,  "Secondary DNS Address \"%s\" is not valid.", pj_optarg));
					return -1;
				}
				global_dns2 = in.s_addr;
				break;
			default:
				PJ_LOG(1,(THIS_FILE,  "Argument \"%s\" is not valid.", argv[pj_optind-1]));
				return -1;
		}
	}
	
	return status;	
}

static void WaitVbl() 
{
	int i;
	while(REG_VCOUNT>192);
	while(REG_VCOUNT<192);
}

const char * assocstatus_strings[] = {
	"ASSOCSTATUS_DISCONNECTED",		// not *trying* to connect
	"ASSOCSTATUS_SEARCHING",		// data given does not completely specify an AP, looking for AP that matches the data.
	"ASSOCSTATUS_AUTHENTICATING",	// connecting...
	"ASSOCSTATUS_ASSOCIATING",		// connecting...
	"ASSOCSTATUS_ACQUIRINGDHCP",	// connected to AP, but getting IP data from DHCP
	"ASSOCSTATUS_ASSOCIATED",		// Connected!
	"ASSOCSTATUS_CANNOTCONNECT" 	// error in connecting...
};

const char * wepmodes[] = {
	"off","64bit","128bit","?"
};
const int wepkeylen[] = {
	0,5,13,0
};

int arm9_wifiConnect(void)
{
	int j, state,delay;
	char txt[128];
	int i;

	if (arm9_readWifiParameters("svsip/wifi") != PJ_SUCCESS)
	{
		//while(1) WaitVbl();
		return 1;
	}

	delay=0;
	Wifi_DisconnectAP();
	if (firmware == 0)
	{
		switch(global_dhcp) 
		{
			case 0: // none
				Wifi_SetIP(global_ipaddr,global_gateway,global_snmask,global_dns1,global_dns2);
				break;
			case 1: // dhcp both
				Wifi_SetIP(0,0,0,0,0);
				break;
			case 2: // dhcp IP/not dns
				Wifi_SetIP(0,0,0,global_dns1,global_dns2);
				break;
		}
	}
	
	j=0;
	state=5;
	printbtm(0,0,"Connecting to AP...");
	while(1) 
	{
		WaitVbl();
		
		// Draw info
		printbtm(0,0,"Connecting to AP...");
		//btm_drawbutton(16,21,31,23,0,"Cancel");
		//printbtm(23,0,"[B]Cancel");
/*		
		sprintf(txt, "config %d", firmware);
		printbtm(5,0, txt);
		sprintf(txt, "ssid %s", global_connectAP.ssid);
		printbtm(6,0, txt);
		sprintf(txt, "ssid_len %d", global_connectAP.ssid_len);
		printbtm(7,0, txt);
		sprintf(txt, "channel %d", global_connectAP.channel);
		printbtm(8,0, txt);
		sprintf(txt, "wepmode %s", wepmodes[global_wepmode]);
		printbtm(9,0, txt);
		sprintf(txt, "wepkeyid %d", global_wepkeyid);
		printbtm(10,0, txt);
		for (i = 0; i < wepkeylen[global_wepmode]; i++)
		{

			snprintf(txt, 8, "%02x", global_wepkeys[global_wepkeyid][i]);
			printbtm(11, 2*i, txt);
		}
*/		
		// draw state info & processing.
		switch(state) {

		case 5: // connect to AP
			state=6;
			if(firmware) {
				Wifi_AutoConnect();
			} else {
				Wifi_ConnectAP(&global_connectAP,global_wepmode,global_wepkeyid,global_wepkeys[0]);
			}
		case 6:
			//           "0123456789ABCDEF0123456789ABCDEF"
			printbtm(2,0,"Connecting to Access Point...");
			printbtm(3,0,assocstatus_strings[j=Wifi_AssocStatus()]);
			if(j == ASSOCSTATUS_ASSOCIATED) state=30;
			if(j == ASSOCSTATUS_CANNOTCONNECT) state=20;
			break;		

		case 20:
			printbtm(2,0,"Cannot connect to Access Point.");
			printbtm(3,0,"(Hit Cancel to continue)");
			break;

		case 30:
			delay=60;
			state=31;
		case 31:
			printbtm(2,0,"Connected!");
			if(!(delay--)) 
				return 1;
			break;

		}
	}
	return 0;
}
