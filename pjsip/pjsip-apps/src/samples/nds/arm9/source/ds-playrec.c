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

#define AUDIO
#define MICRO

/* Configs */
//#define CLOCK_RATE        44100
//#define CLOCK_RATE      16000
#define CLOCK_RATE 8000
//#define SAMPLES_PER_FRAME   (CLOCK_RATE * 20 / 1000)
//#define SAMPLES_PER_FRAME NSAMPLES
#define SAMPLES_PER_FRAME 160
//#define NCHANNELS     2
#define NCHANNELS       1
#define BITS_PER_SAMPLE     16


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
    pjmedia_port *playfile_port, *recfile_port;
    pjmedia_snd_port *g_snd_player, *g_snd_rec;
    pj_status_t status;
//    char *filename="Deux_pieds.wav"; // 22khz mono
//    char *filename="Windows_XP_Demarrage.wav";
//    char *play_filename="Deux_pieds21.wav"; // 8khz stereo
    char *play_filename="Deux_pieds22.wav"; // 22khz stereo
//    char *play_filename="d8.wav"; // 8khz mono
//    char *play_filename="d16.wav"; // 16khz mono
    char *rec_filename="record.wav";


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
    arm9_wifiInit();
    arm9_wifiAutoconnect();

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


#ifdef MICRO_FIRST
    /* Create WAVE file writer port. */
    status = pjmedia_wav_writer_port_create(  pool, rec_filename,
                          CLOCK_RATE,
                          NCHANNELS,
                          SAMPLES_PER_FRAME,
                          BITS_PER_SAMPLE,
                          0, 0, 
                          &recfile_port);
    if (status != PJ_SUCCESS) {
    app_perror(THIS_FILE, "Unable to open WAV file for writing", status);
    return 1;
    }

    /* Create sound player port. */
    status = pjmedia_snd_port_create_rec( 
         pool,                  /* pool         */
         -1,                    /* use default dev.     */
         recfile_port->info.clock_rate,        /* clock rate.      */
         recfile_port->info.channel_count,     /* # of channels.       */
         recfile_port->info.samples_per_frame, /* samples per frame.   */
         recfile_port->info.bits_per_sample,   /* bits per sample.     */
         0,                 /* options          */
         &g_snd_rec              /* returned port        */
         );
    if (status != PJ_SUCCESS) {
    app_perror(THIS_FILE, "Unable to open sound device", status);
    return 1;
    }

    /* Connect file port to the sound player.
     * Stream playing will commence immediately.
     */
    status = pjmedia_snd_port_connect( g_snd_rec, recfile_port);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
#endif

#ifdef AUDIO
    /* Create file media port from the WAV file */
    status = pjmedia_wav_player_port_create(  pool, /* memory pool      */
                          play_filename,  /* file to play     */
                          20,   /* ptime.       */
                          0,    /* flags        */
                          0,    /* default buffer   */
                          &playfile_port/* returned port    */
                          );
    if (status != PJ_SUCCESS) {
    app_perror(THIS_FILE, "Unable to use WAV file", status);
    return 1;
    }

    /* Create sound player port. */
    status = pjmedia_snd_port_create_player( 
         pool,                  /* pool         */
         -1,                    /* use default dev.     */
         playfile_port->info.clock_rate,        /* clock rate.      */
         playfile_port->info.channel_count,     /* # of channels.       */
         playfile_port->info.samples_per_frame, /* samples per frame.   */
         playfile_port->info.bits_per_sample,   /* bits per sample.     */
         0,                 /* options          */
         &g_snd_player              /* returned port        */
         );
    if (status != PJ_SUCCESS) {
    app_perror(THIS_FILE, "Unable to open sound device", status);
    return 1;
    }

    /* Connect file port to the sound player.
     * Stream playing will commence immediately.
     */
    status = pjmedia_snd_port_connect( g_snd_player, playfile_port);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
#endif
#ifdef MICRO
    /* Create WAVE file writer port. */
    status = pjmedia_wav_writer_port_create(  pool, rec_filename,
                          CLOCK_RATE,
                          NCHANNELS,
                          SAMPLES_PER_FRAME,
                          BITS_PER_SAMPLE,
                          0, 0, 
                          &recfile_port);
    if (status != PJ_SUCCESS) {
    app_perror(THIS_FILE, "Unable to open WAV file for writing", status);
    return 1;
    }

    /* Create sound player port. */
    status = pjmedia_snd_port_create_rec( 
         pool,                  /* pool         */
         -1,                    /* use default dev.     */
         recfile_port->info.clock_rate,        /* clock rate.      */
         recfile_port->info.channel_count,     /* # of channels.       */
         recfile_port->info.samples_per_frame, /* samples per frame.   */
         recfile_port->info.bits_per_sample,   /* bits per sample.     */
         0,                 /* options          */
         &g_snd_rec              /* returned port        */
         );
    if (status != PJ_SUCCESS) {
    app_perror(THIS_FILE, "Unable to open sound device", status);
    return 1;
    }

    /* Connect file port to the sound player.
     * Stream playing will commence immediately.
     */
    status = pjmedia_snd_port_connect( g_snd_rec, recfile_port);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
#endif

    /* 
     * File should be playing and looping now, using sound device's thread. 
     */

    /* Sleep to allow log messages to flush */
    pj_thread_sleep(100);


    printf("Playing %s..\n", play_filename);
    printf("Recording %s..\n", rec_filename);
    printf("Press <A> to stop playing and recording, and quit");

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
    status = pjmedia_snd_port_disconnect(g_snd_player);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /* Without this sleep, Windows/DirectSound will repeteadly
     * play the last frame during destroy.
     */
    pj_thread_sleep(100);

    /* Destroy sound device */
    status = pjmedia_snd_port_destroy( g_snd_rec );
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    status = pjmedia_snd_port_destroy( g_snd_player );
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /* Destroy file port */
    status = pjmedia_port_destroy( recfile_port );
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    status = pjmedia_port_destroy( playfile_port );
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /* Release application pool */
    pj_pool_release( pool );

    /* Destroy media endpoint. */
    pjmedia_endpt_destroy( med_endpt );

    /* Destroy pool factory */
    pj_caching_pool_destroy( &cp );

//arm9_wifiDisconnect();

    pj_file_close(file);

    /* Shutdown PJLIB */
    pj_shutdown();


    /* Done. */
    return 0;
}

