gcc -DSWITCH_GTK2 -o switch2 switch2.c -g -O2 -Wall `pkg-config gtk+-2.0 --cflags` `pkg-config gtk+-2.0 --libs`
