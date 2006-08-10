
CC:=gcc -Wall
OBJS:=am29f800.o jtag.o k9s1208.o mem_rdwr.o pin2410.o sjf2410.o strata32.o ppt.o

CFLAGS:=-DLINUX_PPDEV
#CFGLAS:=-DCHAMELEON -D__WINDOWS__ 

sjf2410: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

clean:
	rm -f sjf2410 $(OBJS)
