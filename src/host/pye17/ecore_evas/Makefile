
ECORE_EVAS_PATH=$(E_PATH)/ecore/src/lib/ecore_evas

CFLAGS = -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include/ -I/usr/include/python2.5 -I/usr/include/pygtk-2.0/ -I$(ECORE_PATH) -I. -g
LDFLAGS = -lecore_evas -L/usr/local/lib -lglib-2.0

.PHONY: all clean ecore_evas.so

all: ecore_evas.so

ecore_evas.so: ecore_evas.o ecore_evasmodule.o
	$(CC) $(LDFLAGS) -shared $^ -o $@

ecore_evas.c: ecore_evas.defs ecore_evas.override
	$(PYTHON) $(CODEGEN_PATH)/codegen.py --prefix ecore_evas --override ecore_evas.override ecore_evas.defs > $@

ecore_evas.defs: $(CODEGEN_PATH)/h2def.py $(ECORE_EVAS_PATH)/Ecore_Evas.h
	$(CODEGEN_PATH)/h2def.py $(ECORE_EVAS_PATH)/Ecore_Evas.h > $@

clean:
	rm -f *.o *~ ecore_evas.defs ecore_evas.c ecore_evas.so
