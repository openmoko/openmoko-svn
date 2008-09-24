
ECORE_PATH=$(E_PATH)/ecore/src/lib/ecore

CFLAGS = -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include/ -I/usr/include/python2.5 -I/usr/include/pygtk-2.0/ -I$(ECORE_PATH) -I. -g
LDFLAGS = -lecore -L/usr/local/lib -lglib-2.0

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
