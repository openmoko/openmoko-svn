
CFLAGS:=-I../../include -Wall -g
gsmd_OBJS:=gsmd.o atcmd.o select.o vendor_ti.o usock.o unsolicited.o

all: gsmd

gsmd: $(gsmd_OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

clean:
	rm -f $(gsmd_OBJS) gsmd
