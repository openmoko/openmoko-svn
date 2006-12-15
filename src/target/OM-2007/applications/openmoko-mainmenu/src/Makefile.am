AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CFLAGS = -Wall -std=c99 -pedantic @OPENMOKO_CFLAGS@

bin_PROGRAMS = openmoko-mainmenu

openmoko_mainmenu_SOURCES = main.c mainmenu.c callbacks.c menu-list.c support.c mokoiconview.c app-history.c

openmoko_mainmenu_LDADD = @OPENMOKO_LIBS@

