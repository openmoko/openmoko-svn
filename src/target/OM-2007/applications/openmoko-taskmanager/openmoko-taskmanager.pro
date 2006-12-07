MOKOCONFIG = mokoui

HEADERS = src/taskmanager.h list_view.h callbacks.h \
		popupmenu.h misc.h xatoms.h
SOURCES = src/taskmanager.c list_view.c callbacks.c \
		popupmenu.c misc.c xatoms.c

include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )
