#
#SDIO HIF makefile for atheros SDIO stack
#  
#Copyright (c) 2004-2006 Atheros Communications Inc.
#
#$ATH_LICENSE_HOSTSDK0_MK$
#

# Check for SDIO stack
ifdef ATH_SDIO_STACK_BASE
# Someone already set it on entry, the stack resides outside this tree, we will try to build it
_SDIO_STACK = YES
else
# Check for SDIO stack within this tree
_SDIO_STACK = $(shell if [ -f $(ATH_SRC_BASE)/sdiostack/src/Makefile ]; then echo "YES"; else echo "NO"; fi)

ifeq ($(_SDIO_STACK), YES)
    # SDIO stack is part of the kit and will need to be compiled
ATH_SDIO_STACK_BASE := $(ATH_SRC_BASE)/sdiostack
endif
endif 

ifeq ($(_SDIO_STACK), YES)
    # Pass and translate build variables to the SDIO stack makefile
_SDIO_STACK_MAKE_PARAMS := CT_BUILD_TYPE=$(ATH_BUILD_TYPE) \
                           CT_OS_TYPE=linux \
                           CT_OS_SUB_TYPE=$(ATH_OS_SUB_TYPE) \
                           CT_LINUXPATH=$(ATH_LINUXPATH) \
                           CT_BUILD_TYPE=$(ATH_BUILD_TYPE) \
                           CT_CROSS_COMPILE_TYPE=$(ATH_CROSS_COMPILE_TYPE) \
                           CT_ARCH_CPU_TYPE=$(ATH_ARCH_CPU_TYPE) \
                           CT_HC_DRIVERS=$(ATH_HC_DRIVERS) \
                           CT_MAKE_INCLUDE_OVERRIDE=$(_LOCALMAKE_INCLUDE)   \
                           CT_BUILD_OUTPUT_OVERRIDE=$(COMPILED_IMAGE_OBJECTS_PATH) \
                           BUS_BUILD=1
endif
EXTRA_CFLAGS += -I$(ATH_SRC_BASE)/hif/sdio/linux_athsdio/include
EXTRA_CFLAGS += -DSDIO
EXTRA_CFLAGS += -I$(ATH_SDIO_STACK_BASE)/src/include

ifeq ($(ATH_OS_SUB_TYPE),linux_2_4)
obj-y += ../../hif/sdio/linux_athsdio/src/hif.o
endif

ifneq ($(ATH_OS_SUB_TYPE),linux_2_4)
ar6000-objs := ../../hif/sdio/linux_athsdio/src/hif.o
endif

all:
ifeq ($(_SDIO_STACK),YES)
	$(MAKE) $(_SDIO_STACK_MAKE_PARAMS) -C  $(ATH_SDIO_STACK_BASE)/src default
endif

clean:
ifeq ($(_SDIO_STACK),YES)
	$(MAKE) $(_SDIO_STACK_MAKE_PARAMS) -C $(ATH_SDIO_STACK_BASE)/src clean
endif
