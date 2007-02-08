#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "pin2410.h"
#include "jtag.h"
#include "k9s1208.h"
#include "sjf2410.h"


#define ECC_CHECK	(0)

//*************** JTAG dependent functions ***************
void K9S1208_JtagInit(void);
static void NF_CMD(U8 cmd);
static void NF_ADDR(U8 addr);
static void NF_nFCE_L(void);
static void NF_nFCE_H(void);
static U8 NF_RDDATA(void);
static void NF_WRDATA(U8 data);
static void NF_WAITRB(void);
//*************** H/W dependent functions ***************
static U16 NF_CheckId(void);
static int NF_EraseBlock(U32 blockNum);
static int NF_ReadPage(U32 block,U32 page,U8 *buffer,U8 *spareBuf);
static int NF_WritePage(U32 block,U32 page,U8 *buffer,U8 *spareBuf);
	//buffer size is 512 bytes
static int NF_IsBadBlock(U32 block);
static int NF_MarkBadBlock(U32 block);
static void NF_Reset(void);
static void NF_Init(void);
//*******************************************************

void OpenImageFile(char *filename);

void K9S1208_PrintBlock(void);
void K9S1208_Program(void);
void K9S1208_Read(void);

extern int bad_check;
extern char FileName[];

static U32 targetBlock;	    // Block number (0 ~ 4095)
static U32 targetSize;	    // Total byte size 
static U8 blockBuf[0x4000];

static void *function[][2]=
{
    (void *)K9S1208_Program,		"K9S1208 Program     ",
    (void *)K9S1208_PrintBlock,		"K9S1208 Pr BlkPage  ",
    (void *)K9S1208_Read,		"K9S1208 Read Flash  ",
    (void *)1,			    	"Exit                ",
    0,0
};


void K9S1208_Menu(void)
{
    int i;
    U16 id;

    printf("\n[K9S1208 NAND Flash JTAG Programmer]\n");
    K9S1208_JtagInit();
    NF_Init();

    id=NF_CheckId();
    if(id!=0xec76)
    {
	printf("ERROR: K9S1208 is not detected. Detected ID=0x%x.\n",id);
	return;
    }
    else
    {
    	printf("K9S1208 is detected. ID=0x%x\n",id);
    }

    while(1)
    {

	i=0;
    	while(1)
	{   //display menu
	    printf("%2d:%s",i,function[i][1]);
	    i++;
	    if((int)(function[i][0])==0)
	    {
		printf("\n");
		break;
	    }
	    if((i%4)==0)
		printf("\n");
	}

	printf("Select the function to test :");
	scanf("%d",&i);
	if( i>=0 && (i<((sizeof(function)/8)-2)) ) 
	    ( (void (*)(void)) (function[i][0]) )();  
	else
	    break; //Exit menu
    }
}



