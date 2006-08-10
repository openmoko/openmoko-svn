/* qt2410_boot_usb - Ram Loader for Armzone QT2410 Devel Boards
 * (C) 2006 by Harald Welte <hwelte@hmw-consulting.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 
 *  as published by the Free Software Foundation
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * This program allows you to download executable code from the PC to the target
 * RAM.  After downloading it, the code can be executed.
 *
 * In order to make this work, the board must be running from the usb ram
 * loader in NOR flash rather than the regular OS in NAND.  To achieve this,
 * push move the small switch on the KERNEL board from NAND to NOR before powering
 * up the device.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <usb.h>

const char *
hexdump(const void *data, unsigned int len)
{
	static char string[65535];
	unsigned char *d = (unsigned char *) data;
	unsigned int i, left;

	string[0] = '\0';
	left = sizeof(string);
	for (i = 0; len--; i += 3) {
		if (i >= sizeof(string) -4)
			break;
		snprintf(string+i, 4, " %02x", *d++);
	}
	return string;
}

#define QT2410_VENDOR_ID	0x5345
#define QT2410_PRODUCT_ID	0x1234
#define QT2410_OUT_EP	0x03
#define QT2410_IN_EP	0x81

static struct usb_dev_handle *hdl;
static struct usb_device *find_qt2410_device(void)
{
	struct usb_bus *bus;

	for (bus = usb_busses; bus; bus = bus->next) {
		struct usb_device *dev;
		for (dev = bus->devices; dev; dev = dev->next) {
			if (dev->descriptor.idVendor == QT2410_VENDOR_ID
			    && dev->descriptor.idProduct == QT2410_PRODUCT_ID
			    && dev->descriptor.iManufacturer == 1
			    && dev->descriptor.iProduct == 2
			    && dev->descriptor.bNumConfigurations == 1
			    && dev->config->bNumInterfaces == 1
			    && dev->config->iConfiguration == 0)
				return dev;
		}
	}
	return NULL;
}


static u_int16_t qt2410_csum(const unsigned char *data, u_int32_t len)
{
	u_int16_t csum = 0;
	int j;

	for (j = 0; j < len; j ++) {
		csum += data[j];
	}

	return csum;
}

#define CHUNK_SIZE 100
static int qt2410_send_file(u_int32_t addr, void *data, u_int32_t len)
{
	int ret = 0;
	unsigned char *buf, *cur;
	u_int16_t csum = qt2410_csum(data, len);
	u_int32_t len_total = len + 10;

	printf("csum = 0x%4x\n", csum);

	/* 4 bytes address, 4 bytes length, data, 2 bytes csum */

	buf = malloc(len_total);
	if (!buf)
		return -ENOMEM;

	/* FIXME: endian safeness !!! */
	buf[0] = addr & 0xff;
	buf[1] = (addr >> 8) & 0xff;
	buf[2] = (addr >> 16) & 0xff;
	buf[3] = (addr >> 24) & 0xff;

	buf[4] = len_total & 0xff;
	buf[5] = (len_total >> 8) & 0xff;
	buf[6] = (len_total >> 16) & 0xff;
	buf[7] = (len_total >> 24) & 0xff;

	memcpy(buf+8, data, len);

	buf[len+8] = csum & 0xff;
	buf[len+9] = (csum >> 8) & 0xff;

	printf("send_file: addr = 0x%08x, len = 0x%08x\n", addr, len);

	for (cur = buf; cur < buf+len_total; cur += CHUNK_SIZE) {
		int remain = (buf + len_total) - cur;
		if (remain > CHUNK_SIZE)
			remain = CHUNK_SIZE;

#if 0
		printf("sending urb(0x%08x): %s\n", 
			addr + (cur - buf), hexdump(cur, remain));
#endif

		ret = usb_bulk_write(hdl, QT2410_OUT_EP, cur, remain, 0);
		if (ret < 0)
			break;
	}

	free(buf);

	return ret;
}

#if 0
static int qt2410_send_file(u_int32_t addr, void *data, u_int32_t len)
{
	int i, ret;
	u_int32_t cur_addr;
	void *cur_data;

	for (cur_addr = addr, cur_data = data; 
	     cur_addr < addr + len;
	     cur_addr += CHUNK_SIZE, cur_data += CHUNK_SIZE) {
	     	int remain = (data + len) - cur_data;
		if (remain > CHUNK_SIZE)
			remain = CHUNK_SIZE;
	
		ret = qt2410_send_chunk(cur_addr, cur_data, remain);
		if (ret < 0)
			return ret;
	}

	return 0;
}
#endif

//#define KERNEL_RAM_BASE	0x30008000
#define KERNEL_RAM_BASE	0x33F80000

int main(int argc, char **argv)
{
	struct usb_device *dev;
	char *filename, *prog;
	struct stat st;
	int fd;
	u_int32_t word = 0x7c7c7c7c;

	usb_init();
	if (!usb_find_busses())
		exit(1);
	if (!usb_find_devices())
		exit(1);

	dev = find_qt2410_device();
	if (!dev) {
		printf("Cannot find QT2410 device in bootloader mode\n");
		exit(1);
	}

	hdl = usb_open(dev);
	if (!hdl) {
		printf("Unable to open usb device: %s\n", usb_strerror());
		exit(1);
	}

	if (usb_claim_interface(hdl, 0) < 0) {
		printf("Unable to claim usb interface 1 of device: %s\n", usb_strerror());
		exit(1);
	}

	filename = argv[1];
	if (!filename) {
		printf("You have to specify the file you want to flash\n");
		exit(2);
	}

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		exit(2);

	if (fstat(fd, &st) < 0) {
		printf("Error to access file `%s': %s\n", filename, strerror(errno));
		exit(2);
	}

	/* mmap kernel image passed as parameter */
	prog = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (!prog)
		exit(1);

	if (qt2410_send_file(KERNEL_RAM_BASE, prog, st.st_size)) {
		printf("Error downloading program\n");
		exit(1);
	}
	exit(0);
}
