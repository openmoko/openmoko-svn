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
  data/gtkrc \
  data/matchboxpanel \
  data/openmoko-calculator \
  data/openmoko-contacts \
  data/openmoko-dialer \
  data/openmoko-panel-plugins
