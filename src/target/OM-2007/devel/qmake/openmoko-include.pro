#
# Don't use this if your nickname is not mickeyl
#

CONFIG = debug warn_on link_pkgconfig console $$MOKOCONFIG
PREFIX = /usr/local

INCLUDEPATH += $(ADD_INCLUDE)
LIBS += $(ADD_LIB)

QMAKE_LINK              = gcc
QMAKE_LINK_SHLIB        = gcc
QMAKE_RPATH             =
QMAKE_CFLAGS           += -std=c99 -pedantic
INCLUDEPATH            += $(OPENMOKODIR)

PLATFORM = $$system(uname -m)

MOC_DIR=.moc/$$PLATFORM
OBJECTS_DIR=.obj/$$PLATFORM

#
# integrate glib-genmarshal as additional compiler
#
QMAKE_GENMARSHAL_CC  = glib-genmarshal
glib-genmarshal.commands = $${QMAKE_GENMARSHAL_CC} --prefix=$${GENMARSHALS_PREFIX} ${QMAKE_FILE_IN} --header --body >${QMAKE_FILE_OUT}
glib-genmarshal.output = $$OUT_PWD/${QMAKE_FILE_BASE}.h
glib-genmarshal.input = GENMARSHALS
glib-genmarshal.CONFIG = no_link
glib-genmarshal.variable_out = PRE_TARGETDEPS
glib-genmarshal.name = GENMARSHALS
QMAKE_EXTRA_UNIX_COMPILERS += glib-genmarshal

#
# integrate dbus-binding-tool as additional compiler
#
QMAKE_DBUS_CC  = dbus-binding-tool
dbus-binding-tool.commands = $${QMAKE_DBUS_CC} --mode=glib-server ${QMAKE_FILE_IN} >${QMAKE_FILE_OUT}
dbus-binding-tool.output = $$OUT_PWD/${QMAKE_FILE_BASE}.h
dbus-binding-tool.input = SERVICES
dbus-binding-tool.CONFIG = no_link
dbus-binding-tool.variable_out = HEADERS
dbus-binding-tool = SERVICES
QMAKE_EXTRA_UNIX_COMPILERS += dbus-binding-tool

mokocore {
	INCLUDEPATH += $(OPENMOKODIR)/openmoko-libs
	LIBS += -lmokocore -L$(OPENMOKODIR)/lib
}

mokogsmd {
	INCLUDEPATH += $(OPENMOKODIR)/openmoko-libs
	PKGCONFIG += libgsmd
	LIBS += -lmokogsmd -L$(OPENMOKODIR)/lib
}

mokoui {
	INCLUDEPATH += $(OPENMOKODIR)/openmoko-libs
	PKGCONFIG += gtk+-2.0
	LIBS += -lmokoui -L$(OPENMOKODIR)/lib
}

mokopim {
	INCLUDEPATH += $(OPENMOKODIR)/openmoko-libs
	LIBS += -lmokopim -L$(OPENMOKODIR)/lib
}

mokonet {
	INCLUDEPATH += $(OPENMOKODIR)/openmoko-libs
	LIBS += -lmokonet -L$(OPENMOKODIR)/lib
}

mokojournal {
	INCLUDEPATH += $(OPENMOKODIR)/openmoko-libs
	LIBS += -lmokojournal -L$(OPENMOKODIR)/lib
}

# handle pkgconfig for CFLAGS, CXXFLAGS and LIBS already handled by qmake
for(PKGCONFIG_LIB, $$list($$unique(PKGCONFIG))) {
#        QMAKE_CXXFLAGS += $$system(pkg-config --cflags $$PKGCONFIG_LIB)
        QMAKE_CFLAGS += $$system(pkg-config --cflags $$PKGCONFIG_LIB)
#        LIBS += $$system(pkg-config --libs $$PKGCONFIG_LIB)
}

contains ( MOKOTYPE, panel-plugin ) {
    DATADIR = $(OPENMOKODIR)/panel-plugins/$$TARGET/data
    DESTDIR = $$PREFIX/matchbox-panel
    TEMPLATE = lib
}
!contains ( MOKOTYPE, panel-plugin ) {
    DATADIR = $(OPENMOKODIR)/applications/$$TARGET/data
}
contains ( TEMPLATE, lib ) {
    DATADIR = $(OPENMOKODIR)/openmoko-libs/data
}

contains( CONFIG, debug ) {
	!contains( TEMPLATE, lib ) {
		DEFINES += PKGDATADIR=\\\"$$DATADIR/\\\"
		DEFINES += DATADIR=\\\"$$DATADIR/\\\"
		system( ln -sf . $$DATADIR/icons )
	}
    contains( TEMPLATE, lib ) {
		DEFINES += PKGDATADIR=\\\"$$APPDIR/\\\"
		DEFINES += DATADIR=\\\"$$APPDIR/\\\"
	}
}
!contains( CONFIG, debug ) {
    APPDIR = /usr/share/$$TARGET
    DEFINES += PKGDATADIR=\\\"$$APPDIR/\\\"
}

contains( TEMPLATE, app ) {
    message( configuring application $$TARGET )
    DESTDIR = $(OPENMOKODIR)/bin
}

contains( TEMPLATE, lib ) {
    message( configuring library $$TARGET )
    DESTDIR = $(OPENMOKODIR)/lib
}


DEFINES += G_LOG_DOMAIN=\\\"$$TARGET\\\"
DEFINES += GETTEXT_PACKAGE=\\\"$$TARGET\\\"
DEFINES += LC_ALL=\\\"C\\\"
DEFINES += LOCALEDIR=\\\"/tmp/$$TARGET\\\"

