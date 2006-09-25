export OPENMOKODIR=$PWD

find . -name "Makefile"|xargs rm -f
rm -rf lib/*
/usr/lib/qt4/bin/qmake
