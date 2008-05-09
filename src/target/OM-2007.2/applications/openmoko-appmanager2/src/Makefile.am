INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" @OPENMOKO_CFLAGS@ -Wall -std=c99 -pedantic

bin_PROGRAMS = openmoko-appmanager

openmoko_appmanager_SOURCES = appmanager-window.c\
                              navigation-area.c\
                              tool-box.c\
                              detail-area.c\
                              appmanager-data.c\
                              pixbuf-list.c\
                              install-dialog.c \
                              search-bar.c \
                              am-progress-dialog.c \
                              package-store.c 

openmoko_appmanager_LDADD = @OPENMOKO_LIBS@
