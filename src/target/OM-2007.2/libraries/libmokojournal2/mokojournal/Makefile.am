lib_LTLIBRARIES=libmokojournal2.la

public_headers=moko-journal.h moko-time.h
sources=moko-journal.c moko-time.c

libmokojournal2_la_SOURCES= $(sources) $(public_headers)
libmokojournal2_la_LIBADD=
libmokojournal2_la_LDFLAGS= -version-info 0:1:0
AM_CFLAGS = -Wall -pedantic -std=c99

libmokojournal2headersdir=$(includedir)/libmokojournal2
libmokojournal2headers_DATA=$(public_headers)

