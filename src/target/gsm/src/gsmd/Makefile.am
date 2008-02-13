INCLUDES = $(all_includes) -I$(top_srcdir)/include
AM_CFLAGS = -std=gnu99
plugindir = $(libdir)/gsmd

gsmd-version.h:
	echo -e '#ifndef GSMD_VERSION' \
	'\n#define GSMD_VERSION "$(VERSION)" \\' > gsmd-version.h
	if test -d .svn; then \
	    echo -e '\t"+'`svnversion`'"' >> gsmd-version.h; \
	else \
	    echo -e '\t/* release version */' >> gsmd-version.h; \
	fi
	echo -e '#endif' >> gsmd-version.h

BUILT_SOURCES = gsmd-version.h
sbin_PROGRAMS = gsmd

gsmd_CFLAGS = -D PLUGINDIR=\"$(plugindir)\"
gsmd_SOURCES = gsmd.c atcmd.c select.c machine.c vendor.c unsolicited.c log.c \
	       usock.c talloc.c timer.c operator_cache.c ext_response.c \
	       sms_cb.c sms_pdu.c strl.c
gsmd_LDADD = -ldl
gsmd_LDFLAGS = -Wl,--export-dynamic

plugin_LTLIBRARIES = libgsmd-machine_generic.la \
		     libgsmd-machine_tihtc.la \
		     libgsmd-machine_gta01.la \
		     libgsmd-machine_telit.la \
		     libgsmd-vendor_ti.la \
		     libgsmd-vendor_tihtc.la \
		     libgsmd-vendor_qc.la \
		     libgsmd-vendor_bcm.la \
		     libgsmd-vendor_telit.la

libgsmd_machine_generic_la_SOURCES = machine_generic.c
libgsmd_machine_generic_la_LDFLAGS = -module
libgsmd_machine_tihtc_la_SOURCES = machine_tihtc.c
libgsmd_machine_tihtc_la_LDFLAGS = -module
libgsmd_machine_gta01_la_SOURCES = machine_gta01.c
libgsmd_machine_gta01_la_LDFLAGS = -module
libgsmd_machine_telit_la_SOURCES = machine_telit.c
libgsmd_machine_telit_la_LDFLAGS = -module

libgsmd_vendor_ti_la_SOURCES = vendor_ti.c
libgsmd_vendor_ti_la_LDFLAGS = -module
libgsmd_vendor_tihtc_la_SOURCES = vendor_tihtc.c
libgsmd_vendor_tihtc_la_LDFLAGS = -module
libgsmd_vendor_qc_la_SOURCES = vendor_qc.c
libgsmd_vendor_qc_la_LDFLAGS = -module
libgsmd_vendor_bcm_la_SOURCES = vendor_bcm.c
libgsmd_vendor_bcm_la_LDFLAGS = -module
libgsmd_vendor_telit_la_SOURCES = vendor_telit.c
libgsmd_vendor_telit_la_LDFLAGS = -module


noinst_HEADERS = gsmd.h
