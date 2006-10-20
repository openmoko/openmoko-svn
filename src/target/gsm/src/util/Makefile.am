INCLUDES = $(all_includes) -I$(top_srcdir)/include
AM_CFLAGS = -std=gnu99

bin_PROGRAMS = libgsmd-tool

libgsmd_tool_SOURCES = libgsmd-tool.c
libgsmd_tool_LDADD = ../libgsmd/libgsmd.la
libgsmd_tool_LDFLAGS = -dynamic

