MOKOCONFIG = mokoui

HEADERS = src/main.h src/mainmenu.h src/callbacks.h src/menu-list.h src/support.h
SOURCES = src/main.c src/mainmenu.c src/callbacks.c src/menu-list.c src/support.c

include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )
