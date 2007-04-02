localedir = $(datadir)/locale

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" -DLOCALEDIR=\"$(localedir)\"
AM_CFLAGS =  @OPENMOKO_CFLAGS@ -Wall -std=c99 -pedantic

bin_PROGRAMS = openmoko-calculator

openmoko_calculator_SOURCES = calc-main.c calc-main.h

