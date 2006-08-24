#!/bin/sh

unset GTK_CFLAGS
unset GTK_LIBS

export PKG_CONFIG_PATH=/home/openmoko/workstation/staging/i686-linux/share/pkgconfig
export LD_LIBRARY_PATH=/home/openmoko/workstation/staging/i686-linux/lib

autoreconf

./configure --prefix=$OPENMOKO_I686_INSTALL_DIR
make
