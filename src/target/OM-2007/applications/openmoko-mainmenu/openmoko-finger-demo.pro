MOKOCONFIG = mokoui

HEADERS = src/main.h mainmenu.h callbacks.h menu-list.h support.h
SOURCES = src/main.c mainmenu.c callbacks.c menu-list.c support.c

include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )
