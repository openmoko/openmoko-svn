DEPENDPATH += . src
INCLUDEPATH += . src

# Input
HEADERS += src/app-history.h \
           src/callbacks.h \
           src/main.h \
           src/mainmenu.h \
           src/mokoiconview.h \
	   src/close-page.h \
	   src/mokodesktop_item.h \
	   src/mokodesktop.h
	   
SOURCES += src/app-history.c \
           src/callbacks.c \
           src/main.c \
           src/mainmenu.c \
           src/mokoiconview.c \
	   src/close-page.c \
	   src/mokodesktop_item.c \
	   src/mokodesktop.c

MOKOCONFIG = mokoui
include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )

