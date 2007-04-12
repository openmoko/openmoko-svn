lib_LTLIBRARIES=libmokogsmd.la

public_headers=moko-gsmd-connection.h
sources=moko-gsmd-connection.c

libmokogsmd_la_SOURCES= $(sources) $(public_headers)
libmokogsmd_la_LIBADD=
libmokogsmd_la_LDFLAGS= -version-info 0:0:0
AM_CFLAGS = -Wall -pedantic -std=c99

libmokogsmdheadersdir=$(includedir)/openmoko-libs/libmokogsmd
libmokogsmdheaders_DATA=$(public_headers)

