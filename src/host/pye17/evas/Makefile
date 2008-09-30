CFLAGS = `pkg-config --cflags evas glib-2.0 pygtk-2.0` `python-config --includes`
LDFLAGS = `pkg-config --libs evas glib-2.0 pygtk-2.0` `python-config --libs`

EVAS_PATH=$(E_PATH)/evas/src/lib

.PHONY: all clean evas.so

all: evas.so

evas.so: evas.o evasmodule.o
	$(CC) $(LDFLAGS) -shared $^ -o $@

evas.c: evas.defs evas.override
	$(PYTHON) $(CODEGEN_PATH)/codegen.py --prefix evas --override evas.override evas.defs > $@

evas.defs: $(CODEGEN_PATH)/h2def.py $(EVAS_PATH)/Evas.h $(EVAS_PATH)/Evas_Data.h
	$(CODEGEN_PATH)/h2def.py $(EVAS_PATH)/Evas.h > $@
	$(CODEGEN_PATH)/h2def.py $(EVAS_PATH)/Evas_Data.h >> $@

clean:
	rm -f *.o *~ evas.defs evas.c evas.so


### may need later ??
# 	--override evas.override --register evas_data.defs \
#
# evas_data.defs: $(CODEGEN_PATH)/h2def.py $(EVAS_PATH)/Evas.h $(EVAS_PATH)/Evas_Data.h
# 	$(CODEGEN_PATH)/h2def.py $(EVAS_PATH)/Evas_Data.h > $@
