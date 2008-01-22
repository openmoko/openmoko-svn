
PY_DEFS=`pkg-config --variable=defsdir pygtk-2.0`
PYTHON_INCLUDES=-I/usr/include/python${PYTHON_VERSION}

CLEANFILES = 
	moko.defs \
	moko.c

moko.defs: ../../libmokoui/moko-finger-scroll.h
	python /usr/share/pygtk/2.0/codegen/h2def.py ../../libmokoui/moko-finger-scroll.h > moko.defs

moko.c: moko.defs moko.override            
	pygtk-codegen-2.0 --prefix moko \
     --register $(PY_DEFS)/gdk-types.defs \
    --register $(PY_DEFS)/gtk-types.defs \
    --override moko.override \
    moko.defs > $@            

pyexec_LTLIBRARIES  =moko.la

moko_la_SOURCES =moko.c  mokomodule.c 

# set the include path found by configure
INCLUDES= $(all_includes)

AM_CPPFLAGS =  $(PYTHON_INCLUDES)  $(GTK_CFLAGS)  $(PYGDK_CFLAGS)  $(PYGOB_CFLAGS) -I../../libmokoui/

# the library search path.
moko_la_LDFLAGS =-module  -avoid-version  $(all_libraries)  $(GTK_LIBS) $(PYGDK_LIBS) $(PYGOB_LIBS) ../../libmokoui/libmokoui2.la
