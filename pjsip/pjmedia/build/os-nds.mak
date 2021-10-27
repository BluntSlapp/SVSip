# Generated automatically from os-auto.mak.in by configure.

# PJMEDIA features exclusion
export CFLAGS +=    

# Define the desired sound device backend
# Valid values are:
#   - pa_unix:	    	PortAudio on Unix (OSS or ALSA)
#   - pa_darwinos:  	PortAudio on MacOSX (CoreAudio)
#   - pa_old_darwinos:  PortAudio on MacOSX (old CoreAudio, for OSX 10.2)
#   - pa_win32:	    	PortAudio on Win32 (WMME)
#   - ds:	    	Win32 DirectSound (dsound.c)
#   - nds:          Nintendo DS sound device (nds.c)
#   - null:	    	Null sound device (nullsound.c)
AC_PJMEDIA_SND=nds

# For Unix, specify if ALSA should be supported
AC_PA_USE_ALSA=

# Additional PortAudio CFLAGS are in 

#
# Codecs
#
AC_NO_G711_CODEC=
AC_NO_L16_CODEC=1
AC_NO_GSM_CODEC=1
AC_NO_SPEEX_CODEC=1
AC_NO_ILBC_CODEC=1
AC_NO_G722_CODEC=1

export CODEC_OBJS=

ifeq ($(AC_NO_G711_CODEC),1)
export CFLAGS += -DPJMEDIA_HAS_G711_CODEC=0
else
export CODEC_OBJS +=
endif

ifeq ($(AC_NO_L16_CODEC),1)
export CFLAGS += -DPJMEDIA_HAS_L16_CODEC=0
else
export CODEC_OBJS += l16.o
endif

ifeq ($(AC_NO_GSM_CODEC),1)
export CFLAGS += -DPJMEDIA_HAS_GSM_CODEC=0
else
export CODEC_OBJS += $(GSM_OBJS)
endif

ifeq ($(AC_NO_SPEEX_CODEC),1)
export CFLAGS += -DPJMEDIA_HAS_SPEEX_CODEC=0
else
export CODEC_OBJS += $(SPEEX_OBJS)
endif

ifeq ($(AC_NO_ILBC_CODEC),1)
export CFLAGS += -DPJMEDIA_HAS_ILBC_CODEC=0
else
export CODEC_OBJS += $(ILBC_OBJS)
endif

ifeq ($(AC_NO_G722_CODEC),1)
export CFLAGS += -DPJMEDIA_HAS_G722_CODEC=0
else
export CODEC_OBJS += g722.o g722/g722_enc.o g722/g722_dec.o
endif

#
# PortAudio on Unix
#
ifeq ($(AC_PJMEDIA_SND),pa_unix)
# Host APIs and utils
export PJMEDIA_OBJS += $(PA_DIR)/pa_unix_hostapis.o $(PA_DIR)/pa_unix_util.o

# Include ALSA?
ifeq ($(AC_PA_USE_ALSA),1)
export CFLAGS += -DPA_USE_ALSA=1
export PJMEDIA_OBJS += $(PA_DIR)/pa_linux_alsa.o
endif

export CFLAGS += -DPA_USE_OSS=1 \
	         -DPJMEDIA_SOUND_IMPLEMENTATION=PJMEDIA_SOUND_PORTAUDIO_SOUND
export PJMEDIA_OBJS += $(PA_DIR)/pa_unix_oss.o
endif


#
# PortAudio on MacOS X (using current PortAudio)
#
ifeq ($(AC_PJMEDIA_SND),pa_darwinos)
export PJMEDIA_OBJS +=	$(PA_DIR)/pa_mac_hostapis.o \
			$(PA_DIR)/pa_unix_util.o \
			$(PA_DIR)/pa_mac_core.o \
			$(PA_DIR)/pa_mac_core_blocking.o \
			$(PA_DIR)/pa_mac_core_utilities.o \
			$(PA_DIR)/ringbuffer.o
export CFLAGS += -DPA_USE_COREAUDIO=1 \
	         -DPJMEDIA_SOUND_IMPLEMENTATION=PJMEDIA_SOUND_PORTAUDIO_SOUND
export CFLAGS += 
endif

#
# PortAudio on MacOS X (using old PortAudio, for MacOS X 10.2.x)
#
ifeq ($(AC_PJMEDIA_SND),pa_old_darwinos)
export PJMEDIA_OBJS +=	$(PA_DIR)/pa_mac_hostapis.o \
			$(PA_DIR)/pa_unix_util.o \
			$(PA_DIR)/pa_mac_core_old.o 
export CFLAGS += -DPA_USE_COREAUDIO=1 \
	         -DPJMEDIA_SOUND_IMPLEMENTATION=PJMEDIA_SOUND_PORTAUDIO_SOUND
export CFLAGS += 
endif

#
#
# PortAudio on Win32 (WMME)
#
ifeq ($(AC_PJMEDIA_SND),pa_win32)
export PJMEDIA_OBJS += $(PA_DIR)/pa_win_hostapis.o $(PA_DIR)/pa_win_util.o \
		       $(PA_DIR)/pa_win_wmme.o
export CFLAGS += -DPA_NO_ASIO -DPA_NO_DS \
	         -DPJMEDIA_SOUND_IMPLEMENTATION=PJMEDIA_SOUND_PORTAUDIO_SOUND
endif

#
# Win32 DirectSound
#
ifeq ($(AC_PJMEDIA_SND),ds)
export SOUND_OBJS = dsound.o
export CFLAGS += -DPJMEDIA_SOUND_IMPLEMENTATION=PJMEDIA_SOUND_WIN32_DIRECT_SOUND
endif

#
# Nintendo DS
#
ifeq ($(AC_PJMEDIA_SND),nds)
export SOUND_OBJS = ndssound.o
export CFLAGS += -DPJMEDIA_SOUND_IMPLEMENTATION=PJMEDIA_SOUND_NDS_SOUND \
	-I$(PJDIR)/build.nds/include
endif

#
# Last resort, null sound device
#
ifeq ($(AC_PJMEDIA_SND),null)
export SOUND_OBJS = nullsound.o
export CFLAGS += -DPJMEDIA_SOUND_IMPLEMENTATION=PJMEDIA_SOUND_NULL_SOUND
endif


