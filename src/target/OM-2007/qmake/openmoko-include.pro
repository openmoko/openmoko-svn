CONFIG = debug warn_on link_pkgconfig console $$MOKOCONFIG

QMAKE_CFLAGS += -std=c99 -pedantic
INCLUDEPATH += $(OPENMOKODIR)

MOC_DIR=.moc/$(PLATFORM)
OBJECTS_DIR=.obj/$(PLATFORM)

QMAKE_DBUS_CC  = dbus-binding-tool
dbus-binding-tool.commands = $${QMAKE_DBUS_CC} --mode=glib-server ${QMAKE_FILE_IN} >${QMAKE_FILE_OUT}
dbus-binding-tool.output = $$OUT_PWD/${QMAKE_FILE_BASE}.h
dbus-binding-tool.input = SERVICES
dbus-binding-tool.CONFIG = no_link
dbus-binding-tool.variable_out = HEADERS
dbus-binding-tool = SERVICES
QMAKE_EXTRA_UNIX_COMPILERS += dbus-binding-tool

mokocore {
	INCLUDEPATH += ${OPENMOKODIR}/libraries
	LIBS += -lmokocore -L${OPENMOKODIR}/lib
}

mokoui {
	INCLUDEPATH += ${OPENMOKODIR}/libraries
	PKGCONFIG += gtk+-2.0
	LIBS += -lmokoui -L${OPENMOKODIR}/lib
}

mokopim {
	INCLUDEPATH += ${OPENMOKODIR}/libraries
	LIBS += -lmokopim -L${OPENMOKODIR}/lib
}

mokonet {
	INCLUDEPATH += ${OPENMOKODIR}/libraries
	LIBS += -lmokonet -L${OPENMOKODIR}/lib
}

# handle pkg-config files (from qt4)
for(PKGCONFIG_LIB, $$list($$unique(PKGCONFIG))) {
        QMAKE_CXXFLAGS += $$system(pkg-config --cflags $$PKGCONFIG_LIB)
        QMAKE_CFLAGS += $$system(pkg-config --cflags $$PKGCONFIG_LIB)
        LIBS += $$system(pkg-config --libs $$PKGCONFIG_LIB)
}

contains( TEMPLATE, app ) {
	message( configuring application $$TARGET )
	DESTDIR = ${OPENMOKODIR}/bin
}

contains( TEMPLATE, lib ) {
	message( configuring library $$TARGET )
	DESTDIR = ${OPENMOKODIR}/lib
}

