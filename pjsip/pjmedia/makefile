#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

TOPDIR ?= $(CURDIR)/..

#---------------------------------------------------------------------------------
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# DATA is a list of directories containing binary files
# all directories are relative to this makefile
#---------------------------------------------------------------------------------
BUILD		?=	release
SOURCES		:=	src/pjmedia
INCLUDES	:=	include src/pjmedia-codec src/pjmedia/portaudio \
	../pjsip-apps/src/samples/nds ..
DATA		:=	data

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-mthumb -mthumb-interwork

# note: arm9tdmi isn't the correct CPU arch, but anything newer and LD
# *insists* it has a FPU or VFP, and it won't take no for an answer!
CFLAGS	:=	-Wall -O2\
 			-march=armv5te -mtune=arm946e-s \
 			-fomit-frame-pointer\
			-ffast-math \
			$(ARCH)
#-mcpu=arm9tdmi -mtune=arm9tdmi

CFLAGS	+=	$(INCLUDE) -DARM9

CFLAGS	+=	-DPJ_M_ARMV4=1 -DPJ_NDS=1

CFLAGS  +=  -DPJMEDIA_HAS_SPEEX_AEC=0
CXXFLAGS	:=	$(CFLAGS)

#ASFLAGS	:=	-g $(ARCH)
ASFLAGS	:=	$(ARCH) -march=armv5te -mtune=arm946e-s
LDFLAGS	=	-specs=ds_arm9.specs -g $(ARCH) -mno-fpu -Wl,-Map,$(notdir $*.map)

ifneq ($(BUILD),debug)
export ARM9BIN	:=	$(TOPDIR)/lib/libpjmedia-arm-nds.a
else
export ARM9BIN	:=	$(TOPDIR)/lib/libpjmedia-arm-nds-d.a
CFLAGS	+= -g
ASFLAGS	+= -g 
endif


#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:= 
 
#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(LIBNDS) ../../pjlib ../../pjlib-util ../../pjnath
 
#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------


export DEPSDIR := $(CURDIR)/$(BUILD)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))
 
export CC		:=	$(PREFIX)gcc
export CXX		:=	$(PREFIX)g++
export AR		:=	$(PREFIX)ar
export OBJCOPY	:=	$(PREFIX)objcopy
 
#CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CFILES      := alaw_ulaw.c alaw_ulaw_table.c clock_thread.c codec.c \
			conference.c echo_common.c echo_port.c \
			echo_suppress.c endpoint.c errno.c \
			g711.c jbuf.c master_port.c mem_capture.c mem_player.c \
			null_port.c plc_common.c port.c splitcomb.c \
			resample_resample.c resample_libsamplerate.c \
			resample_port.c rtcp.c rtp.c sdp.c sdp_cmp.c sdp_neg.c \
			session.c silencedet.c sound_port.c stream.c \
			tonegen.c transport_ice.c transport_udp.c \
			wav_player.c wav_playlist.c wav_writer.c wave.c
#echo_speex.c			
			
#CFILES      += nullsound.c
CFILES      += ndssound.c

#CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
 
#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
 
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)
 
.PHONY: $(BUILD) clean
 
#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/makefile
 
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr debug release

all: $(ARM9BIN) 
 
#---------------------------------------------------------------------------------
else
 
DEPENDS	:=	$(OFILES:.o=.d)
 
#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(ARM9BIN)	:	$(OFILES)
	@rm -f "$(ARM9BIN)"
	@$(AR) rcs "$(ARM9BIN)" $(OFILES)
	@echo built ... $(notdir $@)

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data 
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)


-include $(DEPENDS)
 
#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
