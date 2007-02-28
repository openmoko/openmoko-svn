AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\"
AM_CFLAGS = -Wall -std=c99 @OPENMOKO_CFLAGS@ @LIBMB_CFLAGS@

bin_PROGRAMS = openmoko-mainmenu

openmoko_mainmenu_SOURCES = main.c \
			   mainmenu.c mokoiconview.c\
			   mokodesktop.c mokodesktop_item.c\
			   callbacks.c app-history.c dbus-conn.c


openmoko_mainmenu_LDADD = @OPENMOKO_LIBS@ @LIBMB_LIBS@

