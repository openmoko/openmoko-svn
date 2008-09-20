EDJE_PATH=$(E_PATH)/edje/src/lib

CFLAGS = -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include/ -I/usr/include/python2.5 -I/usr/include/pygtk-2.0/ -I$(E_PATH)/evas/src/lib/ -I$(EDJE_PATH) -I.

.PHONY: all clean edje.so

all: edje.so

edje.so: edje.o edjemodule.o                             
	$(CC) $(LDFLAGS) -shared $^ -o $@

edje.c: edje.defs edje.override
	pygtk-codegen-2.0 --prefix edje \
	--override edje.override \
	edje.defs > $@
	
edje.defs: $(H2DEF_PATH) $(EDJE_PATH)/Edje.h
	$(H2DEF_PATH) $(EDJE_PATH)/Edje.h > edje.defs 

clean:
	rm -f *.o edje.defs edje.c
