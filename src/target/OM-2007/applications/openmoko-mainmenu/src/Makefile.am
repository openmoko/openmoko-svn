INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" @OPENMOKO_CFLAGS@ -Wall -std=c99 -pedantic

bin_PROGRAMS = openmoko-mainmenu

openmoko_mainmenu_SOURCES = main.c mainmenu.c callbacks.c menu-list.c support.c 

openmoko_mainmenu_LDADD = @OPENMOKO_LIBS@

