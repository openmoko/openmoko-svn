/*************************************************************************************
 NAME: strata32.c
 DESC: Strata Flash Programming code through 2410 JTAG 
 HISTORY:
 JUN.14.2002:LSJ     : ported for 2410
 Aug.19.2002:purnnamu: ported for 2410 JTAG version.
 Nov.15.2002:purnnamu: - Fast programming by removing _RESET() in programming loop 
                         and omitting nGCS1 deassertion
                       - E28F128 ClearLockBit function is added at sector erase step.
 *************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include "def.h"

#include "pin2410.h"
#include "jtag.h"
//#include "K9S1208.h"
#include "sjf2410.h"

#include "mem_rdwr.h"

//I have not tested yet about fast programming. But, it will reduce the programming time.
//IF the problem is occurred, let FAST_ROM_PROGRAM  0.
#define	FAST_ROM_PROGRAM	1

#define TARGET_ADDR_28F128      0x08000000  // nGCS1, 128MB area

static U32 srcAddr;

static U32 targetOffset; 
static U32 targetAddress; 
static U32 targetSize; 

static int error_erase=0;       // Read Status Register, SR.5
static int error_program=0;     // Read Status Register, SR.4


static int  Strata_ProgFlash(U32 realAddr,U32 data);
static void Strata_EraseSector(int targetAddr);
static int  Strata_CheckID(int targetAddr);
static int  Strata_CheckDevice(int targetAddr);
static int  Strata_CheckBlockLock(int targetAddr);
static int Strata_ClearBlockLock(int targetAddr); 
static int  Strata_BlankCheck(int targetAddr,int targetSize);
static int  _WAIT(void);

#if FAST_ROM_PROGRAM	
    #define _WR(addr,data)  MRW_Wr32QQ(addr,data,0,0xf) 
#else
    #define _WR(addr,data)  MRW_Wr32Q(addr,data,0,0xf) 
#endif
#define _RD(addr)       MRW_Rd32Q(addr,0,0xf)      
#define _RESET()	MRW_Wr32Q(targetAddress,0x00ff00ff,0,0xf)


void Program28F128J3A(void)
{
    int i;
    U32 temp;

    MRW_JtagInit();

    printf("\n[28F128J3A Flash JTAG Programmer]\n");
    printf("\n*** Very Important Notes ***\n");
    printf("1. 28F128J3A must be located at 0x08000000.\n"
            "   J6  : connect 2-3 pins,   J9  : connect 1-2 pins\n"
            "   J33 : Open,               J34 : Short\n");
    printf("2. After programming, 28F128J3A may be located at 0x0.\n"
            "   J6  : connect 1-2 pins,   J9  : connect 2-3 pins\n"
            "   J33 : Short,              J34 : Open\n");
    
    targetAddress=TARGET_ADDR_28F128;
    targetSize=imageSize;

    printf("\nSource size = %xh\n",targetSize);
    printf("\nAvailable Target Offset Address: \n"); 
    printf("0x0,0x20000,0x40000, ..., 0x1ce0000\n");
    printf("Input target address offset [0x?] : ");
    scanf("%x",&targetOffset);

    printf("Target base address(0x08000000) = 0x%x\n",targetAddress);
    printf("Target offset      (0x0)        = 0x%x\n",targetOffset);
    printf("Target size        (0x20000*n)  = 0x%x\n",targetSize);

    if ( (Strata_CheckID(targetAddress) & 0xffff) != 0x0089 )       // ID number = 0x0089
    {
        printf("Identification check error !!\n");
        return ;
    }

    if ( (Strata_CheckDevice(targetAddress) & 0xffff) != 0x0018 )   // Device number=0x0018
    {
        printf("Device check error !!\n");
        return ;
    }

    printf("\nErase the sector from 0x%x.\n", targetAddress+targetOffset);

    for(i=0;i<targetSize;i+=0x20000)
    {
	Strata_ClearBlockLock(targetAddress+targetOffset+i); 
        Strata_EraseSector(targetAddress+targetOffset+i);
    }

#if 0    
    if(!Strata_BlankCheck(targetAddress+targetOffset,targetSize))
    {
        printf("Blank Check Error!!!\n");
        return;
    }
#else
    printf("\nBlank check is skipped.\n");
#endif

    printf("\nStart of the data writing...\n");

    srcAddr=(U32)malloc(targetSize+4);
    if(srcAddr==0)return;
    
    LoadImageFile((U8 *)srcAddr,targetSize);

    for (i=0; i<targetSize; i+=4) 
    {
        Strata_ProgFlash(i+targetAddress+targetOffset, *((U32 *)(srcAddr+i)));
        if((i%0x100)==0xfc)
            printf("[%x]",i+4);
    }
    printf("\nEnd of the data writing \n");

    _RESET();

#if 0
    printf("Verifying Start...\n");
    for (i=0; i<targetSize; i+=4) 
    {
	temp=_RD(i+targetAddress+targetOffset);
        if (temp != *((U32 *)(srcAddr+i))) 
        {
            printf("Verify error: src %08x = %08x\n", srcAddr+i, *((U32 *)(srcAddr+i)));
            printf("              dst %08x = %08x\n", i+targetAddress+targetOffset, temp);
            return;
        }
    }
    printf("Verifying End!!!");
#else
    printf("\nverifying is skipped.\n");
#endif

}




//==========================================================================================
int Strata_CheckID(int targetAddr) 
{
    //_RESET();
    _WR(targetAddr, 0x00900090); 
    return _RD(targetAddr); // Read Identifier Code, including lower, higher 16-bit, 8MB, Intel Strate Flash ROM
                            // targetAddress must be the beginning location of a Block Address
}

//==========================================================================================
int Strata_CheckDevice(int targetAddr) 
{
    //_RESET();
    _WR(targetAddr, 0x00900090);
    return _RD(targetAddr+0x4); // Read Device Code, including lower, higher 16-bit, 8MB, Intel Strate Flash ROM
                                // targetAddress must be the beginning location of a Block Address
}

//==========================================================================================
int Strata_CheckBlockLock(int targetAddr) 
{
    //_RESET();
    _WR(targetAddr, 0x00900090);
    return _RD(targetAddr+0x8); 
    // Read Block Lock configuration, 
    // targetAddress must be the beginning location of a Block Address
}


//==========================================================================================
int Strata_ClearBlockLock(int targetAddr) 
{
    U32 status;
    //_RESET();
    _WR(targetAddr, 0x00600060);
    _WR(targetAddr, 0x00d000d0);
    
    while(1)
    {
	status=_RD(targetAddr+0x8); 
	if(status&(1<<7))break;
    }

    _RESET();  
}

//==========================================================================================
void Strata_EraseSector(int targetAddress) 
{
    unsigned long ReadStatus;
    unsigned long bSR5;     // Erase and Clear Lock-bits Status, lower 16bit, 8MB Intel Strate Flash ROM
    unsigned long bSR5_2;   // Erase and Clear Lock-bits Status, higher 16bit, 8MB Intel Strate Flash ROM
    unsigned long bSR7;     // Write State Machine Status, lower 16bit, 8MB Intel Strate Flash ROM
    unsigned long bSR7_2;   // Write State Machine Status, higher 16bit, 8MB Intel Strate Flash ROM
    //_RESET();
//  _WR(targetAddress, 0x00200020);
//  _WR(targetAddress, 0x00d000d0);
    _WR(targetAddress, 0x00200020); // Block Erase, First Bus Cycle, targetAddress is the address withint the block
    _WR(targetAddress, 0x00d000d0); // Block Erase, Second Bus Cycle, targetAddress is the address withint the block
    
    //_RESET();
    _WR(targetAddress, 0x00700070); // Read Status Register, First Bus Cycle, targetAddress is any valid address within the device
    ReadStatus=_RD(targetAddress);  // Read Status Register, Second Bus Cycle, targetAddress is any valid address within the device
    bSR7=ReadStatus & (1<<7);       // lower 16-bit 8MB Strata
    bSR7_2=ReadStatus & (1<<(7+16));// higher 16-bit 8MB Strata
    while(!bSR7 || !bSR7_2) 
    {
        _WR(targetAddress, 0x00700070);
        ReadStatus=_RD(targetAddress);
        bSR7=ReadStatus & (1<<7);
        bSR7_2=ReadStatus & (1<<(7+16));
//      printf("wait !!\n");
    }

    _WR(targetAddress, 0x00700070); // When the block erase is complete, status register bit SR.5 should be checked. 
                    // If a block erase error is detected, the status register should be cleared before
                    // system software attempts correct actions.
    ReadStatus=_RD(targetAddress);  
    bSR5=ReadStatus & (1<<5);           // lower 16-bit 8MB Strata 
    bSR5_2=ReadStatus & (1<<(5+16));    // higher 16-bit 8MB Strata 
    if (bSR5==0 && bSR5_2==0) 
    {
        printf("Block @%xh Erase O.K. \n",targetAddress);
    } 
    else 
    {
        //printf("Error in Block Erasure!!\n");
        _WR(targetAddress, 0x00500050); // Clear Status Register
        error_erase=1;                  // But not major, is it casual ?
    }

    _RESET();   // write 0xffh(_RESET()) after the last opoeration to reset the device to read array mode.
}

//==========================================================================================
int Strata_BlankCheck(int targetAddr,int targetSize) 
{
    int i,j;
    for (i=0; i<targetSize; i+=4) 
    {
        j=_RD(i+targetAddr);
        if (j!=0xffffffff)      // In erasure it changes all block dta to 0xff
        {
            printf("E : %x = %x\n", (i+targetAddr), j);
            return 0;
        }
    }
    return 1;
}

//==========================================================================================
int Strata_ProgFlash(U32 realAddr,U32 data) 
{
    volatile U32 *ptargetAddr;
    unsigned long ReadStatus;
    unsigned long bSR4;    // Erase and Clear Lock-bits Status, lower 16bit, 8MB Intel Strate Flash ROM
    unsigned long bSR4_2;  // Erase and Clear Lock-bits Status, higher 16bit, 8MB Intel Strate Flash ROM
    unsigned long bSR7;    // Write State Machine Status, lower 16bit, 8MB Intel Strate Flash ROM
    unsigned long bSR7_2;  // Write State Machine Status, higher 16bit, 8MB Intel Strate Flash ROM

    //ptargetAddr = (volatile U32 *)realAddr;
    //_RESET();

    _WR(realAddr, 0x00400040);  // realAddr is any valid adress within the device
                                // Word/Byte Program(or 0x00100010 can be used)
    //*ptargetAddr=data;          // 32 bit data
    _WR(realAddr, data);

#if 0
    //_RESET();
    _WR(realAddr, 0x00700070);  // Read Status Register
    ReadStatus=_RD(realAddr);   // realAddr is any valid address within the device
    bSR7=ReadStatus & (1<<7);
    bSR7_2=ReadStatus & (1<<(7+16));
    while(!bSR7 || !bSR7_2) 
    {
        // _RESET();
        _WR(realAddr, 0x00700070);        // Read Status Register
        ReadStatus=_RD(realAddr);
        bSR7=ReadStatus & (1<<7);
        bSR7_2=ReadStatus & (1<<(7+16));
    }
    
    _WR(realAddr, 0x00700070); 
    ReadStatus=_RD(realAddr);             // Real Status Register
    bSR4=ReadStatus & (1<<4);
    bSR4_2=ReadStatus & (1<<(4+16));
    
    if (bSR4==0 && bSR4_2==0) 
    {
        //printf("Successful Program!!\n");
    } 
    else 
    {
        //printf("Error Program!!\n");
        _WR(realAddr, 0x00500050);          // Clear Status Register
        error_program=1;                    // But not major, is it casual ?
    }
#endif

#if !FAST_ROM_PROGRAM
    _RESET();
#endif
    return 0;
}

