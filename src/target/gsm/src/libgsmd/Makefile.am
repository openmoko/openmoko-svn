LIBVERSION= 0:0:0
INCLUDES = $(all_includes) -I$(top_srcdir)/include
AM_CFLAGS = -std=gnu99

lib_LTLIBRARIES = libgsmd.la

libgsmd_la_LDFLAGS = -Wc,-nostartfiles -version-info $(LIBVERSION)
libgsmd_la_SOURCES = libgsmd.c libgsmd_input.c libgsmd_voicecall.c libgsmd_passthrough.c libgsmd_event.c libgsmd_phone.c libgsmd_network.c libgsmd_pin.c libgsmd_sms.c libgsmd_phonebook.c libgsmd_modem.c

noinst_HEADERS = lgsm_internals.h
