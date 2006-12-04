#!/bin/sh

unset GTK_CFLAGS
unset GTK_LIBS

export CC="arm-linux-gcc -march=armv4t -mtune=arm920t"

export PATH="/openmoko/oe/build/tmp/staging/i686-linux/bin/arm-linux:/openmoko/oe/build/tmp/staging/i686-linux/bin:/openmoko/oe/build/tmp/cross/bin:/openmoko/oe/bitbake/bin:/stuff/bitbake/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/bin/X11"

export PKG_CONFIG_PATH="/openmoko/oe/build/tmp/staging/arm-linux/share/pkgconfig"

export CFLAGS="-isystem/openmoko/oe/build/tmp/staging/arm-linux/include -fexpensive-optimizations -fomit-frame-pointer -frename-registers -O2"


export LDFLAGS="-L/openmoko/oe/build/tmp/staging/arm-linux/lib -Wl,-rpath-link,/openmoko/oe/build/tmp/staging/arm-linux/lib -Wl,-O1"



autoreconf --verbose --install --force --exclude=autopoint -I /openmoko/oe/build/tmp/staging/arm-linux/share/aclocal-1.9 -I /openmoko/oe/build/tmp/staging/arm-linux/share/aclocal

./configure --build=i686-linux --host=arm-linux --target=arm-linux --prefix=$OPENMOKO_ARM_INSTALL_DIR
