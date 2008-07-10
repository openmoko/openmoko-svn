/*
 * Copyright (c) 2004-2006 Atheros Communications Inc.
 * All rights reserved.
 *
 *
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation;
 * 
 *  Software distributed under the License is distributed on an "AS
 *  IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 *  implied. See the License for the specific language governing
 *  rights and limitations under the License.
 * 
 * 
 * 
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/wireless.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#if defined(DWSIM)
#include <sched.h>
#endif

#include <a_config.h>
#include <a_osapi.h>
#include <athdefs.h>
#include <a_types.h>
#include "athdrv_linux.h"
#include "bmi_msg.h"

#define ADDRESS_FLAG                    0x01
#define LENGTH_FLAG                     0x02
#define PARAM_FLAG                      0x04
#define FILE_FLAG                       0x08
#define COUNT_FLAG                      0x10

#define BMI_TEST                        BMI_NO_COMMAND

/* Limit malloc size when reading/writing file */
#define MAX_BUF                         (8*1024)

const char *progname;
const char commands[] = 
"commands:\n\
--get --address=<register address>\n\
--set --address=<register address> --param=<register value>\n\
--read --address=<target address> --length=<bytes> --file=<filename>\n\
--write --address=<target address> [--file=<filename> | --param=<value>]\n\
--execute --address=<function start address> --param=<input param>\n\
--begin --address=<function start address>\n\
--info\n\
--test --address=<target address> --length=<cmd size> --count=<iterations>\n\
--done\n\
The options can also be given in the abbreviated form --option=x or -o x. The options can be given in any order";

#define A_ROUND_UP(x, y)             ((((x) + ((y) - 1)) / (y)) * (y))

INLINE void *
MALLOC(int nbytes)
{
    void *p= malloc(nbytes);

    if (!p)
    {
        err(1, "Cannot allocate memory\n");
    }

    return p;
}

static void
usage(void)
{
    fprintf(stderr, "usage:\n%s [-i device] commands\n", progname);
    fprintf(stderr, "%s\n", commands);
    exit(-1);
}

