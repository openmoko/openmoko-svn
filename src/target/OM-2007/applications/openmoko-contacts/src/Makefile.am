INCLUDES = 						\
	-I$(top_srcdir)

AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\"				\
	-DCONTACTS_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"  	\
	-DDATADIR=\""$(datadir)"\"

AM_CFLAGS = -Wall -export-dynamic $(CONTACTS_CFLAGS)

bin_PROGRAMS = contacts

contacts_SOURCES =  bacon-message-connection.c \
                    bacon-message-connection.h \
                    contacts-callbacks-ebook.c \
                    contacts-callbacks-ebook.h \
                    contacts-callbacks-ui.c \
                    contacts-callbacks-ui.h \
                    contacts-defs.h \
                    contacts-contact-pane.c \
                    contacts-contact-pane.h \
                    contacts-main.c \
                    contacts-main.h \
                    contacts-utils.c \
                    contacts-utils.h \
                    contacts-ui.c \
                    contacts-ui.h \
                    contacts-omoko.c \
                    contacts-omoko.h \
                    contacts-groups-editor.c \
                    contacts-groups-editor.h

contacts_LDADD   = $(CONTACTS_LIBS)
contacts_LDFLAGS = @CONTACTS_LIBS@

MAINTAINERCLEANFILES = config.h.in Makefile.in
