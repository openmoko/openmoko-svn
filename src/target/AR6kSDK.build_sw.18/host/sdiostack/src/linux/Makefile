ifeq ($(CT_OS_TYPE),linux)

ifneq ($(CT_OS_SUB_TYPE),linux_2_4)
ifeq ($(ALL_BUILD),1)
obj-m += sdioplatformdriver/
else 
ifeq ($(FUNCTION_BUILD),1)
# use this to test PDK headers
obj-m += sdioplatformdriver/
else 
ifeq ($(PDK_BUILD),1)
obj-m += sdioplatformdriver/
else
ifeq ($(HDK_BUILD),1)
obj-m += sdioplatformdriver/
endif
endif
endif
endif

endif

ifeq ($(CT_OS_SUB_TYPE),linux_2_4)
endif

include $(CT_OS_TOP_LEVEL_RULE)
endif
endif


