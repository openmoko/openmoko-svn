HEADERS = src/demo-panel-applet.h
SOURCES = src/demo-panel-applet.c src/main.c

MOKOCONFIG = mokoui
MOKOTYPE = panel-plugin
include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )
