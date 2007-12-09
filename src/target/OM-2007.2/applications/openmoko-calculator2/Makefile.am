AUTOMAKE_OPTIONS=foreign
SUBDIRS = src data po

INTLTOOL_BUILT = \
        intltool-extract \
        intltool-merge \
        intltool-update

EXTRA_DIST = $(INTLTOOL_BUILT:=.in)

DISTCLEANFILES = $(INTLTOOL_BUILT)
