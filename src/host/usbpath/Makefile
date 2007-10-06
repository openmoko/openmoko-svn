CFLAGS=-Wall -g
LDLIBS=-L. -lusbpath -lusb

PREFIX=/usr/local


.PHONY:		all install uninstall clean spotless

all:		usbpath libusbpath.a

usbpath:	libusbpath.a

libusbpath.a:	path2devnum.o devnum2path.o portinfo.o
		$(AR) crv $@ $^

libusbpath.pc:	libusbpath.pc.in
		sed 's#PREFIX#$(PREFIX)#' <$^ >$@ || { rm -f $@; exit 1; }

install:	libusbpath.a libusbpath.pc
		install -m 555 usbpath $(PREFIX)/bin
		install -m 444 libusbpath.a $(PREFIX)/lib
		install -m 444 usbpath.h $(PREFIX)/include
		install -D -m 444 libusbpath.pc $(PREFIX)/lib/pkgconfig

uninstall:
		rm -f $(PREFIX)/bin/usbpath
		rm -f $(PREFIX)/lib/libusbpath.a
		rm -f $(PREFIX)/include/usbpath.h
		rm -f $(PREFIX)/lib/pkgconfig/libusbpath.pc

clean:
		rm -f *.o

spotless:	clean
		rm -f libusbpath.a usbpath libusbpath.pc
