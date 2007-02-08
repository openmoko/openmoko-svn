/********************************************************************************** 
 The SJF is written by analyzing ezTAG program, which was written by Jaewook Cheong,
 SEC S/W Centor. 
 Special thanks to Jaewook Cheong and Dale Roberts(the author of GIVEIO.sys)

 In SJF, the following feature is updated from ezTAG
 1. The structure of JTAG functions is changed for speed-up.
 2. The indexs of the pins become coherent with the BSDL file.
 3. SAMPLE/PRELOAD is used for initializing outCellValue[].
 4. The array size error is fixed(S3C2410_MAX_CELLS -> S3C2410_MAX_CELL_INDEX+2)
 5. The array was not followed by '\0', which is fixed.
 6. JTAG_ID reading error is fixed.
 7. Support K9S1208 SMD card for the SMDK2410 board.
 8. The programming speed is enhanced.
 **********************************************************************************/
/*******************************************
 Revision history
 2002.06.10:purnnamu:ver 0.1
  -first release
 2002.06.21:purnnamu:ver 0.11
  -trivial display error is fixed.
 2002.08.20:purnnamu:ver 0.3
  -Strata flash is supported.
 2002.08.20:purnnamu:ver 0.4
  -AM29LV800BB flash is supported.
 *******************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "def.h"
#include "pin2410.h"
#include "jtag.h"
#include "ppt.h"
#include "k9s1208.h"
#include "strata32.h"
#include "am29f800.h"
#include "mem_rdwr.h"

FILE *stream;
U32 imageSize;

char FileName[256];
int bad_check = 0;
void OpenImageFile(char *filename);
void OpenPpt(void);


static void *function[]=
{
    "K9S1208 prog    ",
    "28F128J3A prog  ",
    "AM29LV800 Prog  ",
    "Memory Rd/Wr    ",
    "Exit            ",
    0
};

static char *next_arg(char **argv,int *i)
{
    if (argv[*i][2])
	return argv[*i]+3;
    (*i)++;
    if (!argv[*i]) {
	fprintf(stderr,"argument expected\n");
	exit(1);
    }
    return argv[*i];
}

void main(int argc,char *argv[])
{
    char num=0;
    int i;
    	
    printf("\n");
    printf("+--------------------------------------+\n");
    printf("|     SEC JTAG FLASH(SJF) v 0.4moko2   |\n");
    printf("|     (S3C2410X & SMDK2410 B/D)        |\n");
    printf("+--------------------------------------+\n");
    //printf("Usage: SJF /f:<filename> /d=<delay> /b\n");
    printf("Usage: SJF -f <filename>  -d <delay>  -b\n");

    delayLoopCount=100;
    FileName[0]='\0';
    for(i=1;i<argc;i++)
    {
	switch(argv[i][1])
	{
	case 'f':
	    strcpy(FileName,next_arg(argv,&i));
	    break;
	case 'd':
	    delayLoopCount=atoi(next_arg(argv,&i));
	    break;
	case 'b':
	    bad_check = 1;
	    break;
	default:
	    printf("ERROR: unknown option /%c is detected.\n",argv[i][1]);
	    break;
	}
    }

    OpenPpt();
    	
    JTAG_ReadId();

    S2410_InitCell();
	
    printf("\n[SJF Main Menu]\n");
    i=0;
    while(1)
    {   //display menu
	printf("%2d:%s",i,function[i]);
	i++;
	if((int)(function[i])==0)
	{
	    printf("\n");
	    break;
	}
	if((i%4)==0)
	    printf("\n");
    }
    
    printf("Select the function to test:");
    scanf("%d",&i);
    switch(i)
    {
    case 0:
       	K9S1208_Menu();
	break;

    case 1:
	if(FileName[0]==0)
	{
	    printf("ERROR:Source file name is not valid.\n");
	    return;
	}
	OpenImageFile(FileName);
       	Program28F128J3A();
	break;

    case 2:
	if(FileName[0]==0)
	{
	    printf("ERROR:Source file name is not valid.\n");
	    return;
	}
	OpenImageFile(FileName);
       	ProgramAM29F800();
	break;

    case 3:
	MRW_Menu();
	break;

    default:
	return;
	break; //Exit menu
    }
    return;
}



void OpenImageFile(char *filename)
{
    U32 fileEnd,fileStart;
    stream = fopen(filename,"rb");
    if(stream==NULL)
    {
	printf("\nERROR:can't find the file.\n");
	exit(0);
    }

    fseek(stream,0L,SEEK_END);
    fileEnd=ftell(stream);
    fseek(stream,0L,SEEK_SET);
    fileStart=ftell(stream);

    imageSize=fileEnd-fileStart;  /*fileend == peof+1 */
}


int LoadImageFile(U8 *buf,int size)
{
    int i,readSize=size;
    for(i=0;i<size;i++)
    {
	if(feof(stream))
	{
	    readSize=i;
	    for(;i<size;i++)buf[i]=0;
	    break;
	}
	buf[i] = fgetc(stream);
    }
    return readSize;
}


void OpenPpt(void)
{
    if(!InstallGiveIo())
    {
        printf("ERROR: Couldn't open giveio.sys\n");
        exit(0);
    }

    validPpt = GetValidPpt();
    if(!validPpt)
    {
	printf("ERROR: Unable to find a parallel port\n");
	exit(0);
    }
    SetPptCompMode();	
}


