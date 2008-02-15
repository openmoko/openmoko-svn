CFLAGS=-Wall -I/usr/local/include
LDFLAGS=-L/usr/local/lib -lftdi

.PHONY:		all install uninstall clean spotless

all:		norwp

install:
		install -D norwp $(PREFIX)/bin/norwp

uninstall:
		rm -f $(PREFIX)/bin/norwp

clean:
		rm -f norwp.o

spotless:	clean
		rm -f norwp
