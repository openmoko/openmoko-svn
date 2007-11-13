SUBDIRS = src data po

INTLTOOL_BUILT = \
        intltool-extract \
        intltool-merge \
        intltool-update

EXTRA_DIST = $(INTLTOOL_BUILT:=.in)

DISTCLEANFILES = $(INTLTOOL_BUILT)

# Extra clean files so that maintainer-clean removes *everything*
MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.sub \
                       configure depcomp install-sh ltmain.sh     \
                       Makefile.in missing config.h.in            \
                       intltool-extract intltool-extract.in \
                       intltool-merge intltool-merge.in \
                       intltool-update intltool-update.in \
                       mkinstalldirs


