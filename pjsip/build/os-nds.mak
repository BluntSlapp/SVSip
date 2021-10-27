export OS_CFLAGS   := $(CC_DEF)PJ_NDS=1 -DARM9 \
		-I$(DEVKITPRO)/libnds/include \
		-O2

export OS_CXXFLAGS := 

export OS_LDFLAGS  := -L$(DEVKITPRO)/libnds/lib -lfat -ldswifi9d -lnds9 -lm

export OS_SOURCES  := 


