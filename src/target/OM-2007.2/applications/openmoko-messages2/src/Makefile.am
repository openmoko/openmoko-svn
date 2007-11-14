
INCLUDES = -I$(top_srcdir)/libhito
AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" $(GTK_CFLAGS) $(JANA_CFLAGS) $(MOKOUI_CFLAGS) $(EBOOK_CFLAGS) -Wall
AM_LDFLAGS = $(GTK_LIBS) $(JANA_LIBS) $(MOKOUI_LIBS) $(EBOOK_LIBS)

bin_PROGRAMS=openmoko-messages

openmoko_messages_SOURCES = \
	sms.h \
	sms-main.c \
	sms-contacts.c \
	sms-contacts.h \
	sms-notes.c \
	sms-notes.h

openmoko_messages_LDADD = $(top_srcdir)/libhito/libhito.a