int
main (int argc, char **argv) {
    int c, s, fd;
    unsigned int address, length;
    unsigned int count, param;
    char filename[128], ifname[IFNAMSIZ];
    unsigned int cmd;
    struct ifreq ifr;
    char *buffer;
    struct stat filestat;
    int flag;
    int target_version = -1;
    int target_type = -1;

    progname = argv[0];
    if (argc == 1) usage();

    flag = 0;
    memset(filename, '\0', 128);
    memset(ifname, '\0', IFNAMSIZ);
    strcpy(ifname, "eth1");
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) err(1, "socket");

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"get", 0, NULL, 'g'},
            {"set", 0, NULL, 's'},
            {"read", 0, NULL, 'r'},
            {"test", 0, NULL, 't'},
            {"file", 1, NULL, 'f'},
            {"done", 0, NULL, 'd'},
            {"write", 0, NULL, 'w'},
            {"begin", 0, NULL, 'b'},
            {"count", 1, NULL, 'c'},
            {"param", 1, NULL, 'p'},
            {"length", 1, NULL, 'l'},
            {"execute", 0, NULL, 'e'},
            {"address", 1, NULL, 'a'},
            {"interface", 1, NULL, 'i'},
            {"info", 0, NULL, 'I'},
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, "rwtebdgsIf:l:a:p:i:c:",
                         long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'r':
            cmd = BMI_READ_MEMORY;
            break;

        case 'w':
            cmd = BMI_WRITE_MEMORY;
            break;

        case 'e':
            cmd = BMI_EXECUTE;
            break;

        case 'b':
            cmd = BMI_SET_APP_START;
            break;

        case 'd':
            cmd = BMI_DONE;
            break;

        case 'g':
            cmd = BMI_READ_SOC_REGISTER;
            break;

        case 's':
            cmd = BMI_WRITE_SOC_REGISTER;
            break;

        case 't':
            cmd = BMI_TEST;
            break;

        case 'f':
            memset(filename, '\0', 128);
            strcpy(filename, optarg);
            flag |= FILE_FLAG;
            break;

        case 'l':
            length = atoi(optarg);
            flag |= LENGTH_FLAG;
            break;

        case 'a':
            address = strtoul(optarg, NULL, 0);
            flag |= ADDRESS_FLAG;
            break;

        case 'p':
            param = strtoul(optarg, NULL, 0);
            flag |= PARAM_FLAG;
            break;

        case 'c':
            count = atoi(optarg);
            flag |= COUNT_FLAG;
            break;

        case 'i':
            memset(ifname, '\0', 8);
            strcpy(ifname, optarg);
            break;

        case 'I':
            cmd = BMI_GET_TARGET_INFO;
            break;

        default:
            usage();
        }
    }

    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    /* Verify that the Target is alive.  If not, wait for it. */
    {
        int rv;
        static int waiting_msg_printed = 0;

        buffer = (char *)MALLOC(sizeof(struct bmi_target_info));
        ((int *)buffer)[0] = AR6000_XIOCTL_TARGET_INFO;
        ifr.ifr_data = buffer;
        while ((rv=ioctl(s, AR6000_IOCTL_EXTENDED, &ifr)) < 0)
        {
            if (errno == ENODEV) {
                /* 
                 * Give the Target device a chance to start.
                 * Then loop back and see if it's alive.
                 */
                if (!waiting_msg_printed) {
                    printf("bmiloader is waiting for Target....\n");
                    waiting_msg_printed = 1;
                }
                usleep(100000); /* Wait for 100ms */
            } else {
                printf("Unexpected error on AR6000_XIOCTL_TARGET_INFO: %d\n", rv);
                exit(1);
            }
        }
        target_version = ((int *)buffer)[0];
        target_type = ((int *)buffer)[1];
        free(buffer);
    }
    switch(cmd)
    {
    case BMI_DONE:
        printf("BMI Done\n");
        buffer = (char *)MALLOC(4);
        ((int *)buffer)[0] = AR6000_XIOCTL_BMI_DONE;
        ifr.ifr_data = buffer;
        if (ioctl(s, AR6000_IOCTL_EXTENDED, &ifr) < 0)
        {
            err(1, ifr.ifr_name);
        }
        free(buffer);
        break;

    case BMI_TEST:
        if ((flag & (COUNT_FLAG | LENGTH_FLAG | ADDRESS_FLAG)) == 
            (COUNT_FLAG | LENGTH_FLAG | ADDRESS_FLAG))
        {
            printf("BMI Test (address: 0x%x, length: %d, count: %d)\n", 
                    address, length, count);
            buffer = (char *)MALLOC(16);
            ((int *)buffer)[0] = AR6000_XIOCTL_BMI_TEST;
            ((int *)buffer)[1] = address;
            ((int *)buffer)[2] = length;
            ((int *)buffer)[3] = count;
            ifr.ifr_data = buffer;
            if (ioctl(s, AR6000_IOCTL_EXTENDED, &ifr) < 0)
            {
                err(1, ifr.ifr_name);
            }
            free(buffer);
        }
        else usage();
        break;

    case BMI_READ_MEMORY:
        if ((flag & (ADDRESS_FLAG | LENGTH_FLAG | FILE_FLAG)) == 
            (ADDRESS_FLAG | LENGTH_FLAG | FILE_FLAG))
        {
            printf(
                 "BMI Read Memory (address: 0x%x, length: %d, filename: %s)\n",
                  address, length, filename);

            if ((fd = open(filename, O_CREAT|O_WRONLY|O_TRUNC, 00644)) < 0)
            {
                perror("Could not create a file");
                exit(1);
            }
            buffer = (char *)MALLOC(MAX_BUF + 12);

            {
                unsigned int remaining = length;

                while (remaining)
                {
                    length = (remaining > MAX_BUF) ? MAX_BUF : remaining;
                    ((int *)buffer)[0] = AR6000_XIOCTL_BMI_READ_MEMORY;
                    ((int *)buffer)[1] = address;

                    /*
                     * We round up the requested length because some
                     * SDIO Host controllers can't handle other lengths;
                     * but we still only write the requested number of
                     * bytes to the file.
                     */
                    ((int *)buffer)[2] = A_ROUND_UP(length, 4);
                    ifr.ifr_data = buffer;
                    if (ioctl(s, AR6000_IOCTL_EXTENDED, &ifr) < 0)
                    {
                        err(1, ifr.ifr_name);
                    }
                    else
                    {
                        write(fd, buffer, length);
                    }

                    remaining -= length;
                    address += length;
                }
            }

            close(fd);
            free(buffer);
        }
        else usage();
        break;

    case BMI_WRITE_MEMORY:
        if (!(flag & ADDRESS_FLAG))
        {
            usage(); /* no address specified */
        }
        if (!(flag & (FILE_FLAG | PARAM_FLAG)))
        {
            usage(); /* no data specified */
        }
        if ((flag & FILE_FLAG) && (flag & PARAM_FLAG))
        {
            usage(); /* too much data specified */
        }

        if (flag & FILE_FLAG)
        {
            printf(
                 "BMI Write Memory (address: 0x%x, filename: %s)\n",
                  address, filename);
            if ((fd = open(filename, O_RDONLY)) < 0)
            {
                perror("Could not open file");
                exit(1);
            }
            memset(&filestat, '\0', sizeof(struct stat));
            buffer = (char *)MALLOC(MAX_BUF + 12);
            fstat(fd, &filestat);
            length = filestat.st_size;
        }
        else
        { /* PARAM_FLAG */
            printf(
                 "BMI Write Memory (address: 0x%x, value: 0x%x)\n",
                  address, param);
            length = sizeof(param);
            buffer = (char *)MALLOC(length + 12);
            *(unsigned int *)(&buffer[12]) = param;
            fd = -1;
        }

        /*
         * Write length bytes of data to memory.
         * Data is either present in buffer OR
         * needs to be read from fd in MAX_BUF chunks.
         *
         * Within the kernel, the implementation of
         * AR6000_XIOCTL_BMI_WRITE_MEMORY further
         * limits the size of each transfer over the
         * interconnect according to BMI protocol
         * limitations.
         */ 
        {
            unsigned int remaining = length;

            while (remaining)
            {
                length = (remaining > MAX_BUF) ? MAX_BUF : remaining;
#if defined(DWSIM)
                if ((address & 0xf0000000) == 0) {
                    /*
                     * TBDXXX: 
                     * If it looks like we're writing to an XTENSA address,
                     * write at most 20 bytes at a time.  If it looks like
                     * we're talking MIPS, do the whole thing.  Currently,
                     * we can't write too much at once through BMI to XT
                     * because the Host driver internally divides the request
                     * into multiple BMI transactions and it waits for each
                     * transaction to complete before starting the next one.
                     * But the first one never completes because the ISS
                     * never gets an opportunity to execute properly because
                     * the kernel locks it out when it tries to access
                     * simulated target registers through diag window.
                     */
                    if (remaining > 20) {
                        length = 20;
                    } else {
                        length = remaining;
                    }
                }
#endif
                if (fd > 0)
                {
                    if (read(fd, &buffer[12], length) != length)
                    {
                        perror("read from file failed");
                        exit(1);
                    }
                }

                ((int *)buffer)[0] = AR6000_XIOCTL_BMI_WRITE_MEMORY;
                ((int *)buffer)[1] = address;

                /*
                 * We round up the requested length because some
                 * SDIO Host controllers can't handle other lengths.
                 * This generally isn't a problem for users, but it's
                 * something to be aware of.
                 */
                ((int *)buffer)[2] = A_ROUND_UP(length, 4);
                ifr.ifr_data = buffer;
                while (ioctl(s, AR6000_IOCTL_EXTENDED, &ifr) < 0)
                {
#if defined(DWSIM)
                    printf("Retry BMI write...address=0x%x length=0x%d\n", address, length);
#else
                    err(1, ifr.ifr_name);
#endif
                }

                remaining -= length;
                address += length;
            }
        }

        free(buffer);
        if (fd > 0)
        {
            close(fd);
        }

        break;

    case BMI_READ_SOC_REGISTER:
        if ((flag & (ADDRESS_FLAG)) == (ADDRESS_FLAG))
        {
            printf("BMI Read Register (address: 0x%x)\n", address);
            buffer = (char *)MALLOC(8);
            ((int *)buffer)[0] = AR6000_XIOCTL_BMI_READ_SOC_REGISTER;
            ((int *)buffer)[1] = address;
            ifr.ifr_data = buffer;
            if (ioctl(s, AR6000_IOCTL_EXTENDED, &ifr) < 0)
            {
                err(1, ifr.ifr_name);
            }
            param = ((int *)buffer)[0];
            printf("Return Value from target: 0x%x\n", param);
            free(buffer);
        }
        else usage();
        break;

    case BMI_WRITE_SOC_REGISTER:
        if ((flag & (ADDRESS_FLAG | PARAM_FLAG)) == (ADDRESS_FLAG | PARAM_FLAG))
        {
            printf("BMI Write Register (address: 0x%x, param: 0x%x)\n", address, param);
            buffer = (char *)MALLOC(12);
            ((int *)buffer)[0] = AR6000_XIOCTL_BMI_WRITE_SOC_REGISTER;
            ((int *)buffer)[1] = address;
            ((int *)buffer)[2] = param;
            ifr.ifr_data = buffer;
            if (ioctl(s, AR6000_IOCTL_EXTENDED, &ifr) < 0)
            {
                err(1, ifr.ifr_name);
            }
            free(buffer);
        }
        else usage();
        break;

    case BMI_EXECUTE:
        if ((flag & (ADDRESS_FLAG | PARAM_FLAG)) == (ADDRESS_FLAG | PARAM_FLAG))
        {
            printf("BMI Execute (address: 0x%x, param: 0x%x)\n", address, param);
            buffer = (char *)MALLOC(12);
            ((int *)buffer)[0] = AR6000_XIOCTL_BMI_EXECUTE;
            ((int *)buffer)[1] = address;
            ((int *)buffer)[2] = param;
            ifr.ifr_data = buffer;
            if (ioctl(s, AR6000_IOCTL_EXTENDED, &ifr) < 0)
            {
                err(1, ifr.ifr_name);
            }
            param = ((int *)buffer)[0];
            printf("Return Value from target: 0x%x\n", param);
            free(buffer);
        }
        else usage();
        break;

    case BMI_SET_APP_START:
        if ((flag & ADDRESS_FLAG) == ADDRESS_FLAG)
        {
            printf("BMI Set App Start (address: 0x%x)\n", address);
            buffer = (char *)MALLOC(8);
            ((int *)buffer)[0] = AR6000_XIOCTL_BMI_SET_APP_START;
            ((int *)buffer)[1] = address;
            ifr.ifr_data = buffer;
            if (ioctl(s, AR6000_IOCTL_EXTENDED, &ifr) < 0)
            {
                err(1, ifr.ifr_name);
            }
            free(buffer);
        }
        else usage();
        break;
    case BMI_GET_TARGET_INFO:
        printf("BMI Target Info:\n");
        printf("TARGET_TYPE=%s\n",
                (target_type == TARGET_TYPE_AR6001) ? "AR6001" :
                ((target_type == TARGET_TYPE_AR6002) ? "AR6002" : "unknown"));
        printf("TARGET_VERSION=0x%x\n", target_version);
        break;

    default:
        usage();
    }

    exit (0);
}
