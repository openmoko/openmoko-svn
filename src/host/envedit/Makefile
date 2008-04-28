PREFIX=/usr

CFLAGS_WARN=-Wall -Wshadow -Wstrict-prototypes -Wmissing-prototypes \
       -Wmissing-declarations
CFLAGS=-g -O $(CFLAGS_WARN) -DCPP='"$(PREFIX)/bin/envcpp.pl"'
LDFLAGS=-lz

OBJS=envedit.o env.o parse.o var.o cpp.o

.PHONY:		all install uninstall dep depend clean spotless

all:		envedit

envedit:	$(OBJS)

install:	envedit
		install -D envedit $(PREFIX)/bin/envedit
		install -D envcpp.pl $(PREFIX)/bin/envcpp.pl

uninstall:
		rm -f $(PREFIX)/bin/{envedit,envcpp.pl}

dep depend .depend:
		$(CPP) $(CFLAGS) -MM -MG *.c >.depend || \
		  { rm -f .depend; exit 1; }

clean:
		rm -f $(OBJS)

spotless:	clean
		rm -f envedit
