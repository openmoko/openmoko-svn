
bin_PROGRAMS = openmoko-mediaplayer

openmoko_mediaplayer_LDADD = \
        $(GTK_LIBS) \
        $(DBUS_LIBS) \
        $(OPENMOKO_LIBS)

openmoko_mediaplayer_CFLAGS = \
        $(GTK_CFLAGS) \
        $(DBUS_CFLAGS) \
        $(OPENMOKO_CFLAGS) \
	$(OMP_DEFINES) \
	-I../intl -I${top_srcdir}/intl -Wall -pedantic -std=c99

openmoko_mediaplayer_CPPFLAGS = \
	-Wall -pedantic

openmoko_mediaplayer_SOURCES = \
        main.c main.h \
	playlist.c playlist.h \
	playback.c playback.h \
	mplayer_playback.c mplayer_playback.h \
	framebuffer.c framebuffer.h \
	guitools.c guitools.h \
	utils.c utils.h \
	persistent.c persistent.h \
	main_page.c main_page.h \
	playlist_page.c playlist_page.h \
	editor_page.c editor_page.h \
	files_page.c files_page.h \
	\
	omp_spiff_c.cpp omp_spiff_c.h \
	\
	mplayer_startup_img.h
