MOKOCONFIG = mokoui gsm

HEADERS = src/moko-digit-button.h src/moko-dialer-panel.h src/openmoko-dialer-main.h
SOURCES = src/moko-digit-button.c src/moko-dialer-panel.c src/openmoko-dialer-main.c

include ( $(OPENMOKODIR)/devel/qmake/openmoko-include.pro )
