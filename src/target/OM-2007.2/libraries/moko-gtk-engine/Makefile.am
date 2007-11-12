INCLUDES = $(GTK_CFLAGS) $(DEVELOPMENT_CFLAGS)

enginedir = $(libdir)/gtk-2.0/$(GTK_VERSION)/engines

engine_LTLIBRARIES = libmoko-engine.la

libmoko_engine_la_SOURCES = \
	./src/moko-draw.c             \
	./src/moko-draw.h             \
	./src/moko-main.c             \
	./src/moko-style.c            \
	./src/moko-style.h \
	./src/moko-utils.c \
	./src/moko-utils.h

libmoko_engine_la_LDFLAGS = -module -avoid-version -no-undefined
libmoko_engine_la_LIBADD = $(GTK_LIBS)

gtkthemedir = $(datadir)/themes/Moko/gtk-2.0
dist_gtktheme_DATA = \
  data/gtk-2.0/gtkrc \
  data/gtk-2.0/matchboxpanel \
  data/gtk-2.0/openmoko-calculator \
  data/gtk-2.0/openmoko-contacts \
  data/gtk-2.0/openmoko-dialer \
  data/gtk-2.0/openmoko-panel-plugins

matchboxthemedir = $(datadir)/themes/Moko/matchbox
dist_matchboxtheme_DATA = \
  data/matchbox/closebutton.xpm \
  data/matchbox/menubutton.xpm \
  data/matchbox/theme.desktop \
  data/matchbox/theme.xml

