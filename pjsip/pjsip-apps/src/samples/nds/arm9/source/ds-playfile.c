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


#include <pjmedia.h>
#include <pjlib-util.h>
#include <pjlib.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

#include <nds.h>
#include <fat.h>
#include <dswifi9.h>

#include "arm9_wifi.h"
#include "ipcex.h"

/**
 * \page page_pjmedia_samples_playfile_c Samples: Playing WAV File to Sound Device
 *
 * This is a very simple example to use the @ref PJMEDIA_FILE_PLAY and
 * @ref PJMED_SND_PORT. In this example, we open both the file and sound
 * device, and connect the two of them, and voila! Sound will be playing
 * the contents of the file.
 *
 * @see page_pjmedia_samples_recfile_c
 *
 * This file is pjsip-apps/src/samples/playfile.c
 *
 * \includelineno playfile.c
 */


/*
 * playfile.c
 *
 * PURPOSE:
 *  Play a WAV file to sound player device.
 *
 * USAGE:
 *  playfile FILE.WAV
 *
 *  The WAV file could have mono or stereo channels with arbitrary
 *  sampling rate, but MUST contain uncompressed (i.e. 16bit) PCM.
 *
 */


/* For logging purpose. */
#define THIS_FILE   "playfile.c"


/*static const char *desc = 
" FILE                                      \n"
"                                       \n"
"  playfile.c                                   \n"
"                                       \n"
" PURPOSE                                   \n"
"                                       \n"
"  Demonstrate how to play a WAV file.                  \n"
"                                       \n"
" USAGE                                     \n"
"                                       \n"
"  playfile FILE.WAV                            \n"
"                                       \n"
"  The WAV file could have mono or stereo channels with arbitrary   \n"
"  sampling rate, but MUST contain uncompressed (i.e. 16bit) PCM.   \n";
*/

static pj_oshandle_t         file; /* Log file */

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

/*
 * main()
 */
int main(int argc, char *argv[])
{
    pj_caching_pool cp;
    pjmedia_endpt *med_endpt;
    pj_pool_t *pool;
    pjmedia_port *file_port;
    pjmedia_snd_port *snd_port;
    pj_status_t status;

    char *filename=".wav"; // 8khz stereo


    /*if (argc != 2) {
        puts("Error: filename required");
    puts(desc);
    return 1;
    }*/
   powerON(/*POWER_ALL*/POWER_LCD);
    // install the default exception handler
    defaultExceptionHandler();
//    initSystem();
    consoleDemoInit();

    irqInit();
irqEnable(IRQ_VBLANK);
    fatInitDefault();
#ifdef WIFI    
    arm9_wifiInit();
    arm9_wifiAutoconnect();
#endif
    /* Must init PJLIB first: */
    status = pj_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    pj_file_open( NULL, "logsimple.txt", PJ_O_WRONLY, &file);
    pj_log_set_log_func(&log_write_file);

    /* Must create a pool factory before we can allocate any memory. */
    pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 0);

    /* 
     * Initialize media endpoint.
     * This will implicitly initialize PJMEDIA too.
     */
#if PJ_HAS_THREADS
    status = pjmedia_endpt_create(&cp.factory, NULL, 1, &med_endpt);
#else
    status = pjmedia_endpt_create(&cp.factory, 
                  NULL, 
                  0, &med_endpt);
#endif        
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /* Create memory pool for our file player */
    pool = pj_pool_create( &cp.factory,     /* pool factory     */
               "wav",       /* pool name.       */
               4000,        /* init size        */
               4000,        /* increment size       */
               NULL         /* callback on error    */
               );

    /* Create file media port from the WAV file */
    status = pjmedia_wav_player_port_create(  pool, /* memory pool      */
                          filename,  /* file to play     */
                          20,   /* ptime.       */
                          0,    /* flags        */
                          0,    /* default buffer   */
                          &file_port/* returned port    */
                          );
    if (status != PJ_SUCCESS) {
    app_perror(THIS_FILE, "Unable to use WAV file", status);
    return 1;
    }

    /* Create sound player port. */
    status = pjmedia_snd_port_create_player( 
         pool,                  /* pool         */
         -1,                    /* use default dev.     */
         file_port->info.clock_rate,        /* clock rate.      */
         file_port->info.channel_count,     /* # of channels.       */
         file_port->info.samples_per_frame, /* samples per frame.   */
         file_port->info.bits_per_sample,   /* bits per sample.     */
         0,                 /* options          */
         &snd_port              /* returned port        */
         );
    if (status != PJ_SUCCESS) {
    app_perror(THIS_FILE, "Unable to open sound device", status);
    return 1;
    }

    /* Connect file port to the sound player.
     * Stream playing will commence immediately.
     */
    status = pjmedia_snd_port_connect( snd_port, file_port);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);



    /* 
     * File should be playing and looping now, using sound device's thread. 
     */


    /* Sleep to allow log messages to flush */
    pj_thread_sleep(100);


    printf("Playing %s..\n", filename);
    printf("Press <A> to stop playing and quit");

    //fgets(tmp, sizeof(tmp), stdin);
    while(1) 
    {
        swiWaitForVBlank();
        scanKeys();
        uint32 key = keysDown(); 
        if (key & KEY_A)
        {
            break;
        }
        if (key & KEY_B)
        {
            IPC2->sound_control = IPC2_SOUND_CLICK;
        }
    }       
    printf("bye bye\n");
    
    /* Start deinitialization: */

    /* Disconnect sound port from file port */
    status = pjmedia_snd_port_disconnect(snd_port);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /* Without this sleep, Windows/DirectSound will repeteadly
     * play the last frame during destroy.
     */
    pj_thread_sleep(100);

    /* Destroy sound device */
    status = pjmedia_snd_port_destroy( snd_port );
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /* Destroy file port */
    status = pjmedia_port_destroy( file_port );
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /* Release application pool */
    pj_pool_release( pool );

    /* Destroy media endpoint. */
    pjmedia_endpt_destroy( med_endpt );

    /* Destroy pool factory */
    pj_caching_pool_destroy( &cp );
#ifdef WIFI 
    arm9_wifiDisconnect();
#endif

    pj_file_close(file);

    /* Shutdown PJLIB */
    pj_shutdown();


    /* Done. */
    return 0;
}

