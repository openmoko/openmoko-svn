CONFIG = console debug warn_on link_pkgconfig
TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += input.h
SOURCES += input.c

QMAKE_CFLAGS += -std=c99
QMAKE_LINK = $$QMAKE_CC

PKGCONFIG += glib-2.0

# handle pkgconfig for CFLAGS, CXXFLAGS and LIBS already handled by qmake
for(PKGCONFIG_LIB, $$list($$unique(PKGCONFIG))) {
        QMAKE_CFLAGS += $$system(pkg-config --cflags $$PKGCONFIG_LIB)
}

MOC_DIR = ".moc"
OBJECTS_DIR = ".obj"
UI_DIR = ".ui"

