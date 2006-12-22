export OPENMOKODIR=$PWD

find . -name "Makefile"|xargs rm -f
rm -rf ./lib/lib*
rm -rf ./bin/*-*
qmake
make clean
make

export LD_LIBRARY_PATH=$OPENMOKODIR/lib:/usr/local/lib
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
