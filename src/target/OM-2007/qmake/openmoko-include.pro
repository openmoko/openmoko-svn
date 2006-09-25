CONFIG = debug warn_on link_pkgconfig console

# handle pkg-config files (from qt4)
for(PKGCONFIG_LIB, $$list($$unique(PKGCONFIG))) {
	QMAKE_CXXFLAGS += $$system(pkg-config --cflags $$PKGCONFIG_LIB)
	QMAKE_CFLAGS += $$system(pkg-config --cflags $$PKGCONFIG_LIB)
	LIBS += $$system(pkg-config --libs $$PKGCONFIG_LIB)
}

QMAKE_CFLAGS += -std=c99 -pedantic

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
