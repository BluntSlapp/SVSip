#
# OS specific configuration for Nintendo DS target. 
#

#
# PJLIB_OBJS specified here are object files to be included in PJLIB
# (the library) for this specific operating system. Object files common 
# to all operating systems should go in Makefile instead.
#
export PJLIB_OBJS += 	addr_resolv_nds.o file_access_unistd.o \
			file_io_ansi.o guid_simple.o \
			log_writer_stdout.o os_core_nds.o \
			os_error_unix.o os_time_nds.o \
			os_timestamp_common.o os_timestamp_nds.o \
			pool_policy_malloc.o sock_nds.o sock_select.o \
			ioqueue_select.o

#
# TEST_OBJS are operating system specific object files to be included in
# the test application.
#
export TEST_OBJS +=	main.o

#
# Additional LDFLAGS for pjlib-test
#
export TEST_LDFLAGS += -lm

#
# TARGETS are make targets in the Makefile, to be executed for this given
# operating system.
#
export TARGETS	    =	pjlib 
#pjlib-test

