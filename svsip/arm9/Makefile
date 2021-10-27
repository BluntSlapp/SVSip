#
# SvSIP, SIP/VoIP client for Nintendo DS
# Copyright (C) 2007-2009  Samuel Vinson <samuelv0304@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

#---------------------------------------------------------------------------------
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# DATA is a list of directories containing binary files
# all directories are relative to this makefile
#---------------------------------------------------------------------------------
BUILD		:=	build
SOURCES		:=	src  
INCLUDES	:=	include ../common ../third_party/libpng-1.2.19  ../third_party/zlib-1.2.3
DATA		:= data

THIRD_PARTY_PATH := path

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-mthumb-interwork

CFLAGS	:=	-O2 -s \
 			-march=armv5te -mtune=arm946e-s -fomit-frame-pointer\
			-ffast-math \
			$(ARCH)
#-Werror 

CFLAGS	+=	$(INCLUDE) -DARM9
CFLAGS  +=  -DPJ_M_ARMV4=1 -DPJ_NDS=1
CFLAGS  +=  $(ISREBOOT)

CXXFLAGS	:=	$(CFLAGS) -fno-rtti -fno-exceptions

ASFLAGS	:= $(ARCH) -march=armv5te -mtune=arm946e-s

LDFLAGS	=	-specs=ds_arm9.specs $(ARCH) -mno-fpu -Wl,-Map,$(notdir $*.map)

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
DEBUG_LIB := -d
LIBS	:=  \
		-lpjsua-arm-nds$(DEBUG_LIB) \
		-lpjsip-simple-arm-nds$(DEBUG_LIB) \
		-lpjsip-ua-arm-nds$(DEBUG_LIB) \
		-lpjsip-arm-nds$(DEBUG_LIB) \
		-lpjmedia-arm-nds$(DEBUG_LIB) \
		-lpjnath-arm-nds$(DEBUG_LIB) \
        -lpjlib-util-arm-nds$(DEBUG_LIB)\
        -lpj-arm-nds$(DEBUG_LIB) \
		-lm -lfat -lnds9 -ldswifi9 -lpng -lz 
#        -lresample-arm-nds$(DEBUG_LIB)
 
ifneq ($(ISREBOOT), )
LIBS += -lrebootlib9
endif
 
#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(LIBNDS) \
	$(THIRD_PARTY_PATH)/svsip/third_party/libpng-1.2.19 \
	$(THIRD_PARTY_PATH)/svsip/third_party/zlib-1.2.3 \
	$(THIRD_PARTY_PATH)/pjproject-svn/pjsip \
	$(THIRD_PARTY_PATH)/pjproject-svn/pjlib \
	$(THIRD_PARTY_PATH)/pjproject-svn/pjlib-util \
	$(THIRD_PARTY_PATH)/pjproject-svn/pjmedia \
	$(THIRD_PARTY_PATH)/pjproject-svn/pjnath
 
#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
 
export ARM9BIN	:=	$(TOPDIR)/$(TARGET).arm9
export ARM9ELF	:=	$(CURDIR)/$(TARGET).arm9.elf
export DEPSDIR := $(CURDIR)/$(BUILD)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))
 
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
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
			-I$(CURDIR)/$(BUILD) -I$(CURDIR)/../common/include \
			-I$(DEVKITPRO)/libfat/include -I$(ARM9LIBS)/include
 
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
			-L$(DEVKITPRO)/libfat/nds/lib -L$(DEVKITPRO)/libfat/lib \
			-L$(ARM9LIBS)/lib

.PHONY: $(BUILD) clean
 
#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
 
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) *.elf *.nds* *.bin 
 
 
#---------------------------------------------------------------------------------
else
 
DEPENDS	:=	$(OFILES:.o=.d)
 
#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(ARM9BIN)	:	$(ARM9ELF)
	$(OBJCOPY) -O binary $< $@
	echo built ... $(notdir $@)

$(ARM9ELF)	:	$(OFILES)
	echo linking $(notdir $@)
	$(LD)  $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data 
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	echo $(notdir $<)
	$(bin2o)

%.raw.o	:	%.raw
#---------------------------------------------------------------------------------
	echo $(notdir $<)
	$(bin2o)


-include $(DEPENDS)
 
#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
