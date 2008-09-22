
EVAS_PATH=$(E_PATH)/evas/src/lib
EDJE_PATH=$(E_PATH)/edje/src/lib

CFLAGS = -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include/ -I/usr/include/python2.5 -I/usr/include/pygtk-2.0/ -I$(EVAS_PATH) -I$(EDJE_PATH) -I.

.PHONY: all clean edje.so

all: edje.so

edje.so: edje.o edjemodule.o
	$(CC) $(LDFLAGS) -shared $^ -o $@

edje.c: edje.defs edje.override
	$(PYTHON) $(CODEGEN_PATH)/codegen.py --prefix edje --override edje.override --register ../evas/evas.defs edje.defs > $@

edje.defs: $(CODEGEN_PATH)/h2def.py $(EDJE_PATH)/Edje.h
	$(CODEGEN_PATH)/h2def.py $(EDJE_PATH)/Edje.h > edje.defs

clean:
	rm -f *.o edje.defs edje.c
