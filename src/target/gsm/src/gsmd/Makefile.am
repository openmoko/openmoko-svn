INCLUDES = $(all_includes) -I$(top_srcdir)/include
AM_CFLAGS = -std=gnu99

sbin_PROGRAMS = gsmd

gsmd_SOURCES = gsmd.c atcmd.c select.c vendor.c usock.c unsolicited.c log.c \
	       vendor_ti.c talloc.c
#gsmd_LDADD = ../libgsmd/libgsmd.la
#gsmd_LDFLAGS = -dynamic

noinst_HEADERS = gsmd.h
