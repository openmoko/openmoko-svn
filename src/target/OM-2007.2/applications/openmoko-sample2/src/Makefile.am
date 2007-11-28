INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS = \
  -DPKGDATADIR=\"$(pkgdatadir)\" \
  -DTERMINAL_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
  -DDATADIR=\""$(datadir)"\" -D_GNU_SOURCE

AM_CFLAGS = -Wall -pedantic -std=c99 @DEPENDENCIES_CFLAGS@

# add the name of your application
bin_PROGRAMS = openmoko-sample

# add the sources to compile for your application
openmoko_sample_SOURCES = \
  sample-main.c

openmoko_sample_LDADD  = @DEPENDENCIES_LIBS@

MAINTAINERCLEANFILES  = config.h.in Makefile.in

