# AR6K Host driver makefile
# Copyright 2006 Atheros Communications, Inc.
# 
# Minimal build invocation:
#   
#     make (all | clean | clobber)  
#
# Extended Make invocation:
#
#     make ATH_BUILD_TYPE=<build type> ATH_SDIO_STACK_BASE=<sdio stack install path> (all | clean | clobber)
#
# Notes:
#     1. This makefile must be invoked from the host/ directory
#     2. The <build type> must match an entry in localmake.linux.inc.
#     3. The localmake.linux.inc can be overridden using an include file outside the build tree.
#        This file (name and path) can be set via the ATH_MAKE_INCLUDE_OVERRIDE variable.
#        ** If ATH_MAKE_INCLUDE_OVERRIDE is used, you can define all build variables in that file 
#        instead of using command line arguments **.  This feature is provided for developers
#        that may want to customize the build using a single include file.
#
#        For example :
#             
#            " make ATH_MAKE_INCLUDE_OVERRIDE=$HOME/mymake.inc "
#        
#             could be used, as long as "mymake.inc" defines all the required variables (see below)
#
#  Required Variables:  
#      
#      ATH_OS_SUB_TYPE - on linux, this must be "linux_2_4" for 2.4 kernels or left blank for 2.6 kernels.
#      ATH_LINUXPATH - linux kernel source path
#      ATH_CROSS_COMPILE_TYPE - optional cross compiler path , leave blank for local gcc compilation
#      ATH_ARCH_CPU_TYPE  - CPU architecture type, leave blank for local gcc compilation
#      ATH_SDIO_STACK_BASE - SDIO Stack installation path to compile the SDIO HIF layer against an externally supplied
#                            SDIO stack source.
#
#  Override variables:
#
#    ATH_MAKE_INCLUDE_OVERRIDE - full path to include file which overrides the default (localmake.linux.inc)
#                                this file can contain other overrides specific to a developers
#                                workspace environment.
#    ATH_BUILD_OUTPUT_OVERRIDE - output path override for compiled executable and
#                                database image
#

# Include local variables
ifdef ATH_MAKE_INCLUDE_OVERRIDE
_LOCALMAKE_INCLUDE = $(ATH_MAKE_INCLUDE_OVERRIDE)
else
_LOCALMAKE_INCLUDE = localmake.linux.inc
endif

-include $(_LOCALMAKE_INCLUDE)

export  ATH_SRC_BASE
export  ATH_BUILD_TYPE
export  ATH_OS_SUB_TYPE
export  ATH_LINUXPATH
export  ATH_CROSS_COMPILE_TYPE
export  ATH_ARCH_CPU_TYPE
export  ATH_SDIO_STACK_BASE
export  ATH_BUS_TYPE
export  ATH_HC_DRIVERS

ATH_SRC_BASE :=$(shell pwd)
MAKE :=make
CC :=$(ATH_CROSS_COMPILE_TYPE)gcc
LD :=$(ATH_CROSS_COMPILE_TYPE)ld
STRIP :=$(ATH_CROSS_COMPILE_TYPE)strip
ATH_HIF_TYPE :=`echo $(ATH_BUS_TYPE) | tr [:upper:] [:lower:]`

_INTERNAL_BUILD = $(shell if [ -r tools/flashloader ]; then echo "YES"; else echo "NO"; fi)

export  STRIP

# Set cross compile type (if any)
ifdef ATH_CROSS_COMPILE_TYPE
_CROSS_COMPILE_LINE := ARCH=$(ATH_ARCH_CPU_TYPE) CROSS_COMPILE=$(ATH_CROSS_COMPILE_TYPE)
endif

# Figure out module extension
ifneq ($(ATH_OS_SUB_TYPE),linux_2_4)
KMOD_EXTENSION :=ko
endif
ifeq ($(ATH_OS_SUB_TYPE),linux_2_4)
KMOD_EXTENSION :=o
endif

# Set up object output areas
ifdef ATH_BUILD_OUTPUT_OVERRIDE
_COMPILED_OBJECTS_PATH :=$(ATH_BUILD_OUTPUT_OVERRIDE)
COMPILED_BIN_OBJECTS_PATH :=$(ATH_BUILD_OUTPUT_OVERRIDE)
COMPILED_IMAGE_OBJECTS_PATH :=$(ATH_BUILD_OUTPUT_OVERRIDE)
_MAKE_IMAGE_OUTPUT_DIR :=
_MAKE_BIN_OUTPUT_DIR :=
_CLEAN_IMAGE_OUTPUT_DIR :=
_CLEAN_BIN_OUTPUT_DIR :=
else
_COMPILED_OBJECTS_PATH := $(ATH_SRC_BASE)/.output/$(ATH_BUILD_TYPE)-$(ATH_BUS_TYPE)
COMPILED_BIN_OBJECTS_PATH := $(ATH_SRC_BASE)/.output/bin
COMPILED_IMAGE_OBJECTS_PATH :=$(_COMPILED_OBJECTS_PATH)/$(COMPILED_OBJECTS_PREFIX)image
_MAKE_IMAGE_OUTPUT_DIR := mkdir --parents $(COMPILED_IMAGE_OBJECTS_PATH)
_MAKE_BIN_OUTPUT_DIR := mkdir --parents $(COMPILED_BIN_OBJECTS_PATH)
_CLEAN_IMAGE_OUTPUT_DIR := rm -R -f $(COMPILED_IMAGE_OBJECTS_PATH)
_CLEAN_BIN_OUTPUT_DIR := rm -R -f $(COMPILED_BIN_OBJECTS_PATH)
endif

# Export the required variables to other Makefiles in the system
export COMPILED_IMAGE_OBJECTS_PATH
export COMPILED_BIN_OBJECTS_PATH

all: modules

modules:
ifndef ATH_BUILD_TYPE 
	@echo "Please edit the $(_LOCALMAKE_INCLUDE) file"
	exit 1
endif
	$(_MAKE_IMAGE_OUTPUT_DIR)
	$(_MAKE_BIN_OUTPUT_DIR)
	$(MAKE)  $(_CROSS_COMPILE_LINE) -C $(ATH_LINUXPATH) ATH_HIF_TYPE=$(ATH_HIF_TYPE) SUBDIRS=$(ATH_SRC_BASE)/os/linux modules
	$(MAKE) -C hif/$(ATH_HIF_TYPE)
	$(MAKE) -C tools/bmiloader/
	$(MAKE) -C tools/wmiconfig/
ifeq ($(_INTERNAL_BUILD), YES)
	$(MAKE) -C tools/mkdsetimg/
	$(MAKE) -C tools/eeprom/
	$(MAKE) -C tools/flashloader/
	$(MAKE) -C tools/forcereset/
	$(MAKE) -C tools/regDbGen/
	$(MAKE) -C tools/ini2dset/
	$(MAKE) -C tools/tcmd/
	$(MAKE) -C tools/createimage/
	$(MAKE) -C tools/checkTargetStatus/
#	$(MAKE) -C tools/phyregs/
	$(MAKE) -C tools/recEvent/
	$(MAKE) -C tools/rompatcher/
	$(MAKE) -C tools/bdiff/
	$(MAKE) -C tools/licensing/
	$(MAKE) -C tests/floodtest/
	$(MAKE) -C tests/mboxping/
#	$(MAKE) -C tests/dsetpatch/
	$(MAKE) -C 3rdparty/supplicant/opensrc_0_4_9/
	$(MAKE) -C 3rdparty/pgen/
#	$(MAKE) -C 3rdparty/WPS/Intel_SDK_1_0_5/linux/	
#	$(MAKE) -C dwsim/
endif
	cp -f $(ATH_SRC_BASE)/os/linux/ar6000.$(KMOD_EXTENSION) $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tools/bmiloader/bmiloader $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tools/wmiconfig/wmiconfig $(COMPILED_IMAGE_OBJECTS_PATH)
ifeq ($(_INTERNAL_BUILD), YES)	
	cp -f tools/mkdsetimg/mkdsetimg $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tools/eeprom/eeprom.AR6001 $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tools/eeprom/eeprom.AR6002 $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tools/flashloader/flashloader $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tools/forcereset/forcereset $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tools/createimage/createimage $(COMPILED_IMAGE_OBJECTS_PATH)
#	cp -f tools/phyregs/phyregs $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tools/checkTargetStatus/checkTargetStatus $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tools/tcmd/athtestcmd $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tools/recEvent/recEvent $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tools/recEvent/restore.sh $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tools/rompatcher/rompatcher $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tools/rompatcher/fwpatch $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tools/bdiff/bdiff $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tests/floodtest/floodtest $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tests/mboxping/mboxping $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f tools/regDbGen/regulatory*.bin $(COMPILED_BIN_OBJECTS_PATH)
#	cp -f tests/dsetpatch/regulatory*.diff $(COMPILED_BIN_OBJECTS_PATH)
	cp -rf tools/ini2dset/ini_ar600* $(COMPILED_BIN_OBJECTS_PATH)
	cp -f 3rdparty/supplicant/opensrc_0_4_9/wpa_supplicant  $(COMPILED_IMAGE_OBJECTS_PATH)
	cp -f 3rdparty/supplicant/opensrc_0_4_9/wpa_cli  $(COMPILED_IMAGE_OBJECTS_PATH)	
	cp -f 3rdparty/pgen/pgen  $(COMPILED_IMAGE_OBJECTS_PATH)	
	cp -f 3rdparty/pgen/pget  $(COMPILED_IMAGE_OBJECTS_PATH)	
	cp -f 3rdparty/pgen/phone  $(COMPILED_IMAGE_OBJECTS_PATH)	
	cp -f 3rdparty/pgen/staut  $(COMPILED_IMAGE_OBJECTS_PATH)	
	cp -f 3rdparty/pgen/upsd  $(COMPILED_IMAGE_OBJECTS_PATH)	
#	cp -f 3rdparty/WPS/Intel_SDK_1_0_5/linux/wsccmd  $(COMPILED_IMAGE_OBJECTS_PATH)			
#	cp -f 3rdparty/WPS/Intel_SDK_1_0_5/linux/wsc_config.txt  $(COMPILED_IMAGE_OBJECTS_PATH)				
#	if [ -r  $(ATH_SRC_BASE)/3rdparty/supplicant/ds/linux/bin/$(ATH_BUILD_TYPE) ]; then \
#		cp -f  $(ATH_SRC_BASE)/3rdparty/supplicant/ds/linux/bin/$(ATH_BUILD_TYPE)/*  $(COMPILED_IMAGE_OBJECTS_PATH);\
#	fi
endif

clean:
	rm -f os/linux/ar6000.mod.c os/linux/*.o os/linux/*.ko wmi/*.o htc/AR6000/src/*.o htc/*.o bmi/src/*.o wlan/src/*.o driver/* apps/wpa_supplicant/*.o tools/bmiloader/bmiloader tools/dsetsvr/dsetsvr tools/flashloader/flashloader tools/forcereset/forcereset tools/regDbGen/regDomainGenDb tools/regDbGen/*.bin tools/rompatcher/rompatcher tools/rompatcher/fwpatch tests/floodtest/floodtest tests/mboxping/mboxping tools/wmiconfig/wmiconfig 
	rm -f os/linux/ar6000.mod.c os/linux/*.o os/linux/*.ko wmi/*.o htc/AR6000/src/*.o htc/*.o bmi/src/*.o wlan/src/*.o driver/* apps/wpa_supplicant/*.o tools/bmiloader/bmiloader tools/dsetsvr/dsetsvr tools/flashloader/flashloader tools/forcereset/forcereset tools/regDbGen/regDomainGenDb tools/regDbGen/*.bin tools/rompatcher/rompatcher tools/rompatcher/fwpatch tests/floodtest/floodtest tests/mboxping/mboxping tools/wmiconfig/wmiconfig tools/mkdsetimg/mkdsetimg tools/eeprom/eeprom
	rm -f os/linux/ar6000.mod.c os/linux/*.o os/linux/*.ko wmi/*.o htc/AR6000/src/*.o htc/*.o bmi/src/*.o wlan/src/*.o driver/* apps/wpa_supplicant/*.o tools/bmiloader/bmiloader tools/dsetsvr/dsetsvr tools/flashloader/flashloader tools/forcereset/forcereset tools/regDbGen/regDomainGenDb tools/regDbGen/*.bin tools/rompatcher/rompatcher tools/rompatcher/fwpatch tests/floodtest/floodtest tests/mboxping/mboxping tools/wmiconfig/wmiconfig dwsim/*.o dwsim/dwsim tools/mkdsetimg/mkdsetimg tools/eeprom/eeprom.AR6001 tools/eeprom/eeprom.AR6002

# The kernel module build process leaves some intermediate files, this will clean up all these files
	find $(ATH_SRC_BASE) \( -not -path "*.output*" -a -name '*.[oas]' -o -name core -o -name '.*.flags' -o -name '.ko' -o -name '.*.cmd' \) -type f -print \
		| grep -v lxdialog/ | xargs rm -f
	$(MAKE) -C hif/$(ATH_HIF_TYPE) clean
ifeq ($(_INTERNAL_BUILD), YES)		
	$(MAKE) -C tools/ini2dset clean
	$(MAKE) -C tests/dsetpatch/ clean
endif

clobber:clean
	rm -rf .output
