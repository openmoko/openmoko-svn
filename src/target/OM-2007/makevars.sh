export OPENMOKODIR=$PWD

find . -name "Makefile"|xargs rm -f
rm -rf ./lib/lib*
rm -rf ./bin/*-*
/usr/lib/qt4/bin/qmake
make clean

export LD_LIBRARY_PATH=$OPENMOKODIR/lib

