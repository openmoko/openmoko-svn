AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
              -DPKGDATADIR=\"$(pkgdatadir)\"

INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" @OPENMOKO_CFLAGS@ -Wall -std=c99 -pedantic

bin_PROGRAMS = openmoko-messages

openmoko_messages_SOURCES = main.c main.h \
                            foldersdb.c foldersdb.h \
			    callbacks.c callbacks.h \
			    detail-area.c detail-area.h \
			    sms-dialog-window.c sms-dialog-window.h \
			    sms-membership-window.c sms-membership-window.h \
			    sms-contact-window.c sms-contact-window.h

openmoko_messages_LDADD = @OPENMOKO_LIBS@

