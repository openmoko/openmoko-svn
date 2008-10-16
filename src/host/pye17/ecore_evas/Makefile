CFLAGS = `pkg-config --cflags ecore-evas glib-2.0 pygtk-2.0` `python-config --includes`
LDFLAGS = `pkg-config --libs ecore-evas glib-2.0 pygtk-2.0` `python-config --libs`

ECORE_EVAS_PATH = $(E_PATH)/ecore/src/lib/ecore_evas
LIB_NAME = ecore_evas.so

.PHONY: all clean examples

all: $(LIB_NAME)

$(LIB_NAME): ecore_evas.o ecore_evasmodule.o
	$(CC) $(LDFLAGS) -shared $^ -o $@

ecore_evas.c: ecore_evas.defs ecore_evas.override
	$(PYTHON) $(CODEGEN_PATH)/codegen.py --prefix ecore_evas --override ecore_evas.override ecore_evas.defs > $@

ecore_evas.defs: $(CODEGEN_PATH)/h2def.py $(ECORE_EVAS_PATH)/Ecore_Evas.h
	$(CODEGEN_PATH)/h2def.py $(ECORE_EVAS_PATH)/Ecore_Evas.h > $@

clean:
	rm -f *.o *~ ecore_evas.defs ecore_evas.c $(LIB_NAME) $(EXAMPLE_PATH)/$(LIB_NAME) $(EXAMPLE_PATH)/*/$(LIB_NAME)

examples: $(LIB_NAME) $(EXAMPLE_PATH)/$(LIB_NAME)
	ln -s $(EXAMPLE_PATH)/$(LIB_NAME) $(EXAMPLE_PATH)/ecore_evas/

$(EXAMPLE_PATH)/$(LIB_NAME):
	cp $(LIB_NAME) $(EXAMPLE_PATH)
