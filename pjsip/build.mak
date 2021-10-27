# build.mak.  Generated from build.mak.in by configure.
export MACHINE_NAME := arm5
export OS_NAME := nds
export HOST_NAME := devkitpro
export CC_NAME := gcc
export TARGET_NAME := arm-nds-mingw32
export CROSS_COMPILE := arm-eabi-
export LINUX_POLL := select 

# Determine which party libraries to use
export APP_THIRD_PARTY_LIBS := 
#-lresample-$(TARGET_NAME)

ifneq (1,1)
APP_THIRD_PARTY_LIBS += -lgsmcodec-$(TARGET_NAME)
endif

ifneq (1,1)
APP_THIRD_PARTY_LIBS += -lspeex-$(TARGET_NAME)
endif

ifneq (1,1)
APP_THIRD_PARTY_LIBS += -lilbccodec-$(TARGET_NAME)
endif

ifneq ($(findstring pa,nds),)
APP_THIRD_PARTY_LIBS += -lportaudio-$(TARGET_NAME)
endif


# CFLAGS, LDFLAGS, and LIBS to be used by applications
export PJDIR := /g/Samuel/sources/c/pjproject-svn
export APP_CC := $(CROSS_COMPILE)$(CC_NAME)
export APP_CFLAGS := -DPJ_NDS=1\
	-O2\
	-I$(PJDIR)/pjlib/include\
	-I$(PJDIR)/pjlib-util/include\
	-I$(PJDIR)/pjnath/include\
	-I$(PJDIR)/pjmedia/include\
	-I$(PJDIR)/pjsip/include\
	-L$(DEVKITPRO)/libnds/include

export APP_CXXFLAGS := $(APP_CFLAGS)
export APP_LDFLAGS := -L$(PJDIR)/pjlib/lib\
	-L$(PJDIR)/pjlib-util/lib\
	-L$(PJDIR)/pjnath/lib\
	-L$(PJDIR)/pjmedia/lib\
	-L$(PJDIR)/pjsip/lib\
	-L$(PJDIR)/third_party/lib\
	-L$(DEVKITPRO)/libnds/lib
	
export APP_LDLIBS := -lpjsua-$(TARGET_NAME)\
	-lpjsip-ua-$(TARGET_NAME)\
	-lpjsip-simple-$(TARGET_NAME)\
	-lpjsip-$(TARGET_NAME)\
	-lpjmedia-codec-$(TARGET_NAME)\
	-lpjmedia-$(TARGET_NAME)\
	-lpjnath-$(TARGET_NAME)\
	-lpjlib-util-$(TARGET_NAME)\
	$(APP_THIRD_PARTY_LIBS)\
	-lpj-$(TARGET_NAME)\
	-lfat -ldswifi9d -lnds9 -lm 

export PJ_DIR := $(PJDIR)
export PJ_CC := $(APP_CC)
export PJ_CFLAGS := $(APP_CFLAGS)
export PJ_CXXFLAGS := $(APP_CXXFLAGS)
export PJ_LDFLAGS := $(APP_LDFLAGS)
export PJ_LDLIBS := $(APP_LDLIBS)

