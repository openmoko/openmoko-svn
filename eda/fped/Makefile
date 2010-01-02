#
# Makefile - Makefile of fped, the footprint editor
#
# Written 2009, 2010 by Werner Almesberger
# Copyright 2009, 2010 by Werner Almesberger
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#

PREFIX = /usr/local

UPLOAD = werner@sita.openmoko.org:public_html/fped/

OBJS = fped.o expr.o coord.o obj.o delete.o inst.o util.o error.o \
       unparse.o file.o dump.o kicad.o postscript.o meas.o \
       layer.o overlap.o \
       cpp.o lex.yy.o y.tab.o \
       gui.o gui_util.o gui_style.o gui_inst.o gui_status.o gui_canvas.o \
       gui_tool.o gui_over.o gui_meas.o gui_frame.o

XPMS = point.xpm delete.xpm delete_off.xpm \
       vec.xpm frame.xpm frame_locked.xpm frame_ready.xpm \
       line.xpm rect.xpm pad.xpm rpad.xpm arc.xpm circ.xpm \
       meas.xpm meas_x.xpm meas_y.xpm \
       stuff.xpm stuff_off.xpm meas_off.xpm \
       bright.xpm bright_off.xpm all.xpm all_off.xpm

PNGS = intro-1.png intro-2.png intro-3.png intro-4.png intro-5.png \
       intro-6.png concept-inst.png

SHELL = /bin/bash
CFLAGS_GTK = `pkg-config --cflags gtk+-2.0`
LIBS_GTK = `pkg-config --libs gtk+-2.0`

CFLAGS_WARN = -Wall -Wshadow -Wmissing-prototypes \
	      -Wmissing-declarations -Wno-format-zero-length
CFLAGS = -g -std=gnu99 $(CFLAGS_GTK) -DCPP='"cpp"' $(CFLAGS_WARN)
SLOPPY = -Wno-unused -Wno-implicit-function-declaration \
	 -Wno-missing-prototypes -Wno-missing-declarations
LDLIBS = -lm -lfl $(LIBS_GTK)
YACC = bison -y
YYFLAGS = -v

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

.PHONY:		all dep depend clean install uninstall manual upload-manual
.PHONY:		update montage

.SUFFIXES:	.fig .xpm .ppm

# generate 26x26 pixels icons, then drop the 1-pixel frame

.fig.ppm:
		$(GEN) fig2dev -L ppm -Z 0.32 -S 4 $< | \
		  convert -crop 24x24+1+1 - - >$@; \
		  [ "$${PIPESTATUS[*]}" = "0 0" ] || { rm -f $@; exit 1; }

# ppmtoxpm is very chatty, so we suppress its stderr

.ppm.xpm:
		$(GEN) export TMP=_tmp$$$$; ppmcolormask white $< >$$TMP && \
		  ppmtoxpm -name xpm_`basename $@ .xpm` -alphamask $$TMP \
		  $< >$@ 2>/dev/null && rm -f $$TMP || \
		  { rm -f $@ $$TMP; exit 1; }

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

# ----- Upload the GUI manual -------------------------------------------------

manual:		$(XPMS:%=icons/%)
		for n in $(XPMS:%.xpm=%); do \
		    convert icons/$$n.xpm manual/$$n.png || exit 1; done
		fig2dev -L png -S 4 manual/concept-inst.fig \
		    >manual/concept-inst.png

upload-manual:	manual
		scp gui.html README $(UPLOAD)/
		scp $(XPMS:%.xpm=manual/%.png) $(PNGS:%=manual/%) \
		  $(UPLOAD)/manual/

# ----- Debugging help --------------------------------------------------------

montage:
		montage -label %f -frame 3 __dbg????.png png:- | display -

# ----- Dependencies ----------------------------------------------------------

dep depend .depend: lex.yy.c y.tab.h y.tab.c *.h *.c
		$(DEPEND)

-include .depend

# ----- Cleanup ---------------------------------------------------------------

clean:
		rm -f $(OBJS) $(XPMS:%=icons/%) $(XPMS:%.xpm=icons/%.ppm)
		rm -f lex.yy.c y.tab.c y.tab.h y.output .depend
		rm -f __dbg????.png _tmp*

# ----- Install / uninstall ---------------------------------------------------

install:	all
		install -m 755 fped $(PREFIX)/bin/

uninstall:
		rm -f $(PREFIX)/bin/fped

# ----- SVN update ------------------------------------------------------------

update:
		svn update
		$(MAKE) dep all
