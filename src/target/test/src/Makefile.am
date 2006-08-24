## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES = \
	$(GTK_CFLAGS) \
	$(LIBIPKG_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = appmgr

appmgr_SOURCES = \
	support.h\
	support.c\
	main.c\
	interface.h\
	interface.c\
	callbacks.h\
	callbacks.c\
	errorcode.c\
	errorcode.h\
	winresize.c\
	winresize.h\
	ipkgapi.c\
	ipkgapi.h\
	pkgmgr.c\
	pkgmgr.h\
	detareamgr.c\
	detareamgr.h

appmgr_LDFLAGS = 

appmgr_LDADD =  \
	$(GTK_LIBS) \
	$(LIBIPKG_LIBS)

