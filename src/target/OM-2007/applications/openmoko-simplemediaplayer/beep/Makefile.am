SUBDIRS = 

beepincludedir = $(includedir)/bmp

bin_PROGRAMS = beep
transform = s/beep/beep-media-player/

beep_LDFLAGS = -export-dynamic
beep_LDADD = -lid3 \
	$(X_LIBS) $(X_PRE_LIBS) -lX11 $(X_EXTRA_LIBS) \
	$(GTK_LIBS)      \
	$(DBUS_LIBS)      \
	$(OPENMOKO_LIBS)      \
	$(LIBGLADE_LIBS) \
	$(LTLIBINTL)     \
	$(top_builddir)/libbeep/libbeep.la

beep_CFLAGS = \
	$(X_CFLAGS)        \
	$(GTK_CFLAGS)      \
	$(DBUS_CFLAGS)      \
	$(OPENMOKO_CFLAGS)      \
	$(LIBGLADE_CFLAGS) \
	$(BEEP_DEFINES)    \
	$(ARCH_DEFINES)    \
	-I$(top_srcdir)    \
	-I$(top_srcdir)/intl

beep_SOURCES = \
	skin.c skin.h \
	util.c util.h \
	output.c output.h \
	fft.c fft.h \
	input.c input.h \
	effect.c effect.h \
	general.c general.h \
	visualization.c visualization.h \
	fullscreen.c fullscreen.h \
	pluginenum.c pluginenum.h \
	playlist.c playlist.h \
	controlsocket.c controlsocket.h \
	dock.c dock.h \
	widget.c widget.h \
	sbutton.c sbutton.h \
	pbutton.c pbutton.h \
	tbutton.c tbutton.h \
	textbox.c textbox.h \
	menurow.c menurow.h \
	hslider.c hslider.h \
	monostereo.c monostereo.h \
	vis.c vis.h \
	svis.c svis.h \
	number.c number.h \
	playback.c playback.h \
	playstatus.c playstatus.h \
	playlist_list.c playlist_list.h \
	playlist_slider.c playlist_slider.h \
	playlist_popup.c playlist_popup.h \
	eq_graph.c eq_graph.h \
	eq_slider.c eq_slider.h \
	main.c main.h \
	logger.c logger.h \
	xml_document.c xml_document.h \
	mainwin.c mainwin.h \
	skinwin.c skinwin.h \
	prefswin.c prefswin.h \
	playlistwin.c playlistwin.h \
	equalizer.c equalizer.h \
	glade.c glade.h \
	hints.c hints.h \
	about.c about.h \
	plugin.h \
	softvolume.c softvolume.h \
	getopt.c getopt1.c getopt.h \
	urldecode.c urldecode.h \
	dnd.h \
	debug.h \
	mkdtemp.c \
	pixmaps.h \
	iir.c iir.h 

beepinclude_HEADERS = plugin.h fullscreen.h

desktop_DATA = bmp.desktop
desktopdir = $(datadir)/applications


BUILT_SOURCES = pixmaps.h

embedded_icons = 

EXTRA_DIST = \
	$(embedded_icons) build.list

pixmaps.h: $(embedded_icons)
	gdk-pixbuf-csource --stream --build-list `cat build.list` > pixmaps.h
