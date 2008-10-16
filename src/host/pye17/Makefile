
# E_PATH not given (env variable)
ifeq ($(wildcard $(E_PATH)),)

E_PATH_TMP=~/openmoko/e17/libs
ifneq ($(wildcard $(E_PATH_TMP)),)
E_PATH = $(E_PATH_TMP)
endif

endif

ifeq ($(wildcard $(E_PATH)),)
$(error Path to enlightenment libaries variable is missing, please set E_PATH (e.g. ~/e17/libs))
endif


PWD := $(shell pwd)
CODEGEN_PATH = $(PWD)/codegen
EXAMPLE_PATH = $(PWD)/examples
PYTHON = $(shell which python)


e_bindings = evas ecore ecore_evas edje

### ToDo ###
# eet ? embryo ?


e_bindings_mk = $(e_bindings:=.mk)

.PHONY: all clean


all:	$(e_bindings_mk)

clean:	all

examples: all

%.mk:
	$(MAKE) -C $*/ CODEGEN_PATH=$(CODEGEN_PATH) E_PATH=$(E_PATH) PYTHON=$(PYTHON) EXAMPLE_PATH=$(EXAMPLE_PATH) $(MAKECMDGOALS)
