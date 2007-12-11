#! /bin/sh
autoreconf -v --install || exit 1
libtoolize --copy --force || exit 1
glib-gettextize --force --copy || exit 1
intltoolize --copy --force --automake || exit 1
./configure --enable-maintainer-mode "$@"
