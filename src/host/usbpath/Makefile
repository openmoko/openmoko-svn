CFLAGS=-Wall -g
LDLIBS=-L. -lusbpath -lusb

PREFIX=/usr/local


.PHONY:		all install uninstall clean spotless

all:		usbpath libusbpath.a

usbpath:	libusbpath.a

libusbpath.a:	path2devnum.o devnum2path.o portinfo.o
		$(AR) crv $@ $^

install:	libusbpath.a
		install -m 555 usbpath $(PREFIX)/bin
		install -m 444 libusbpath.a $(PREFIX)/lib

uninstall:
		rm -f $(PREFIX)/bin/usbpath
		rm -f $(PREFIX)/lib/libusbpath.a

clean:
		rm -f *.o

spotless:	clean
		rm -f libusbpath.a usbpath
