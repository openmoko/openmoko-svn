
PY_DEFS=`pkg-config --variable=defsdir pygtk-2.0`
PYTHON_INCLUDES=-I/usr/include/python${PYTHON_VERSION}

CLEANFILES = \
	mokoui.defs \
	mokoui.c

mokoui.defs: ../../libmokoui/moko-finger-scroll.h
	python /usr/share/pygtk/2.0/codegen/h2def.py ../../libmokoui/moko-finger-scroll.h > mokoui.defs

mokoui.c: mokoui.defs mokoui.override
	pygtk-codegen-2.0 --prefix mokoui \
		--register $(PY_DEFS)/gdk-types.defs \
		-register $(PY_DEFS)/gtk-types.defs \
		-override mokoui.override \
		mokoui.defs > $@            

pyexec_LTLIBRARIES = mokoui.la

mokoui_la_SOURCES = mokoui.c mokouimodule.c 

# set the include path found by configure
INCLUDES= $(all_includes)

AM_CPPFLAGS =  $(PYTHON_INCLUDES)  $(GTK_CFLAGS)  $(PYGDK_CFLAGS)  $(PYGOB_CFLAGS) -I../../libmokoui/

# the library search path.
mokoui_la_LDFLAGS = -module -avoid-version $(all_libraries) $(GTK_LIBS) $(PYGDK_LIBS) $(PYGOB_LIBS) ../../libmokoui/libmokoui2.la

