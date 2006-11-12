MOKOCONFIG = mokoui

HEADERS = src/callbacks.h src/chordsdb.h src/fretboard-widget.h src/main.h
SOURCES = src/callbacks.c src/chordsdb.c src/fretboard-widget.c src/main.c

include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )
