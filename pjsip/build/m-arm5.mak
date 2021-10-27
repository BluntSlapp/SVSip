ARCH	:=	-mthumb-interwork
#-mthumb 

# note: arm9tdmi isn't the correct CPU arch, but anything newer and LD
# *insists* it has a FPU or VFP, and it won't take no for an answer!
export M_CFLAGS := $(CC_DEF)PJ_M_ARMV4=1 \
		-march=armv5te -mtune=arm946e-s \
		-fomit-frame-pointer -ffast-math \
		$(ARCH)
		
export M_CXXFLAGS :=
export M_LDFLAGS := -specs=ds_arm9.specs -g $(ARCH) -mno-fpu

export M_SOURCES :=
