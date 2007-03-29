lib_LTLIBRARIES=libmokojournal.la

public_headers=moko-journal.h
sources=moko-journal.c

libmokojournal_la_SOURCES= $(sources) $(public_headers)
libmokojournal_la_LIBADD=
libmokojournal_la_LDFLAGS= -version-info 0:0:0
AM_CFLAGS = -Wall -pedantic -std=c99

libmokojournalheadersdir=$(includedir)/openmoko-libs/libmokojournal
libmokojournalheaders_DATA=$(public_headers)

