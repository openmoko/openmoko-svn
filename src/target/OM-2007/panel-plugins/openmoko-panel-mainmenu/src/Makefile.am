INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" @OPENMOKO_CFLAGS@ -Wall -std=c99 -pedantic

bin_PROGRAMS = openmoko-panel-mainmenu

openmoko_panel_mainmenu_SOURCES = moko-panel-mainmenu.c\
				  moko-panel-mainmenu.h\
				  main.c

openmoko_panel_mainmenu_LDADD = @OPENMOKO_LIBS@

