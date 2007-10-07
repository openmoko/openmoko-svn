AM_CFLAGS = -Wall

lib_LIBRARIES = libusbpath.a
bin_PROGRAMS = usbpath
include_HEADERS = usbpath.h

libusbpath_a_SOURCES = path2devnum.c devnum2path.c portinfo.c usbpath.h
usbpath_SOURCES = usbpath.c usbpath.h
usbpath_LDADD = libusbpath.a
