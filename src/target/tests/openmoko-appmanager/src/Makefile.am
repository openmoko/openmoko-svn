## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES = \
	$(GTK_CFLAGS)

AM_CFLAGS =\
	$(LIBIPKG_CFLAGS) -DIPKG_LIB\
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
	errorcode.h\
	ipkgapi.c\
	ipkgapi.h\
	ipkg_cmd.h\
	widgets.c\
	widgets.h\
	iconmgr.c\
	iconmgr.h\
	pkglist.c\
	pkglist.h\
	menumgr.c\
	menumgr.h\
	treeview.c\
	treeview.h\
	alldialog.c\
	alldialog.h\
	initwidgets.c\
	initwidgets.h

appmgr_LDFLAGS = \
	$(LIBIPKG_LIBS)

appmgr_LDADD =  \
	$(GTK_LIBS)\
	-lpthread

