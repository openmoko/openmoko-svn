INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" @OPENMOKO_CFLAGS@ -Wall -std=c99 -pedantic

bin_PROGRAMS = openmoko-appmanager

openmoko_appmanager_SOURCES = appmanager-window.c\
                              ipkgapi.c\
                              navigation-area.c\
                              tool-box.c\
                              detail-area.c\
                              appmanager-data.c\
                              package-list.c\
                              pixbuf-list.c\
                              apply-dialog.c\
                              install-dialog.c \
                              search-bar.c \
                              ipkg-utils.c \
                              am-progress-dialog.c

openmoko_appmanager_LDADD = @OPENMOKO_LIBS@
