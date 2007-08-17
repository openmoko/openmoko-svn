INCLUDES = $(all_includes) -I$(top_srcdir)/include
AM_CFLAGS = -std=gnu99

bin_PROGRAMS = libgsmd-tool

libgsmd_tool_SOURCES = libgsmd-tool.c shell.c event.c pin.c atcmd.c \
		       $(top_srcdir)/src/gsmd/talloc.c
libgsmd_tool_LDADD = $(top_builddir)/src/libgsmd/libgsmd.la
libgsmd_tool_LDFLAGS = -dynamic

noinst_HEADERS = atcmd.h event.h pin.h shell.h
