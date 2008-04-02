INCLUDES = -I$(top_srcdir)/libhito

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" $(GTK_CFLAGS) $(JANA_CFLAGS) $(MOKOUI_CFLAGS) $(EBOOK_CFLAGS) $(DBUS_CFLAGS) -Wall
AM_LDFLAGS = $(GTK_LIBS) $(JANA_LIBS) $(MOKOUI_LIBS) $(EBOOK_LIBS) $(DBUS_LIBS) $(top_builddir)/libhito/libhito.a

bin_PROGRAMS=openmoko-messages

noinst_PROGRAMS=test-notes

test_notes_SOURCES = \
	test-notes.c

openmoko_messages_SOURCES = \
	sms.h \
	sms-main.c \
	sms-contacts.c \
	sms-contacts.h \
	sms-notes.c \
	sms-notes.h \
	sms-compose.c \
	sms-compose.h \
	sms-utils.c \
	sms-utils.h \
	sms-dbus.c \
	sms-dbus.h \
	moko-save-number.c \
	moko-save-number.h

%-glue.h: %.xml
	$(LIBTOOL) --mode=execute $(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=$(subst -,_,$*) --mode=glib-server --output=$@ $<
        
BUILT_SOURCES = sms-dbus-glue.h

EXTRA_DIST = sms-dbus.xml

