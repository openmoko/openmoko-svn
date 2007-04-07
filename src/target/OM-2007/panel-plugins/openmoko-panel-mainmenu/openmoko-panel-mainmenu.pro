MOKOCONFIG = mokoui
MOKOTYPE = panel-plugin

HEADERS = \
  src/buttonactions.h \
  src/mokodesktop.h \
  src/mokodesktop_item.h \
  src/stylusmenu.h

SOURCES = \
  src/openmoko-panel-mainmenu.c \
  src/buttonactions.c \
  src/mokodesktop.c \
  src/mokodesktop_item.c \
  src/stylusmenu.c

include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )

