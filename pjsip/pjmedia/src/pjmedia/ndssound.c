/* $Id$ */
/* 
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

#include <pjmedia/sound.h>
#include <pjmedia/errno.h>
#include <pj/assert.h>
#include <pj/log.h>
#include <pj/os.h>
#include <pj/string.h>

#if PJMEDIA_SOUND_IMPLEMENTATION == PJMEDIA_SOUND_NDS_SOUND

#include <nds.h>
#include "ipcex.h"
//#include "sound_tables.h"

#define THIS_FILE	    "ndssound.c"
#define BITS_PER_SAMPLE     16

#if 1
#   define TRACE_(x)    PJ_LOG(5,x)
#else
#   define TRACE_(x)
#endif

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define POOL_NAME       "NintendoSound"
#define POOL_SIZE       512
#define POOL_INC        512

static pjmedia_snd_dev_info nintendo_snd_dev_info = 
{
    "Nintendo DS Sound Device",
    1,
    1,
    8000
};

static pj_pool_factory *snd_pool_factory;

/* 
 * Sound stream descriptor.
 */
struct pjmedia_snd_stream
{
    pj_pool_t      *pool;       /**< Memory pool.       */
    pjmedia_dir     dir;        /**< Sound direction.   */
    int             play_id;    /**< Playback dev id.   */
    int             rec_id;     /**< Recording dev id.  */

    unsigned        clock_rate;         /**< Clock rate.          */
    unsigned        channel_count;      /**< Channel count.       */
    unsigned        samples_per_frame;  /**< Samples per frame.   */
    unsigned        bytes_per_sample;

    pjmedia_snd_rec_cb      rec_cb;    /**< Capture callback.      */
    pjmedia_snd_play_cb     play_cb;   /**< Playback callback.     */
    
    void           *user_data;         /**< Application data.      */
    pj_uint32_t     timestamp;
};

static pjmedia_snd_stream *play_strm, *rec_strm;

//#define SOUND_MAX_SAMPLES_TRANSFER 1024
//#define SOUND_BUFFER_SAMPLES 1024
//#define SOUND_MAX_SAMPLES_TRANSFER 896
//#define SOUND_BUFFER_SAMPLES 896
//#define SOUND_MAX_SAMPLES_TRANSFER 160
//#define SOUND_BUFFER_SAMPLES 160
#define SOUND_MAX_SAMPLES_TRANSFER 80
#define SOUND_BUFFER_SAMPLES 80
//#define SOUND_BUFFER_SAMPLES 16384

static pj_int32_t interrupt_handler_started = 0;

pj_char_t *mic;
pj_char_t *micbuf;

pj_char_t *pcmL, *pcmR;
pj_char_t *pcmbufL, *pcmbufR;
pj_int32_t buffer_samples, buffer_end, buffer_size, buffer_pos, buffer_lowest;

static char *readbuffer;

/**
 */
void sound_update(void)
{
    pj_status_t ret;
    unsigned size;
    
    switch(play_strm->channel_count)
    {
        case 1:
        {
            // Récupérer les données à écouter !!
            while(buffer_samples < buffer_size) 
            {
                size = 
                    2 * MIN(buffer_size - buffer_samples, buffer_size - buffer_end);
                ret = play_strm->play_cb(play_strm->user_data, play_strm->timestamp, 
                    (void *) (pcmbufL + buffer_end * 2), size);
    
                if(ret != PJ_SUCCESS) 
                {
                    //state = FINISHING;
                    return;
                }
        
                buffer_end += size / 2;
                buffer_end %= buffer_size;
                buffer_samples += size / 2;
    //            strm->timestamp += strm->samples_per_frame;
    //            strm->timestamp += (size / (2 * strm->samples_per_frame * strm->bytes_per_sample) ) * strm->samples_per_frame;
            }
            break;
        }
        case 2:
        {
            int i;
            s16 *srcL, *dstL, *srcR, *dstR;
            
            while(buffer_samples < buffer_size) 
            {
                size = 
                    4 * MIN(buffer_size - buffer_samples, buffer_size - buffer_end);
                ret = play_strm->play_cb(play_strm->user_data, play_strm->timestamp, 
                    readbuffer, size);
                
                if(ret != PJ_SUCCESS) 
                {
                    //state = FINISHING;
                    return;
                }
                
                srcL = (s16 *) readbuffer;
                dstL = ((s16 *) pcmbufL) + buffer_end;
                srcR = ((s16 *) readbuffer) + 1;
                dstR = ((s16 *) pcmbufR) + buffer_end;
    
                for(i = 0; i < size / 2; i += 2) {
                    dstL[i/2] = srcL[i];
                    dstR[i/2] = srcR[i];
                }
    
                buffer_end += size / 4;
                buffer_end %= buffer_size;
                buffer_samples += size / 4;
    //            strm->timestamp += strm->samples_per_frame;
            }
            break;
        }
    }
}

/**
 */
static void InterruptHandler_IPC_SYNC(void) {
    u8 sync;

    sync = IPC_GetSync();

    if(play_strm && (sync == IPC2_REQUEST_WRITE_SOUND)) 
    {
        pj_uint32_t sound_bps;
        
		sound_update();

        if(buffer_samples < SOUND_MAX_SAMPLES_TRANSFER)
            return;

        sound_bps = play_strm->bytes_per_sample;
        if(buffer_size - buffer_pos >= SOUND_MAX_SAMPLES_TRANSFER)
        {
            memcpy(pcmL, pcmbufL + buffer_pos * sound_bps, SOUND_MAX_SAMPLES_TRANSFER * sound_bps);
            if(play_strm->channel_count == 2)
                memcpy(pcmR, pcmbufR + buffer_pos * sound_bps, SOUND_MAX_SAMPLES_TRANSFER * sound_bps);
        }
        else 
        {
            memcpy(pcmL, pcmbufL + buffer_pos * sound_bps, (buffer_size - buffer_pos) * sound_bps);
            if(play_strm->channel_count == 2)
                memcpy(pcmR, pcmbufR + buffer_pos * sound_bps, (buffer_size - buffer_pos) * sound_bps);

            memcpy(pcmL + (buffer_size - buffer_pos) * sound_bps, pcmbufL, (SOUND_MAX_SAMPLES_TRANSFER - (buffer_size - buffer_pos)) * sound_bps);
            if(play_strm->channel_count == 2)
                memcpy(pcmR + (buffer_size - buffer_pos) * sound_bps, pcmbufR, (SOUND_MAX_SAMPLES_TRANSFER - (buffer_size - buffer_pos)) * sound_bps);
        }

        buffer_pos += SOUND_MAX_SAMPLES_TRANSFER;
        buffer_pos %= buffer_size;

        buffer_samples -= SOUND_MAX_SAMPLES_TRANSFER;
        if(buffer_samples < buffer_lowest)
            buffer_lowest = buffer_samples;

        DC_FlushRange(pcmL, SOUND_MAX_SAMPLES_TRANSFER * sound_bps);
        if(play_strm->channel_count == 2)
            DC_FlushRange(pcmR, SOUND_MAX_SAMPLES_TRANSFER * sound_bps);

        IPC2->sound_writerequest = 0;
    }
    else if (rec_strm && (sync == IPC2_REQUEST_READ_SOUND))
    {
        pj_status_t ret;
        pj_uint32_t bytes_per_frame;
        
        bytes_per_frame = rec_strm->samples_per_frame * rec_strm->bytes_per_sample;
        
        DC_FlushRange(mic, bytes_per_frame);
        memcpy(micbuf, mic, bytes_per_frame); // FIXME: est-ce nécessaire ?

        IPC2->micro_readrequest = 0;
        ret = rec_strm->rec_cb(rec_strm->user_data, rec_strm->timestamp, 
            (void *)micbuf, bytes_per_frame);
        rec_strm->timestamp += rec_strm->samples_per_frame;
        if(ret != PJ_SUCCESS) 
        {
            //state = FINISHING;
            return;
        }
    }
}

/**
 */
PJ_DEF(pj_status_t) pjmedia_snd_init(pj_pool_factory *factory)
{
    //TODO: sur ARM7 powerON(POWER_SOUND)
	TRACE_((THIS_FILE, "pjmedia_snd_init."));
    snd_pool_factory = factory;
    return PJ_SUCCESS;
}

/**
 */
PJ_DEF(int) pjmedia_snd_get_dev_count(void)
{
	TRACE_((THIS_FILE, "pjmedia_snd_get_dev_count."));
    /* Always return 1 */
    return 1;
}

/**
 */
PJ_DEF(const pjmedia_snd_dev_info*) pjmedia_snd_get_dev_info(unsigned index)
{
    TRACE_((THIS_FILE, "pjmedia_snd_get_dev_info %d.", index));
    /* Always return the default sound device */
    PJ_ASSERT_RETURN(index==0 || index==(unsigned)-1, NULL);
    return &nintendo_snd_dev_info;
}

/**
 */
PJ_DEF(pj_status_t) pjmedia_snd_open_rec( int index,
					  unsigned clock_rate,
					  unsigned channel_count,
					  unsigned samples_per_frame,
					  unsigned bits_per_sample,
					  pjmedia_snd_rec_cb rec_cb,
					  void *user_data,
					  pjmedia_snd_stream **p_snd_strm)
{
    PJ_ASSERT_RETURN(rec_cb && p_snd_strm, PJ_EINVAL);
	TRACE_((THIS_FILE, "pjmedia_snd_open_rec."));
         
   return pjmedia_snd_open(index, -2, clock_rate, channel_count,
        samples_per_frame, bits_per_sample,
        rec_cb, NULL, user_data, p_snd_strm);
}

/**
 */
PJ_DEF(pj_status_t) pjmedia_snd_open_player( int index,
					unsigned clock_rate,
					unsigned channel_count,
					unsigned samples_per_frame,
					unsigned bits_per_sample,
					pjmedia_snd_play_cb play_cb,
					void *user_data,
					pjmedia_snd_stream **p_snd_strm )
{
    PJ_ASSERT_RETURN(play_cb && p_snd_strm, PJ_EINVAL);
	TRACE_((THIS_FILE, "pjmedia_snd_open_player."));
         
    return pjmedia_snd_open(-2, index, clock_rate, channel_count,
        samples_per_frame, bits_per_sample,
        NULL, play_cb, user_data, p_snd_strm);      
}

/**
 */
PJ_DEF(pj_status_t) pjmedia_snd_open( int rec_id,
				      int play_id,
				      unsigned clock_rate,
				      unsigned channel_count,
				      unsigned samples_per_frame,
				      unsigned bits_per_sample,
				      pjmedia_snd_rec_cb rec_cb,
				      pjmedia_snd_play_cb play_cb,
				      void *user_data,
				      pjmedia_snd_stream **p_snd_strm)
{
    TRACE_((THIS_FILE, "pjmedia_snd_open."));

    pj_pool_t *pool;
    pjmedia_snd_stream *snd_strm;
     
    /* Make sure sound subsystem has been initialized with
     * pjmedia_snd_init() */
    PJ_ASSERT_RETURN( snd_pool_factory != NULL, PJ_EINVALIDOP );

    /* Can only support 16bits per sample */
    PJ_ASSERT_RETURN(bits_per_sample == BITS_PER_SAMPLE, PJ_EINVAL);

    pool = pj_pool_create(snd_pool_factory, POOL_NAME, POOL_SIZE, POOL_INC, 
                  NULL);
    if (!pool)
        return PJ_ENOMEM;
    snd_strm = PJ_POOL_ZALLOC_T(pool, pjmedia_snd_stream);
    
    snd_strm->pool = pool;
    
    if (rec_id == -1) rec_id = 0;
    if (play_id == -1) play_id = 0;
    
    if (rec_id != -2 && play_id != -2)
        snd_strm->dir = PJMEDIA_DIR_CAPTURE_PLAYBACK;
    else if (rec_id != -2)
        snd_strm->dir = PJMEDIA_DIR_CAPTURE;
    else if (play_id != -2)
        snd_strm->dir = PJMEDIA_DIR_PLAYBACK;
    
    snd_strm->rec_id = rec_id;
    snd_strm->play_id = play_id;
    snd_strm->clock_rate = clock_rate;
    snd_strm->channel_count = channel_count;
    snd_strm->samples_per_frame = samples_per_frame;
    snd_strm->bytes_per_sample = bits_per_sample / 8;
    snd_strm->rec_cb = rec_cb;
    snd_strm->play_cb = play_cb;
    snd_strm->user_data = user_data;
    
    /* Create player stream */
    if (snd_strm->dir & PJMEDIA_DIR_PLAYBACK) 
    {
        buffer_size = SOUND_BUFFER_SAMPLES;
        pcmbufL = pj_pool_zalloc(pool, SOUND_BUFFER_SAMPLES * 2);
        pcmbufR = pj_pool_zalloc(pool, SOUND_BUFFER_SAMPLES * 2);
        pcmL = pj_pool_zalloc(pool, SOUND_MAX_SAMPLES_TRANSFER * 2);
        pcmR = pj_pool_zalloc(pool, SOUND_MAX_SAMPLES_TRANSFER * 2);
          
        readbuffer = (channel_count == 2 ? 
            pj_pool_zalloc(pool, buffer_size * 4) : NULL);

        play_strm = snd_strm;
    }

    /* Create capture stream */
    if (snd_strm->dir & PJMEDIA_DIR_CAPTURE) 
    {
        mic = pj_pool_zalloc(pool, samples_per_frame * bits_per_sample / 8);
        micbuf = pj_pool_zalloc(pool, samples_per_frame * bits_per_sample / 8);
        
        rec_strm = snd_strm;
    }

    *p_snd_strm = snd_strm;

    return PJ_SUCCESS;
}

/**
 */
PJ_DEF(pj_status_t) pjmedia_snd_stream_start(pjmedia_snd_stream *stream)
{
    TRACE_((THIS_FILE, "pjmedia_snd_stream_start."));
    
    /* Start the Interruption Handler for IPC */
    if (++interrupt_handler_started == 1)
    {
        irqSet(IRQ_IPC_SYNC, InterruptHandler_IPC_SYNC);
        REG_IPC_SYNC = IPC_SYNC_IRQ_ENABLE;
    }
    
    /* Create player stream */
    if (stream->dir & PJMEDIA_DIR_PLAYBACK)
    {
        TRACE_((THIS_FILE, "pjmedia_snd_stream_start : play back starting..."));

        buffer_samples = buffer_end = buffer_pos = 0;
        buffer_lowest = 0xffff;
        
        IPC2->sound_channels = stream->channel_count;
        IPC2->sound_frequency = stream->clock_rate;
        IPC2->sound_bytes_per_sample = stream->bytes_per_sample;
        IPC2->sound_samples = SOUND_MAX_SAMPLES_TRANSFER; // TODO: stream ?
        IPC2->sound_writerequest = 0;

        IPC2->sound_lbuf = (void *) pcmL; // TODO: stream ? initialisation ?
        IPC2->sound_rbuf = (void *) pcmR; // TODO: stream ? initialisation ?

//        readbuffer = (stream->channel_count == 2 ? 
//            pj_pool_zalloc(stream->pool, buffer_size * 4) : NULL);
        
        IPC2->sound_control = IPC2_SOUND_START;
        while(IPC2->sound_state != IPC2_PLAYING);

        TRACE_((THIS_FILE, "pjmedia_snd_stream_start : play back started"));
    }
    
    /* Create capture stream */
    if (stream->dir & PJMEDIA_DIR_CAPTURE) 
    {
        TRACE_((THIS_FILE, "pjmedia_snd_stream_start : capture starting..."));
        
        IPC2->sound_frequency = stream->clock_rate;
        IPC2->sound_bytes_per_sample = stream->bytes_per_sample;
        IPC2->sound_samples = stream->samples_per_frame;
        IPC2->micro_readrequest = 0;
        
        IPC2->micro_buf = (void *) mic;
        
        IPC2->sound_control = IPC2_RECORD_START;
        while(IPC2->sound_state != IPC2_RECORDING);

        TRACE_((THIS_FILE, "pjmedia_snd_stream_start : capture started..."));
    }
    return PJ_SUCCESS;
}

/**
 */
PJ_DEF(pj_status_t) pjmedia_snd_stream_stop(pjmedia_snd_stream *stream)
{
    TRACE_((THIS_FILE, "pjmedia_snd_stream_stop."));
    if (stream->dir & PJMEDIA_DIR_PLAYBACK) 
    {
        TRACE_((THIS_FILE, "pjmedia_snd_stream_stop : play back stopping..."));
//        if(IPC2->sound_state == IPC2_STOPPED)
//            return PJ_SUCCESS;
    
        if(IPC2->sound_state == IPC2_PLAYING)
        {
            IPC2->sound_control = IPC2_SOUND_STOP;
            while(IPC2->sound_state != IPC2_STOPPED);
        }    

        buffer_samples = buffer_end = buffer_pos = 0;
        TRACE_((THIS_FILE, "pjmedia_snd_stream_stop : play back stopped"));        
    }
    
    if (stream->dir & PJMEDIA_DIR_CAPTURE)
    {
        TRACE_((THIS_FILE, "pjmedia_snd_stream_stop : capture stopping %d...",
            IPC2->sound_state));
           
        if(IPC2->sound_state == IPC2_RECORDING)
        {
            IPC2->sound_control = IPC2_RECORD_STOP;
            while(IPC2->sound_state != IPC2_STOPPED);
        }

        TRACE_((THIS_FILE, "pjmedia_snd_stream_stop : capture stopped..."));       
    } 
    
    if (--interrupt_handler_started == 0)
    {
        irqClear(IRQ_IPC_SYNC);
    }
    
    return PJ_SUCCESS;
}

/**
 */
PJ_DEF(pj_status_t) pjmedia_snd_stream_get_info(pjmedia_snd_stream *strm,
						pjmedia_snd_stream_info *pi)
{

    PJ_ASSERT_RETURN(strm && pi, PJ_EINVAL);
    TRACE_((THIS_FILE, "pjmedia_snd_stream_get_info."));
    pj_bzero(pi, sizeof(pjmedia_snd_stream_info));
    pi->dir = strm->dir;
    pi->play_id = strm->play_id;
    pi->rec_id = strm->rec_id;
    pi->clock_rate = strm->clock_rate;
    pi->channel_count = strm->channel_count;
    pi->samples_per_frame = strm->samples_per_frame;
    pi->bits_per_sample = strm->bytes_per_sample * 8;
    pi->rec_latency = 0;
    pi->play_latency = 0;
    
    return PJ_SUCCESS;
}

/**
 */
PJ_DEF(pj_status_t) pjmedia_snd_stream_close(pjmedia_snd_stream *stream)
{
    pj_pool_t *pool;
    
    PJ_ASSERT_RETURN(stream != NULL, PJ_EINVAL);

    TRACE_((THIS_FILE, "pjmedia_snd_stream_close."));

    if (stream->dir & PJMEDIA_DIR_PLAYBACK)
    {
        play_strm = NULL;
    }
    
    if (stream->dir & PJMEDIA_DIR_CAPTURE)
    {
        rec_strm = NULL;
    }
    
    pool = stream->pool;
    if (pool) 
    { 
        stream->pool = NULL;
        pj_pool_release(pool);
    }

    return PJ_SUCCESS;
}

/**
 */
PJ_DEF(pj_status_t) pjmedia_snd_deinit(void)
{
    // TODO: sur ARM7 powerOFF(POWER_SOUND)
	TRACE_((THIS_FILE, "pjmedia_snd_deinit."));

    snd_pool_factory = NULL;

    return PJ_SUCCESS;
}

#endif
