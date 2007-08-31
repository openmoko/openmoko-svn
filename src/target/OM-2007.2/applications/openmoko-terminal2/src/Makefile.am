INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
  -DTERMINAL_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
  -DDATADIR=\""$(datadir)"\" -D_GNU_SOURCE

AM_CFLAGS = -Wall -pedantic -std=c99 @MOKOUI_CFLAGS@

bin_PROGRAMS = openmoko-terminal

openmoko_terminal_SOURCES =	\
  main.c \
  moko-terminal.c

openmoko_terminal_LDADD  = @MOKOUI_LIBS@

MAINTAINERCLEANFILES  = config.h.in Makefile.in

