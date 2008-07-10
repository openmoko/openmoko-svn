#
#  Local Makefile includes for tool and kernel source paths
#  ***** This is only a sample, modify the paths for your specific build environment *****
# 
#  Copyright 2004-2006 Atheros Communications, Inc.
#
# When creating a new build type, use the following template:
#  	ifeq ($(ATH_BUILD_TYPE),<platform name>) 
#       ATH_ARCH_CPU_TYPE :=  < cpu architecture >
#       ATH_CROSS_COMPILE_TYPE := < cross compiler path >
#       ATH_LINUXPATH := < kernel source path >
#	endif 
#
# For a complete list of supported platforms, please refer to the documentation
# packaged in the release
#


#ATH_BUILD_TYPE := <Platform (Eg: LOCAL_i686)>
#ATH_BUS_TYPE := <Interconnect type (Eg: SDIO, SPI)>
#ATH_LINUXPATH := <Pointer to kernel source (Eg: /<path>/fc3-i686-2.6.9)>
#ATH_ARCH_CPU_TYPE := <Type of architecture (Eg: arm)>
#ATH_CROSS_COMPILE_TYPE := <Path to the compiler toolchain - Eg: (/<path>/arm_v4t_le-)
#ATH_OS_SUB_TYPE := <Kernel version - (Eg: linux_2_4, linux_2_6)>
#ATH_HC_DRIVERS := <sdio stack host controller drivers (optional)>

# Example: Compiling locally (i686) for an SDIO interconnect using Atheros SDIO stack
#ATH_BUILD_TYPE=LOCAL_i686
# set linux kernel source path here (or set environment variable):
#ATH_LINUXPATH :=$(KERNEL_SRC)
#ATH_BUS_TYPE :=SDIO
#ATH_HC_DRIVERS :='pci_ellen/ pci_std/'

