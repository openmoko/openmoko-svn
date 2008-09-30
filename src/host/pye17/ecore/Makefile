CFLAGS = `pkg-config --cflags ecore glib-2.0 pygtk-2.0` `python-config --includes`
LDFLAGS = `pkg-config --libs ecore glib-2.0 pygtk-2.0` `python-config --libs`

ECORE_PATH=$(E_PATH)/ecore/src/lib/ecore

.PHONY: all clean ecore.so

all: ecore.so

ecore.so: ecore.o ecoremodule.o
	$(CC) $(LDFLAGS) -shared $^ -o $@

ecore.c: ecore.defs ecore.override
	$(PYTHON) $(CODEGEN_PATH)/codegen.py --prefix ecore --override ecore.override ecore.defs > $@

ecore.defs: $(CODEGEN_PATH)/h2def.py $(ECORE_PATH)/Ecore.h $(ECORE_PATH)/Ecore_Data.h
	$(CODEGEN_PATH)/h2def.py $(ECORE_PATH)/Ecore.h > $@
	$(CODEGEN_PATH)/h2def.py $(ECORE_PATH)/Ecore_Data.h >> $@

clean:
	rm -f *.o *~ ecore.defs ecore.c ecore.so
