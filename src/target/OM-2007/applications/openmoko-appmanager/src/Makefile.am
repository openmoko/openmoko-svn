INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" @OPENMOKO_CFLAGS@ -Wall -std=c99 -pedantic

bin_PROGRAMS = openmoko-appmanager

openmoko_appmanager_SOURCES = appmanager-window.c\
                              application-menu.c\
                              filter-menu.c\
                              ipkgapi.c\
                              navigation-area.c\
                              tool-box.c\
                              detail-area.c\
                              appmanager-data.c\
                              package-list.c\
                              pixbuf-list.c\
                              select-menu.c\
                              apply-dialog.c

openmoko_appmanager_LDADD = @OPENMOKO_LIBS@
