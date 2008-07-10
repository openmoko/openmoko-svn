ifeq ($(CT_OS_TYPE),linux)
obj-m += sdio_pciellen_hcd.o

sdio_pciellen_hcd-objs := sdio_hcd.o linux/sdio_hcd_os.o
endif


ifeq ($(CT_OS_TYPE),qnx)
##QNX
LIST=VARIANT
ifndef QRECURSE
QRECURSE=recurse.mk
ifdef QCONFIG
QRDIR=$(dir $(QCONFIG))
endif
endif
include $(QRDIR)$(QRECURSE)
endif
  