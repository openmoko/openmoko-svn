SUBDIRS = src layouts

if WANT_EXAMPLES
SUBDIRS += examples
endif

EXTRA_DIST = openmoko-keyboard.desktop openmoko-keyboard.png

desktopdir = $(datadir)/applications/inputmethods
desktop_DATA = openmoko-keyboard.desktop

pixmapsdir = $(datadir)/pixmaps/
pixmaps_DATA = openmoko-keyboard.png


snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-snap`date +"%Y%m%d"`
