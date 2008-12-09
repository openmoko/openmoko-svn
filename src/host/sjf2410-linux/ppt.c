#include <stdio.h>
#ifdef __WINDOWS__
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <stdlib.h>
#ifdef LINUX_PPDEV
#include <linux/parport.h>
#include <linux/ppdev.h>
#else
#include <sys/io.h>
#endif
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include "ppt.h"

/* Add support for Linux host:
 * (C) 2006 by Harald Welte <hwelte@hmw-consulting.de>
 *
 * Sponsored by FIC First International Computing, Taiwan.
 */

int validPpt; 

int GetValidPpt(void)
{
#ifdef __WINDOWS__
	// search for valid parallel port
	_outp(LPT1, 0x55);
	if((int)_inp(LPT1) == 0x55)
	    return LPT1;
	
	_outp(LPT2, 0x55);
	if((int)_inp(LPT2) == 0x55)
	    return LPT2;
	
	_outp(LPT3, 0x55);
	if((int)_inp(LPT3) == 0x55)
	    return LPT3;

	return 0;	
#else
#ifdef LINUX_PPDEV
	int i, ppt;

	ppt = open("/dev/parport0", O_WRONLY);
	if (ppt < 0) {
		fprintf(stderr, "can't open /dev/parport0\n");
		return 0;
	}

	i = ioctl(ppt, PPCLAIM);
	if (i < 0) {
		fprintf(stderr, "can't claim device\n");
		close(ppt);
		return 0;
	}
	return ppt;
#else
	if (ioperm(LPT1, 3, 1) != 0) {
		fprintf(stderr, "missing privileges for direct i/o");
		exit(1);
	}
		
	return LPT1;
#endif
#endif
}

#define ECP_ECR		    (0x402)
#define ECR_STANDARD	    (0x0)
#define ECR_DISnERRORINT    (0x10)
#define ECR_DISDMA	    (0x0)
#define ECR_DISSVCINT	    (0x4)

void SetPptCompMode(void)
{
#ifdef __WINDOWS__
    //configure the parallel port at the compatibility mode.
    _outp(validPpt+ECP_ECR,ECR_STANDARD | ECR_DISnERRORINT | ECR_DISDMA | ECR_DISSVCINT);
#else
#ifdef LINUX_PPDEV
	int i;

	i = PARPORT_MODE_COMPAT;
	i = ioctl(validPpt, PPSETMODE, &i);
	if (i < 0) {
		fprintf(stderr, "can't set compatible mode\n");
		close(validPpt);
		exit(1);
	}

	i = IEEE1284_MODE_COMPAT;
	i = ioctl(validPpt, PPNEGOT, &i);
	if (i < 0) {
		fprintf(stderr, "can't set compatible 1284 mode\n");
		close(validPpt);
		exit(1);
	}
#endif
#endif
}

int InstallGiveIo(void)
{
#ifdef __WINDOWS__
    HANDLE h;
    OSVERSIONINFO osvi;
    
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
	
    if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
	//OS=NT/2000
	h = CreateFile("\\\\.\\giveio", GENERIC_READ, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CloseHandle(h);
	if(h == INVALID_HANDLE_VALUE)
    	    return 0;
	else
	    return 0x2000;
    }
    else
    {	//OS=WIN98
	return 0x0098;
    }
#endif
}

#ifndef __WINDOWS__
#ifdef LINUX_PPDEV
void OutputPpt(unsigned char value)
{
	int i = value;

	ioctl(validPpt, PPWDATA, &i);
}

unsigned char InputPpt(void)
{
	int i;

	ioctl(validPpt, PPRSTATUS, &i);

	return (i & 0xff);
}
#else
void OutputPpt(unsigned char value)
{
	outb(value, validPpt);
}
unsigned char InputPpt(void)
{
	return inb(validPpt+1);
}
#endif
#endif
