ifeq ($(CT_OS_TYPE),linux)
obj-m += sdio_pcistd_hcd.o

sdio_pcistd_hcd-objs := linux/sdio_hcd_os.o ../stdhost/sdio_std_hcd.o ../stdhost/linux/sdio_std_hcd_os.o
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
  