void K9S1208_Program(void)
{
    int i;
    int programError=0;
    U32 blockIndex;
    int noLoad=0;
    U8 spareBuf[16]=
	{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
    U8 *srcPt;
    U32 progSize=0;

    if (!*FileName) {
	    printf("need a file name (-f file_name)\n");
	    return;
    }
    OpenImageFile(FileName);

    printf("\n[SMC(K9S1208V0M) NAND Flash Writing Program]\n");
    
    printf("\nSource size:0h~%xh\n",imageSize-1);
    printf("\nAvailable target block number: 0~4095\n");
    printf("Input target block number:");
    scanf("%d",&targetBlock);
    targetSize=((imageSize+0x4000-1)/0x4000)*0x4000;
    printf("target start block number     =%d\n",targetBlock);
    printf("target size        (0x4000*n) =0x%x\n",targetSize);
    printf("STATUS:");
    blockIndex=targetBlock;
    while(1)
    {
	if(noLoad==0)
	{
	    LoadImageFile(blockBuf,0x4000);
	}
	noLoad=0;
	
	if(bad_check && NF_IsBadBlock(blockIndex) && blockIndex!=0 )
		// 1:bad 0:good
        {
	    blockIndex++;   // for next block
	    noLoad=1;
	    continue;
	}
	if(!NF_EraseBlock(blockIndex))
	{
	    blockIndex++;   // for next block
	    noLoad=1;
	    continue;
	}

	printf("E");
	fflush(stdout);
	srcPt=blockBuf;

	for(i=0;i<32;i++)
	{
	    if(!NF_WritePage(blockIndex,i,srcPt,NULL/*spareBuf*/))// block num, page num, buffer
	    {
	        programError=1;
	        break;
	    }

	    srcPt+=512;	// Increase buffer addr one pase size
	    printf("p");
	    fflush(stdout);
	}
	printf("\n");

        if(programError==1)
	{
	    blockIndex++;
	    noLoad=1;
	    programError=0;
	    continue;
	}
	progSize+=0x4000;
	if(progSize>=imageSize)
	    break;	// Exit while loop
	blockIndex++;
    }
}

void K9S1208_Read(void)
{
	int sourceBlock,numBlocks;
	int of, block, page;
	unsigned char buffer[512];

	if (!*FileName) {
		printf("need a file name (-f file_name)\n");
		return;
	}

	printf("\n[SMC(K9S1208V0M) NAND Flash Reading Program]\n");

	of = creat(FileName, 0660);
	if (of < 0) {
		printf("error opening out file");
		return;
	}
	printf("\nSource size:0h~%xh\n",imageSize-1);
	printf("\nAvailable target block number: 0~4095\n");
	printf("Input source block number:");
	scanf("%d",&sourceBlock);
	printf("\nAvailable source blocks (length): 0~%d\n",4096-sourceBlock);
	printf("Input source blocks (length):");
	scanf("%d",&numBlocks);
	printf("STATUS:");

	for (block = sourceBlock; block < sourceBlock+numBlocks; block++) {
		for (page = 0; page < 32; page++) {
			NF_ReadPage(block, page, buffer,NULL);
			write(of, buffer, 512);
			printf("r");
			fflush(stdout);
		}
		printf("\n");
	}
	printf("\n");

	close(of);
}
   

void K9S1208_PrintBlock(void)// Printf one page
{
    int i;
    U16 id;
    U32 block,page;
    U8	buffer[512+16];

    printf("\n[SMC(K9S1208) NAND Flash block read]\n");	
    
    NF_Init();
    id=NF_CheckId();
    printf("ID=%x(0xec76)\n",id);
    if(id!=0xec76)
	return;

    printf("Input target block number:");
    scanf("%d",&block);
    printf("Input target page number:");   
    scanf("%d",&page);
    
    NF_ReadPage(block,page,buffer,buffer+512);
    
    printf("block=%d,page=%d:",block,page);
    for(i=0;i<512;i++)
    {
        if(i%16==0)
	    printf("\n%3xh:",i);
        printf("%02x ",buffer[i]);
    }
    printf("\nS.A.:");

    for(i=512;i<512+16;i++)
    {
        printf("%02x ",buffer[i]);
    }

    printf("\n");    	
}

//*************************************************
//*************************************************
//**           H/W dependent functions           **
//************************************************* 
//*************************************************

// NAND Flash Memory Commands
#define	SEQ_DATA_INPUT			(0x80)
#define	READ_ID				(0x90)
#define	RESET				(0xFF)
#define	READ_1_1			(0x00)
#define	READ_1_2			(0x01)
#define	READ_2				(0x50)
#define	PAGE_PROGRAM			(0x10)
#define	BLOCK_ERASE			(0x60)
#define	BLOCK_ERASE_CONFIRM		(0xD0)
#define	READ_STATUS			(0x70)


// block0: reserved for boot strap
// block1~4095: used for OS image
// badblock SE: xx xx xx xx xx 00 ....
// good block SE: ECC0 ECC1 ECC2 FF FF FF ....

#define WRITEVERIFY  (0)  //verifing is enable at writing.

/*
#define NF_CMD(cmd)	{rNFCMD=cmd;}
#define NF_ADDR(addr)	{rNFADDR=addr;}	
#define NF_nFCE_L()	{rNFCONF&=~(1<<11);}
#define NF_nFCE_H()	{rNFCONF|=(1<<11);}
#define NF_RSTECC()	{rNFCONF|=(1<<12);}
#define NF_RDDATA() 	(rNFDATA)
#define NF_WRDATA(data) {rNFDATA=data;}

#define NF_WAITRB()    {while(!(rNFSTAT&(1<<0)));} 
	    //wait tWB and check F_RNB pin.   
*/
#define ID_K9S1208V0M	0xec76

static U8 seBuf[16]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

// 1block=(512+16)bytes x 32pages
// 4096block

// A[23:14][13:9]
//  block   page

static int NF_EraseBlock(U32 block)
{
    U32 blockPage=(block<<5);

    if(bad_check && NF_IsBadBlock(block) && block!=0)
	//block #0 can't be bad block for NAND boot
	return 0;

    NF_nFCE_L();
    
    NF_CMD(0x60);   // Erase one block 1st command

    NF_ADDR(blockPage&0xff);	    // Page number=0
    NF_ADDR((blockPage>>8)&0xff);   
    NF_ADDR((blockPage>>16)&0xff);

    NF_CMD(0xd0);   // Erase one blcok 2nd command
    
    Delay(1); //wait tWB(100ns)

    NF_WAITRB();    // Wait tBERS max 3ms.
    NF_CMD(0x70);   // Read status command

    if (NF_RDDATA()&0x1) // Erase error
    {	
    	NF_nFCE_H();
	printf("[ERASE_ERROR:block#=%d]\n",block);
	NF_MarkBadBlock(block);
	return 0;
    }
    else 
    {
    	NF_nFCE_H();
        return 1;
    }
}


static int NF_IsBadBlock(U32 block)
{
    unsigned int blockPage;
    U8 data;
    
    
    blockPage=(block<<5);	// For 2'nd cycle I/O[7:5] 
    
    NF_nFCE_L();    
    NF_CMD(0x50);		// Spare array read command
    NF_ADDR(517&0xf);		// Read the mark of bad block in spare array(M addr=5) 
    NF_ADDR(blockPage&0xff);	// The mark of bad block is in 0 page
    NF_ADDR((blockPage>>8)&0xff);   // For block number A[24:17]
    NF_ADDR((blockPage>>16)&0xff);  // For block number A[25]

    Delay(1);		// wait tWB(100ns)
    
    NF_WAITRB();	// Wait tR(max 12us)
    
    data=NF_RDDATA();

    NF_nFCE_H();    

    if(data!=0xff)
    {
    	printf("[block %d:bad block(%x)]\n",block,data);
    	return 1;
    }
    else
    {
	printf(".");
    	return 0;
    }
}


static int NF_MarkBadBlock(U32 block)
{
    int i;
    U32 blockPage=(block<<5);
 
    seBuf[0]=0xff;
    seBuf[1]=0xff;    
    seBuf[2]=0xff;    
    seBuf[5]=0x44;   // Bad blcok mark=0
    
    NF_nFCE_L(); 
    NF_CMD(0x50);   
    NF_CMD(0x80);   // Write 1st command
    
    NF_ADDR(0x0);		    // The mark of bad block is 
    NF_ADDR(blockPage&0xff);	    // marked 5th spare array 
    NF_ADDR((blockPage>>8)&0xff);   // in the 1st page.
    NF_ADDR((blockPage>>16)&0xff);  
    
    for(i=0;i<16;i++)
    {
	NF_WRDATA(seBuf[i]);	// Write spare array
    }

    NF_CMD(0x10);   // Write 2nd command
    
    Delay(1);  //tWB = 100ns. 

    NF_WAITRB();      // Wait tPROG(200~500us)
  
    NF_CMD(0x70);
    
    Delay(1);	 //twhr=60ns//
    
    if (NF_RDDATA()&0x1) // Spare arrray write error
    {	
    	NF_nFCE_H();
    	printf("[Program error is occurred but ignored]\n");
    }
    else 
    {
    	NF_nFCE_H();
    }

    printf("[block #%d is marked as a bad block]\n",block);
    return 1;
}


static int NF_ReadPage(U32 block,U32 page,U8 *buffer,U8 *spareBuf)
{
    int i;
    unsigned int blockPage;
    U8 *bufPt=buffer;
    
    page=page&0x1f;
    blockPage=(block<<5)+page;
        
    NF_nFCE_L();    
    NF_CMD(0x00);   // Read command
    NF_ADDR(0);	    // Column = 0
    NF_ADDR(blockPage&0xff);	    //
    NF_ADDR((blockPage>>8)&0xff);   // Block & Page num.
    NF_ADDR((blockPage>>16)&0xff);  //

    Delay(1);	    //wait tWB(100ns)/////??????
    
    NF_WAITRB();    // Wait tR(max 12us)
    
    for(i=0;i<(512);i++)
    {
    	*bufPt++=NF_RDDATA();	// Read one page
    }

    if(spareBuf!=NULL)
    {
	for(i=0;i<16;i++)
	    spareBuf[i]=NF_RDDATA();	// Read spare array
    }

    NF_nFCE_H();    

    return 1;
}


static int NF_WritePage(U32 block,U32 page,U8 *buffer,U8 *spareBuf)
{
    int i;
    U32 blockPage=(block<<5)+page;
    U8 *bufPt=buffer;

    NF_nFCE_L(); 
    NF_CMD(0x0);
    NF_CMD(0x80);		    // Write 1st command
    NF_ADDR(0);			    // Column 0
    NF_ADDR(blockPage&0xff);	    //
    NF_ADDR((blockPage>>8)&0xff);   // Block & page num.
    NF_ADDR((blockPage>>16)&0xff);  //

    for(i=0;i<512;i++)
    {
	NF_WRDATA(*bufPt++);	// Write one page to NFM from buffer
    }  

    if(spareBuf!=NULL)
    {
	for(i=0;i<16;i++)
	{
	    NF_WRDATA(spareBuf[i]);	// Write spare array(ECC and Mark)
	}
    }

    NF_CMD(0x10);   // Write 2nd command
    
    Delay(1);	    //tWB = 100ns. 

    NF_WAITRB();    //wait tPROG 200~500us;
 
    NF_CMD(0x70);   // Read status command   
    
    Delay(1);	    //twhr=60ns
    
    if (NF_RDDATA()&0x1) // Page write error
    {	
    	NF_nFCE_H();
	printf("[PROGRAM_ERROR:block#=%d]\n",block);
	NF_MarkBadBlock(block);
	return 0;
    }
    else 
    {
    	NF_nFCE_H();
    #if (WRITEVERIFY==1)
	//return NF_VerifyPage(block,page,pPage);	
    #else
	return 1;
    #endif
    }
}



static U16 NF_CheckId(void)
{
    U16 id;
    
    NF_nFCE_L();
    
    NF_CMD(0x90);
    NF_ADDR(0x0);
    
    Delay(1);	//wait tWB(100ns)
    
    id=NF_RDDATA()<<8;	// Maker code(K9S1208V:0xec)
    id|=NF_RDDATA();	// Devide code(K9S1208V:0x76)
    
    NF_nFCE_H();
    
    return id;
}


static void NF_Reset(void)
{
    NF_nFCE_L();

    NF_CMD(0xFF);   //reset command

    Delay(1);	    //tWB = 100ns. 

    NF_WAITRB();    //wait 200~500us;
     
    NF_nFCE_H();    
}


static void NF_Init(void)
{
    NF_Reset();

    //NF_nFCE_L();
    NF_CMD(READ_1_1);        
    //NF_nFCE_H();
}



//*************************************************
//*************************************************
//**     JTAG dependent primitive functions      **
//************************************************* 
//*************************************************
void K9S1208_JtagInit(void)
{
    JTAG_RunTestldleState();
    JTAG_ShiftIRState(EXTEST);

    S2410_SetPin(CLE,LOW); 
    S2410_SetPin(ALE,LOW); 
}


static void NF_CMD(U8 cmd)
{   
    
    //Command Latch Cycle
    S2410_SetPin(DATA0_7_CON ,LOW); //D[7:0]=output
    S2410_SetPin(nFCE,LOW); 
    S2410_SetPin(nFRE,HIGH); 
    S2410_SetPin(nFWE,LOW); //Because tCLS=0, CLE & nFWE can be changed simultaneously.
    S2410_SetPin(ALE,LOW); 
    S2410_SetPin(CLE,HIGH); 
    S2410_SetDataByte(cmd);
    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(nFWE,HIGH);
    JTAG_ShiftDRStateNoTdo(outCellValue); 

#if 1
    S2410_SetPin(CLE,LOW);	
    S2410_SetPin(DATA0_7_CON,HIGH); //D[7:0]=input
    JTAG_ShiftDRStateNoTdo(outCellValue); 
#endif
}


static void NF_ADDR(U8 addr)
{
    //rNFADDR=addr;
    S2410_SetPin(DATA0_7_CON ,LOW); //D[7:0]=output
    S2410_SetPin(nFCE,LOW); 
    S2410_SetPin(nFRE,HIGH); 
    S2410_SetPin(nFWE,LOW);
    S2410_SetPin(ALE,HIGH);
    S2410_SetPin(CLE,LOW);
    S2410_SetDataByte(addr);
    JTAG_ShiftDRStateNoTdo(outCellValue); 
    
    S2410_SetPin(nFWE,HIGH);
    JTAG_ShiftDRStateNoTdo(outCellValue); 
    
#if 1
    S2410_SetPin(ALE,LOW);	
    S2410_SetPin(DATA0_7_CON,HIGH); //D[7:0]=input
    JTAG_ShiftDRStateNoTdo(outCellValue); 
#endif
}


static void NF_nFCE_L(void)
{
    S2410_SetPin(nFCE,LOW); 
    JTAG_ShiftDRStateNoTdo(outCellValue); 
}


static void NF_nFCE_H(void)
{
    S2410_SetPin(nFCE,HIGH); 
    JTAG_ShiftDRStateNoTdo(outCellValue); 
}


static U8 NF_RDDATA(void)
{
    S2410_SetPin(DATA0_7_CON ,HIGH); //D[7:0]=input
    S2410_SetPin(nFRE,LOW);
    JTAG_ShiftDRStateNoTdo(outCellValue); 
    
    S2410_SetPin(nFRE,HIGH);
    JTAG_ShiftDRState(outCellValue,inCellValue); 
    return S2410_GetDataByte();
}

static void NF_WRDATA(U8 data)
{   
    S2410_SetPin(DATA0_7_CON ,LOW); //D[7:0]=output
    S2410_SetPin(nFWE,LOW);
    S2410_SetDataByte(data);
    JTAG_ShiftDRStateNoTdo(outCellValue); 
    
    S2410_SetPin(nFWE,HIGH);
    JTAG_ShiftDRStateNoTdo(outCellValue); 
}


static void NF_WAITRB(void)
{
    while(1)
    {
	JTAG_ShiftDRState(outCellValue,inCellValue); 
	if( S2410_GetPin(nWAIT)==HIGH && S2410_GetPin(NCON0)==HIGH)break;
    }
}

