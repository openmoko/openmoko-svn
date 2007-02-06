SUBDIRS = src po data

INTLTOOL_BUILT = \
        intltool-extract \
        intltool-merge \
        intltool-update

EXTRA_DIST = $(INTLTOOL_BUILT:=.in)

DISTCLEANFILES = $(INTLTOOL_BUILT)
MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.sub configure depcomp $(INTLTOOL_BUILT:=.in) install-sh ltmain.sh mkinstalldirs Makefile.in missing

snapshot:
        $(MAKE) dist distdir=$(PACKAGE)-snap`date +"%Y%m%d"`
