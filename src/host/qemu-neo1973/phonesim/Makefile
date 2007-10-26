# Copyright (C) 2007 OpenMoko, Inc.

QMAKE=qmake
all: phonesim
phonesim: Makefile.phonesim lib/libphonesim.a
	$(MAKE) -f Makefile.phonesim
Makefile.phonesim: phonesim.pro
	$(QMAKE) -o $@
lib/libphonesim.a: lib/Makefile
	$(MAKE) -C lib
lib/Makefile: lib/lib.pro
	cd lib; $(QMAKE) 
clean: Makefile.phonesim lib/Makefile
	$(MAKE) -C lib clean
	$(MAKE) -f Makefile.phonesim clean
	rm -f phonesim
