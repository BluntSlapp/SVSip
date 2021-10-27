/*
 * Copyright (C) 2003-2007 Benny Prijono <benny@prijono.org>
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

#include <nds.h>
#include <fat.h>

#include "arm9_wifi.h"

#define THIS_FILE   "test.c"

static pj_oshandle_t         file; /* Log file */


static void check_error(const char *func, pj_status_t status)
{
     if (status != PJ_SUCCESS) {
    char errmsg[PJ_ERR_MSG_SIZE];
    pj_strerror(status, errmsg, sizeof(errmsg));
    PJ_LOG(1,(THIS_FILE, "%s error: %s", func, errmsg));
    pj_file_close(file);
    exit(1);
     }
}

#define DO(func)  status = func; check_error(#func, status);


void log_write_file(int level, const char *buffer, int len)
{
    long l =(long)(len);
    PJ_CHECK_STACK();
    /* Copy to terminal/file. */
    fputs(buffer, stdout);
    fflush(stdout);

    pj_file_write(file, buffer, &l);
//    pj_file_flush(file);
}

int main()
{
     pj_sock_t sock;
     pj_sockaddr_in addr;
     //pj_str_t stun_srv = pj_str("stun.fwdnet.net");
     pj_str_t stun_srv = pj_str("60.32.135.104");
     pj_caching_pool cp;
     pj_status_t status;

    // install the default exception handler
    defaultExceptionHandler();
    consoleDemoInit();
    irqInit();
irqEnable(IRQ_VBLANK);
    fatInitDefault();
    arm9_wifiInit();
    arm9_wifiAutoconnect();

     DO( pj_init() );
     
     pj_file_open( NULL, "logsimple.txt", PJ_O_WRONLY, &file);
     pj_log_set_log_func(&log_write_file);

     pj_caching_pool_init(&cp, NULL, 0);

     DO( pjlib_util_init() );
     DO( pj_sock_socket(pj_AF_INET(), pj_SOCK_DGRAM(), 0, &sock) );
     DO( pj_sock_bind_in(sock, 0, 0) );

     DO( pjstun_get_mapped_addr(&cp.factory, 1, &sock,
                   &stun_srv, 3478,
                   &stun_srv, 3478,
                   &addr) );

     PJ_LOG(3,(THIS_FILE, "Mapped address is %s:%d",
          pj_inet_ntoa(addr.sin_addr),
          (int)pj_ntohs(addr.sin_port)));

     DO( pj_sock_close(sock) );
     pj_caching_pool_destroy(&cp);
     
     pj_file_close(file);
     
     pj_shutdown();
     
     arm9_wifiDisconnect();
     return 0;
}
