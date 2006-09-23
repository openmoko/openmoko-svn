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

