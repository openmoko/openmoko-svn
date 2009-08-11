#
# Makefile - Makefile of fped, the footprint editor
#
# Written 2009 by Werner Almesberger
# Copyright 2009 by Werner Almesberger
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#

OBJS = fped.o expr.o coord.o obj.o delete.o inst.o util.o error.o \
       unparse.o dump.o kicad.o postscript.o meas.o \
       cpp.o lex.yy.o y.tab.o \
       gui.o gui_util.o gui_style.o gui_inst.o gui_status.o gui_canvas.o \
       gui_tool.o gui_over.o gui_meas.o gui_frame.o

XPMS = point.xpm delete.xpm vec.xpm frame.xpm frame_locked.xpm frame_ready.xpm \
       line.xpm rect.xpm pad.xpm circ.xpm meas.xpm meas_x.xpm meas_y.xpm \
       stuff.xpm stuff_off.xpm meas_off.xpm

CFLAGS_GTK = `pkg-config --cflags gtk+-2.0`
LIBS_GTK = `pkg-config --libs gtk+-2.0`

CFLAGS_WARN=-Wall -Wshadow -Wmissing-prototypes \
            -Wmissing-declarations
CFLAGS=-g $(CFLAGS_GTK) -DCPP='"cpp"' $(CFLAGS_WARN)
SLOPPY=-Wno-unused -Wno-implicit-function-declaration -Wno-missing-prototypes \
       -Wno-missing-declarations
LDLIBS = -lm -lfl $(LIBS_GTK)
YACC=bison -y
YYFLAGS=-v

# ----- Verbosity control -----------------------------------------------------

CPP := $(CPP)   # make sure changing CC won't affect CPP

CC_normal	:= $(CC)
YACC_normal	:= $(YACC)
LEX_normal	:= $(LEX)
DEPEND_normal = \
  $(CPP) $(CFLAGS) -MM -MG *.c >.depend || \
  { rm -f .depend; exit 1; }

CC_quiet	= @echo "  CC       " $@ && $(CC_normal)
YACC_quiet	= @echo "  YACC     " $@ && $(YACC_normal)
LEX_quiet	= @echo "  LEX      " $@ && $(LEX_normal)
GEN_quiet	= @echo "  GENERATE " $@ &&
DEPEND_quiet	= @echo "  DEPENDENCIES" && $(DEPEND_normal)

ifeq ($(V),1)
    CC		= $(CC_normal)
    LEX		= $(LEX_normal)
    YACC	= $(YACC_normal)
    GEN		=
    DEPEND	= $(DEPEND_normal)
else
    CC		= $(CC_quiet)
    LEX		= $(LEX_quiet)
    YACC	= $(YACC_quiet)
    GEN		= $(GEN_quiet)
    DEPEND	= $(DEPEND_quiet)
endif

# ----- Rules -----------------------------------------------------------------

.PHONY:		all dep depend clean

.SUFFIXES:	.fig .xpm

# generate 26x26 pixels icons, then drop the 1-pixel frame

# this adds a magenta border
# sed '/2 2 0 1 /{s//2 2 0 15 /;s/ 0 7 / 22 7 /;}' $< | \

.fig.xpm:
		$(GEN) fig2dev -L xpm -Z 0.32 -S 4 $< | \
		  convert -crop 24x24+1+1 - - | \
		  sed "s/*.*\[]/*xpm_`basename $@ .xpm`[]/" >$@

all:		fped

fped:		$(OBJS)
		$(CC) -o $@ $(OBJS) $(LDLIBS)

lex.yy.c:	fpd.l y.tab.h
		$(LEX) fpd.l

lex.yy.o:	lex.yy.c y.tab.h
		$(CC) -c $(CFLAGS) $(SLOPPY) lex.yy.c

y.tab.c y.tab.h: fpd.y
		$(YACC) $(YYFLAGS) -d fpd.y

y.tab.o:	y.tab.c
		$(CC) -c $(CFLAGS) $(SLOPPY) y.tab.c

gui_tool.o gui.o: $(XPMS:%=icons/%)

# ----- Dependencies ----------------------------------------------------------

dep depend .depend: lex.yy.c y.tab.h y.tab.c
		$(DEPEND)

ifeq (.depend,$(wildcard .depend))
include .depend
endif

# ----- Cleanup ---------------------------------------------------------------

      clean:
		rm -f $(OBJS) $(XPMS:%=icons/%)
		rm -f lex.yy.c y.tab.c y.tab.h y.output .depend
