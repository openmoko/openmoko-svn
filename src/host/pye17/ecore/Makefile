CFLAGS = `pkg-config --cflags ecore glib-2.0 pygtk-2.0` `python-config --includes`
LDFLAGS = `pkg-config --libs ecore glib-2.0 pygtk-2.0` `python-config --libs`

ECORE_PATH = $(E_PATH)/ecore/src/lib/ecore
LIB_NAME = ecore.so

.PHONY: all clean examples

all: $(LIB_NAME)

$(LIB_NAME): ecore.o ecoremodule.o
	$(CC) $(LDFLAGS) -shared $^ -o $@

ecore.c: ecore.defs ecore.override
	$(PYTHON) $(CODEGEN_PATH)/codegen.py --prefix ecore --override ecore.override ecore.defs > $@

ecore.defs: $(CODEGEN_PATH)/h2def.py $(ECORE_PATH)/Ecore.h $(ECORE_PATH)/Ecore_Data.h
	$(CODEGEN_PATH)/h2def.py $(ECORE_PATH)/Ecore.h > $@
	$(CODEGEN_PATH)/h2def.py $(ECORE_PATH)/Ecore_Data.h >> $@

clean:
	rm -f *.o *~ ecore.defs ecore.c $(LIB_NAME) $(EXAMPLE_PATH)/$(LIB_NAME) $(EXAMPLE_PATH)/*/$(LIB_NAME)

examples: $(LIB_NAME) $(EXAMPLE_PATH)/$(LIB_NAME) $(EXAMPLE_PATH)/ecore_evas/$(LIB_NAME)

$(EXAMPLE_PATH)/$(LIB_NAME): $(LIB_NAME)
	cp $(LIB_NAME) $(EXAMPLE_PATH)

$(EXAMPLE_PATH)/ecore_evas/$(LIB_NAME):
	ln -s $(EXAMPLE_PATH)/$(LIB_NAME) $(EXAMPLE_PATH)/ecore_evas/
