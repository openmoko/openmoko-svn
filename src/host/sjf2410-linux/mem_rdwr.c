#include <stdio.h>
#include "pin2410.h"
#include "jtag.h"
#include "mem_rdwr.h"
#include "sjf2410.h"

static int MRW_RdByte(U32 addr,U8 *pDataByte);
static int MRW_RdHW(U32 addr,U16 *pDataHW);
static int MRW_RdWord(U32 addr,U32 *pDataWord);
static int MRW_WrByte(U32 addr,U8 dataByte);
static int MRW_WrHW(U32 addr,U16 dataHW);
static int MRW_WrWord(U32 addr,U32 dataWord);

int bankWidth[8]={16,16,16,16,16,16,16,16};
int bank_nBE[8]={0,0,1,1,1,1,1,1};


void MRW_Menu(void)
{
    int i,j;
    char command[128];
    U32 addr;
    U8 dataByte;
    U16 dataHW;
    U32 dataWord;
    int bank;
    int error;

    MRW_JtagInit();

    printf("\n[memory read/write command line]\n");
    printf("h:help x:exit\n");
    while(1)
    {
	error=0;
	printf(">");
	scanf("%s",command);
	switch(command[0])
	{
	case 'h':
	    printf("+---------------------- COMMAND LIST ------------------------+\n");
	    printf("| bs <bank #> <bw> <type> : set bank attribute               |\n");
	    printf("|                           bw=8,16,32 type:0(nWBE),1(nBE)   |\n");
	    printf("| bp                      : print bank attributes            |\n"); 
	    printf("| d <hex_addr>            : dump 64 bytes                    |\n");
	    printf("| rb <hex_addr>           : read, byte data                  |\n");
	    printf("| rh <hex_addr>           : read, half-word data             |\n");
	    printf("| rw <hex_addr>           : read, word data                  |\n");
    	    printf("| wb <hex_addr> <hex_data>: write, byte data                 |\n");
	    printf("| wh <hex_addr> <hex_data>: write, half-word data            |\n");
	    printf("| ww <hex_addr> <hex_data>: write, word data                 |\n");
	    printf("+-------------------------- NOTE ----------------------------+\n");
	    printf("| 1. nGCS6,7 SDRAM read/write isn't supported now.           |\n");
	    printf("| 2. example: >bs 2 16 1                                     |\n");
	    printf("|             >wb 110003e0 a5                                |\n");
	    printf("+------------------------------------------------------------+\n");
	    break;
	case 'x':
	    return;
	case 'b':
	    if(command[1]=='s')
	    {
	    scanf("%d",&bank);
	    scanf("%d %d",&bankWidth[bank],&bank_nBE[bank]);
	    printf("nGCS%d= width=%d write_type=%s\n",bank,bankWidth[bank],
		(bank_nBE[bank]==0)? "nWBE":"nBE");
	    }
	    else
	    {
    		for(i=0;i<7;i++)
		{
		    printf("nGCS%d width=%d %s\n",i,bankWidth[i],
			(bank_nBE[i]==0)? "nWBE":"nBE");
		}
	    }
	    break;

	case 'd':
	    scanf("%x",&addr);
	    addr=addr&0xfffffff0;
	    bank=S2410_Addr2Bank(addr);
	    for(i=0;i<4;i++)
	    {
		printf("%8x:",addr);
		for(j=0;j<16;j++)
		{
		    error=MRW_RdByte(addr++,&dataByte);
		    if(error!=0)
			printf("?? ");
		    else
			printf("%02x ",dataByte);
		}
		printf("\n");
	    }
	    break;
	case 'r': //read
	    switch(command[1])
	    {
	    case 'b': //read byte
		scanf("%x",&addr);
		addr=addr&0xffffffff;
		bank=S2410_Addr2Bank(addr);
		error=MRW_RdByte(addr,&dataByte);
		if(error!=0)
		    printf("ERROR:incorrect bank bus width\n");    
		else
		    printf("rb: %x(%d,%s): %02x\n",addr,bankWidth[bank],
			(bank_nBE[bank]==0)? "nWBE":"nBE",dataByte);
		break;
	    case 'h': //read half-word
		scanf("%x",&addr);
		addr=addr&0xfffffffe;
		bank=S2410_Addr2Bank(addr);
		error=MRW_RdHW(addr,&dataHW);
		if(error!=0)
		    printf("ERROR:incorrect bank bus width\n");    
		else
		    printf("rh: %x(%d,%s): %04x\n",addr,bankWidth[bank],
			(bank_nBE[bank]==0)? "nWBE":"nBE",dataHW);
		break;
	    case 'w': //read word
		scanf("%x",&addr);
		addr=addr&0xfffffffc;
		bank=S2410_Addr2Bank(addr);
		error=MRW_RdWord(addr,&dataWord);
		if(error!=0)
		    printf("ERROR:incorrect bank bus width\n");
		else
		    printf("rw: %x(%d,%s): %08x\n",addr,bankWidth[bank],
			(bank_nBE[bank]==0)? "nWBE":"nBE",dataWord);
		break;
	    default:
		printf("ERROR:incorrect command.\n");
		break;
	    }
	    break;

	case 'w': //write
	    switch(command[1])
	    {
	    case 'b': //write byte
		scanf("%x %x",&addr,&dataByte);
		addr=addr&0xffffffff;
		bank=S2410_Addr2Bank(addr);
		error=MRW_WrByte(addr,dataByte);
		if(error!=0)
		    printf("ERROR:incorrect bank bus width\n");
		else
		    printf("wb: %x(%d,%s): %02x\n",addr,bankWidth[bank],
			(bank_nBE[bank]==0)? "nWBE":"nBE",dataByte);
		break;
	    case 'h': //write half-word
		scanf("%x %x",&addr,&dataHW);
		addr=addr&0xfffffffe;
		bank=S2410_Addr2Bank(addr);
		
		error=MRW_WrHW(addr,dataHW);
		
		if(error!=0)
		    printf("ERROR:incorrect bank bus width\n");
		else
		    printf("wh: %x(%d,%s): %04x\n",addr,bankWidth[bank],
			(bank_nBE[bank]==0)? "nWBE":"nBE",dataHW);
		break;
	    case 'w': //write word
		scanf("%x %x",&addr,&dataWord);
		addr=addr&0xfffffffc;
		bank=S2410_Addr2Bank(addr);

		error=MRW_WrWord(addr,dataWord);

		if(error!=0)
		    printf("ERROR:incorrect bank bus width\n");
		else
		    printf("ww: %x(%d,%s): %08x\n",addr,bankWidth[bank],
			(bank_nBE[bank]==0)? "nWBE":"nBE",dataWord);
		break;
	    default:
		printf("ERROR:incorrect command.\n");
		break;
	    }
	    break;
        default:
	    printf("ERROR:incorrect command.\n");
	    break;
	}
    }
}


static int MRW_RdByte(U32 addr,U8 *pDataByte)
//address should be aligned
{
    U8 dataByte;
    int bank;
    int error=0;
		
    bank=S2410_Addr2Bank(addr);

    switch(bankWidth[bank])
    {
    case 8:
	dataByte=MRW_Rd8(addr);
	break;
    case 16:
	if((addr%2)==0)
	    dataByte=MRW_Rd16(addr,bank_nBE[bank],(1<<0))&0xff;
	else
	    dataByte=(MRW_Rd16(addr,bank_nBE[bank],(1<<1))&0xff00)>>8;
	break;
    case 32:
	switch(addr%4)
	{
	case 0:
	    dataByte=MRW_Rd32(addr,bank_nBE[bank],(1<<0))&0xff;
	    break;
	case 1:
    	    dataByte=(MRW_Rd32(addr,bank_nBE[bank],(1<<1))&0xff00)>>8;
	    break;
	case 2:
	    dataByte=(MRW_Rd32(addr,bank_nBE[bank],(1<<2))&0xff0000)>>16;
	    break;
	case 3:
    	    dataByte=(MRW_Rd32(addr,bank_nBE[bank],(1<<3))&0xff000000)>>24;
	    break;
	default:
	    error=1;
	    break;
	}
	break;
    default:
	error=1;
	break;
    }
    *pDataByte=dataByte;
    return error;
}


static int MRW_RdHW(U32 addr,U16 *pDataHW)
//address should be aligned
{
    U16 dataHW;
    int bank;
    int error=0;
		
    bank=S2410_Addr2Bank(addr);
    switch(bankWidth[bank])
    {
    case 16:
	dataHW=MRW_Rd16(addr,bank_nBE[bank],(3<<0));
	break;
    case 32:
	switch(addr%4)
	{
	case 0:
	    dataHW=MRW_Rd32(addr,bank_nBE[bank],(3<<0))&0xffff;
	    break;
	case 2:
	    dataHW=(MRW_Rd32(addr,bank_nBE[bank],(3<<2))&0xffff0000)>>16;
	    break;
	}
	break;
    default:
	error=1;
	break;
    }
    *pDataHW=dataHW;
    return error;
}


static int MRW_RdWord(U32 addr,U32 *pDataWord)
//address should be aligned
{
    U32 dataWord;
    int bank;
    int error=0;

    bank=S2410_Addr2Bank(addr);
    switch(bankWidth[bank])
    {
    case 32:
	dataWord=MRW_Rd32(addr,bank_nBE[bank],(0xf<<0));
	break;
    default:
	error=1;
	break;
    }
    *pDataWord=dataWord;
    return error;
}


static int MRW_WrByte(U32 addr,U8 dataByte)
{
    int bank;
    int error=0;

    bank=S2410_Addr2Bank(addr);
    switch(bankWidth[bank])
    {
    case 8:
	MRW_Wr8(addr,dataByte);
	break;
    case 16:
	if((addr%2)==0)
	    MRW_Wr16(addr,dataByte|(dataByte<<8),bank_nBE[bank],(1<<0));
	else
	    MRW_Wr16(addr,dataByte|(dataByte<<8),bank_nBE[bank],(1<<1));
	break;
    case 32:
	switch(addr%4)
	{
	case 0:
	    MRW_Wr32(addr,dataByte|(dataByte<<8)|(dataByte<<16)|(dataByte<<24),
		    bank_nBE[bank],(1<<0));
	    break;
	case 1:
	    MRW_Wr32(addr,dataByte|(dataByte<<8)|(dataByte<<16)|(dataByte<<24),
		    bank_nBE[bank],(1<<1));
	    break;
	case 2:
	    MRW_Wr32(addr,dataByte|(dataByte<<8)|(dataByte<<16)|(dataByte<<24),
		    bank_nBE[bank],(1<<2));
	    break;
	case 3:
	    MRW_Wr32(addr,dataByte|(dataByte<<8)|(dataByte<<16)|(dataByte<<24),
		    bank_nBE[bank],(1<<3));
	    break;
	}
	break;
    default:
	error=1;
	break;
    }
    return error;
}


static int MRW_WrHW(U32 addr,U16 dataHW)
//address should be aligned
{
    int bank;
    int error=0;

    bank=S2410_Addr2Bank(addr);
    switch(bankWidth[bank])
    {
    case 16:
	MRW_Wr16(addr,dataHW,bank_nBE[bank],(3<<0));
	break;
    case 32:
	switch(addr%4)
	{
	case 0:
	    MRW_Wr32(addr,dataHW|(dataHW<<16),bank_nBE[bank],(3<<0));
	    break;
	case 2:
	    MRW_Wr32(addr,dataHW|(dataHW<<16),bank_nBE[bank],(3<<2));
	    break;
	}
    default:
	error=1;
	break;
    }
    return error;
}


static int MRW_WrWord(U32 addr,U32 dataWord)
//address should be aligned
{
    int bank;
    int error=0;

    bank=S2410_Addr2Bank(addr);
    switch(bankWidth[bank])
    {
    case 32:
	MRW_Wr32(addr,dataWord,bank_nBE[bank],(0xf<<0));
	break;
    default:
	error=1;
	break;
    }
    return error;
}


//*************************************************
//*************************************************
//**     JTAG dependent primitive functions      **
//************************************************* 
//*************************************************
void MRW_JtagInit(void)
{
    JTAG_RunTestldleState();
    JTAG_ShiftIRState(EXTEST);

    //The initial value of pins will be defined here.
}


int S2410_Addr2Bank(U32 addr)
{
    if(addr<0x8000000)
	return 0;
    else if(addr<0x10000000)
	return 1;
    else if(addr<0x18000000)
	return 2;
    else if(addr<0x20000000)
	return 3;
    else if(addr<0x28000000)
	return 4;
    else if(addr<0x30000000)
	return 5;
    else if(addr<0x38000000)
	return 6;
    else if(addr<0x40000000)
	return 7;

    printf("ERROR:address range overflow\n");
    return 0;
}


void S2410_Assert_nGCS(U32 addr)
{
    if(addr<0x8000000)
	S2410_SetPin(nGCS0 ,LOW);
    else if(addr<0x10000000)
	S2410_SetPin(nGCS1 ,LOW);
    else if(addr<0x18000000)
	S2410_SetPin(nGCS2 ,LOW);
    else if(addr<0x20000000)
	S2410_SetPin(nGCS3 ,LOW);
    else if(addr<0x28000000)
	S2410_SetPin(nGCS4 ,LOW);
    else if(addr<0x30000000)
	S2410_SetPin(nGCS5 ,LOW);
    else if(addr<0x38000000)
	S2410_SetPin(nGCS6 ,LOW);
    else if(addr<0x40000000)
	S2410_SetPin(nGCS7 ,LOW);
}


void S2410_Deassert_nGCS(U32 addr)
{
    if(addr<0x8000000)
	S2410_SetPin(nGCS0 ,HIGH);
    else if(addr<0x10000000)
	S2410_SetPin(nGCS1 ,HIGH);
    else if(addr<0x18000000)
	S2410_SetPin(nGCS2 ,HIGH);
    else if(addr<0x20000000)
	S2410_SetPin(nGCS3 ,HIGH);
    else if(addr<0x28000000)
	S2410_SetPin(nGCS4 ,HIGH);
    else if(addr<0x30000000)
	S2410_SetPin(nGCS5 ,HIGH);
    else if(addr<0x38000000)
	S2410_SetPin(nGCS6 ,HIGH);
    else if(addr<0x40000000)
	S2410_SetPin(nGCS7 ,HIGH);
}




//***********************************************************
// Fast Version                   
// xxxQ: fast version by removing nGCS_to_nWEnOE setup time.
// xxxQQ: more fast version by omitting nGCS deassertion.
//***********************************************************
U8 MRW_Rd8Q(U32 addr)
{
    S2410_SetPin(DATA0_7_CON ,HIGH); 
    S2410_SetPin(DATA8_15_CON ,HIGH); 
    S2410_SetPin(DATA16_23_CON ,HIGH); 
    S2410_SetPin(DATA24_31_CON ,HIGH); 
    S2410_SetAddr(addr);
    S2410_Assert_nGCS(addr);
    S2410_SetPin(nOE,LOW); 
    JTAG_ShiftDRStateNoTdo(outCellValue); 
    
    S2410_SetPin(nOE,HIGH);
    S2410_Deassert_nGCS(addr);
    JTAG_ShiftDRState(outCellValue,inCellValue); 
    return S2410_GetDataByte();
}


U16 MRW_Rd16Q(U32 addr,int en_nBE,U32 bs)
{
    S2410_SetPin(DATA0_7_CON ,HIGH); 
    S2410_SetPin(DATA8_15_CON ,HIGH); 
    S2410_SetPin(DATA16_23_CON ,HIGH); 
    S2410_SetPin(DATA24_31_CON ,HIGH); 
    S2410_SetAddr(addr);
    S2410_Assert_nGCS(addr);
    S2410_SetPin(nOE,LOW); 
    if(en_nBE)
    {
	if(bs&(1<<0))S2410_SetPin(nBE0,LOW); 
	if(bs&(1<<1))S2410_SetPin(nBE1,LOW); 
    }
    JTAG_ShiftDRStateNoTdo(outCellValue); 
    
    S2410_SetPin(nOE,HIGH);
    S2410_Deassert_nGCS(addr);
    if(en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); 
	S2410_SetPin(nBE1,HIGH); 
    }
    JTAG_ShiftDRState(outCellValue,inCellValue); 
    return S2410_GetDataHW();
}


U32 MRW_Rd32Q(U32 addr,int en_nBE,U32 bs)
{
    S2410_SetPin(DATA0_7_CON ,HIGH); 
    S2410_SetPin(DATA8_15_CON ,HIGH); 
    S2410_SetPin(DATA16_23_CON ,HIGH); 
    S2410_SetPin(DATA24_31_CON ,HIGH); 
    S2410_SetAddr(addr);
    S2410_Assert_nGCS(addr);
    S2410_SetPin(nOE,LOW); 
    if(en_nBE)
    {
	if(bs&(1<<0))S2410_SetPin(nBE0,LOW); 
	if(bs&(1<<1))S2410_SetPin(nBE1,LOW); 
	if(bs&(1<<2))S2410_SetPin(nBE2,LOW); 
	if(bs&(1<<3))S2410_SetPin(nBE3,LOW); 
    }
    JTAG_ShiftDRStateNoTdo(outCellValue); 
    
    S2410_SetPin(nOE,HIGH);
    S2410_Deassert_nGCS(addr);
    if(en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); 
	S2410_SetPin(nBE1,HIGH); 
	S2410_SetPin(nBE2,HIGH); 
	S2410_SetPin(nBE3,HIGH); 
    }
    JTAG_ShiftDRState(outCellValue,inCellValue); 
    return S2410_GetDataWord();
}


void MRW_Wr8Q(U32 addr,U8 data)
{
    S2410_SetPin(DATA0_7_CON ,LOW); 
    S2410_SetPin(DATA8_15_CON ,LOW); 
    S2410_SetPin(DATA16_23_CON ,LOW); 
    S2410_SetPin(DATA24_31_CON ,LOW); 
    S2410_SetAddr(addr);
    S2410_Assert_nGCS(addr);
    S2410_SetPin(nWE,LOW); 
    S2410_SetDataByte(data);
    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(nWE,HIGH);
    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(DATA0_7_CON ,HIGH); 
    S2410_SetPin(DATA8_15_CON ,HIGH); 
    S2410_SetPin(DATA16_23_CON ,HIGH); 
    S2410_SetPin(DATA24_31_CON ,HIGH); 
    S2410_Deassert_nGCS(addr);
    JTAG_ShiftDRStateNoTdo(outCellValue); 
}


void MRW_Wr8QQ(U32 addr,U8 data)
{
    S2410_SetPin(DATA0_7_CON ,LOW); 
    S2410_SetPin(DATA8_15_CON ,LOW); 
    S2410_SetPin(DATA16_23_CON ,LOW); 
    S2410_SetPin(DATA24_31_CON ,LOW); 
    S2410_SetAddr(addr);
    S2410_Assert_nGCS(addr);
    S2410_SetPin(nWE,LOW); 
    S2410_SetDataByte(data);
    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(nWE,HIGH);
    JTAG_ShiftDRStateNoTdo(outCellValue); 
#if 0
    S2410_SetPin(DATA0_7_CON ,HIGH); 
    S2410_SetPin(DATA8_15_CON ,HIGH); 
    S2410_SetPin(DATA16_23_CON ,HIGH); 
    S2410_SetPin(DATA24_31_CON ,HIGH); 
    S2410_Deassert_nGCS(addr);
    JTAG_ShiftDRStateNoTdo(outCellValue); 
#endif
}


void MRW_Wr16Q(U32 addr,U16 data,int en_nBE,U32 bs)
{
    S2410_SetPin(DATA0_7_CON ,LOW); 
    S2410_SetPin(DATA8_15_CON ,LOW); 
    S2410_SetPin(DATA16_23_CON ,LOW); 
    S2410_SetPin(DATA24_31_CON ,LOW); 
    S2410_SetAddr(addr);
    S2410_Assert_nGCS(addr);
    S2410_SetPin(nWE,LOW);
    if(bs&(1<<0))S2410_SetPin(nBE0,LOW); //used as nWBE/nBE
    if(bs&(1<<1))S2410_SetPin(nBE1,LOW); 
    S2410_SetDataHW(data);
    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(nWE,HIGH);
    if(!en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); //nWBE is deasserted here.
	S2410_SetPin(nBE1,HIGH); 
    }
    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(DATA0_7_CON ,HIGH); //nBE is deasserted here.
    S2410_SetPin(DATA8_15_CON ,HIGH); 
    S2410_SetPin(DATA16_23_CON ,HIGH); 
    S2410_SetPin(DATA24_31_CON ,HIGH); 
    S2410_Deassert_nGCS(addr);
    if(en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); 
	S2410_SetPin(nBE1,HIGH); 
    }
    JTAG_ShiftDRStateNoTdo(outCellValue); 
}



void MRW_Wr16QQ(U32 addr,U16 data,int en_nBE,U32 bs)
// Very fast version nGCS is never inactive for speed-up.
{
    S2410_SetPin(DATA0_7_CON ,LOW); 
    S2410_SetPin(DATA8_15_CON ,LOW); 
    S2410_SetPin(DATA16_23_CON ,LOW); 
    S2410_SetPin(DATA24_31_CON ,LOW); 
    S2410_SetAddr(addr);
    S2410_Assert_nGCS(addr);
    S2410_SetPin(nWE,LOW);
    if(bs&(1<<0))S2410_SetPin(nBE0,LOW); //used as nWBE/nBE
    if(bs&(1<<1))S2410_SetPin(nBE1,LOW); 
    S2410_SetDataHW(data);
    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(nWE,HIGH);
    if(!en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); //nWBE is deasserted here.
	S2410_SetPin(nBE1,HIGH); 
    }
    JTAG_ShiftDRStateNoTdo(outCellValue); 

#if 0
    S2410_SetPin(DATA0_7_CON ,HIGH); //nBE is deasserted here.
    S2410_SetPin(DATA8_15_CON ,HIGH); 
    S2410_SetPin(DATA16_23_CON ,HIGH); 
    S2410_SetPin(DATA24_31_CON ,HIGH); 
    S2410_Deassert_nGCS(addr);
    if(en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); 
	S2410_SetPin(nBE1,HIGH); 
    }
    JTAG_ShiftDRStateNoTdo(outCellValue); 
#endif
}


void MRW_Wr32Q(U32 addr,U32 data,int en_nBE,U32 bs)
{
    S2410_SetPin(DATA0_7_CON ,LOW); 
    S2410_SetPin(DATA8_15_CON ,LOW); 
    S2410_SetPin(DATA16_23_CON ,LOW); 
    S2410_SetPin(DATA24_31_CON ,LOW); 
    S2410_SetAddr(addr);
    S2410_Assert_nGCS(addr);
    S2410_SetPin(nWE,LOW);
    
    if(bs&(1<<0))S2410_SetPin(nBE0,LOW); //used as nWBE/nBE
    if(bs&(1<<1))S2410_SetPin(nBE1,LOW); 
    if(bs&(1<<2))S2410_SetPin(nBE2,LOW); 
    if(bs&(1<<3))S2410_SetPin(nBE3,LOW); 
    S2410_SetDataWord(data);
    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(nWE,HIGH);
    if(!en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); //nWBE is deasserted here.
	S2410_SetPin(nBE1,HIGH); 
	S2410_SetPin(nBE2,HIGH); 
	S2410_SetPin(nBE3,HIGH); 
    }

    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(DATA0_7_CON ,HIGH); 
    S2410_SetPin(DATA8_15_CON ,HIGH); 
    S2410_SetPin(DATA16_23_CON ,HIGH); 
    S2410_SetPin(DATA24_31_CON ,HIGH); 
    S2410_Deassert_nGCS(addr);
    if(en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); //nBE is deasserted here.
	S2410_SetPin(nBE1,HIGH); 
	S2410_SetPin(nBE2,HIGH); 
	S2410_SetPin(nBE3,HIGH); 
    }
    JTAG_ShiftDRStateNoTdo(outCellValue); 
}


void MRW_Wr32QQ(U32 addr,U32 data,int en_nBE,U32 bs)
{
    S2410_SetPin(DATA0_7_CON ,LOW); 
    S2410_SetPin(DATA8_15_CON ,LOW); 
    S2410_SetPin(DATA16_23_CON ,LOW); 
    S2410_SetPin(DATA24_31_CON ,LOW); 
    S2410_SetAddr(addr);
    S2410_Assert_nGCS(addr);
    S2410_SetPin(nWE,LOW);
    
    if(bs&(1<<0))S2410_SetPin(nBE0,LOW); //used as nWBE/nBE
    if(bs&(1<<1))S2410_SetPin(nBE1,LOW); 
    if(bs&(1<<2))S2410_SetPin(nBE2,LOW); 
    if(bs&(1<<3))S2410_SetPin(nBE3,LOW); 
    S2410_SetDataWord(data);
    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(nWE,HIGH);
    if(!en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); //nWBE is deasserted here.
	S2410_SetPin(nBE1,HIGH); 
	S2410_SetPin(nBE2,HIGH); 
	S2410_SetPin(nBE3,HIGH); 
    }

    JTAG_ShiftDRStateNoTdo(outCellValue); 

#if 0
    S2410_SetPin(DATA0_7_CON ,HIGH); 
    S2410_SetPin(DATA8_15_CON ,HIGH); 
    S2410_SetPin(DATA16_23_CON ,HIGH); 
    S2410_SetPin(DATA24_31_CON ,HIGH); 
    S2410_Deassert_nGCS(addr);
    if(en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); //nBE is deasserted here.
	S2410_SetPin(nBE1,HIGH); 
	S2410_SetPin(nBE2,HIGH); 
	S2410_SetPin(nBE3,HIGH); 
    }
    JTAG_ShiftDRStateNoTdo(outCellValue); 
#endif
}


U8 MRW_Rd8(U32 addr)
{
    S2410_SetPin(DATA0_7_CON ,HIGH); 
    S2410_SetPin(DATA8_15_CON ,HIGH); 
    S2410_SetPin(DATA16_23_CON ,HIGH); 
    S2410_SetPin(DATA24_31_CON ,HIGH); 
    S2410_SetAddr(addr);
    
    S2410_Assert_nGCS(addr);
    JTAG_ShiftDRStateNoTdo(outCellValue); //tCOS

    S2410_SetPin(nOE,LOW); 
    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(nOE,HIGH);
    S2410_Deassert_nGCS(addr);
    JTAG_ShiftDRState(outCellValue,inCellValue); 
    return S2410_GetDataByte();
}


U16 MRW_Rd16(U32 addr,int en_nBE,U32 bs)
{
    S2410_SetPin(DATA0_7_CON ,HIGH); 
    S2410_SetPin(DATA8_15_CON ,HIGH); 
    S2410_SetPin(DATA16_23_CON ,HIGH); 
    S2410_SetPin(DATA24_31_CON ,HIGH); 
    S2410_SetAddr(addr);
    S2410_Assert_nGCS(addr);
    if(en_nBE)
    {
	if(bs&(1<<0))S2410_SetPin(nBE0,LOW); 
	if(bs&(1<<1))S2410_SetPin(nBE1,LOW); 
    }
    JTAG_ShiftDRStateNoTdo(outCellValue); //tCOS

    S2410_SetPin(nOE,LOW); 
    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(nOE,HIGH);
    S2410_Deassert_nGCS(addr);
    if(en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); 
	S2410_SetPin(nBE1,HIGH); 
    }
    JTAG_ShiftDRState(outCellValue,inCellValue); 
    return S2410_GetDataHW();
}


U32 MRW_Rd32(U32 addr,int en_nBE,U32 bs)
{
    S2410_SetPin(DATA0_7_CON ,HIGH); 
    S2410_SetPin(DATA8_15_CON ,HIGH); 
    S2410_SetPin(DATA16_23_CON ,HIGH); 
    S2410_SetPin(DATA24_31_CON ,HIGH); 
    S2410_SetAddr(addr);
    S2410_Assert_nGCS(addr);
    if(en_nBE)
    {
	if(bs&(1<<0))S2410_SetPin(nBE0,LOW); 
	if(bs&(1<<1))S2410_SetPin(nBE1,LOW); 
	if(bs&(1<<2))S2410_SetPin(nBE2,LOW); 
	if(bs&(1<<3))S2410_SetPin(nBE3,LOW); 
    }
    JTAG_ShiftDRStateNoTdo(outCellValue); //tCOS
    
    S2410_SetPin(nOE,LOW); 
    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(nOE,HIGH);
    S2410_Deassert_nGCS(addr);
    if(en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); 
	S2410_SetPin(nBE1,HIGH); 
	S2410_SetPin(nBE2,HIGH); 
	S2410_SetPin(nBE3,HIGH); 
    }
    JTAG_ShiftDRState(outCellValue,inCellValue); 
    return S2410_GetDataWord();
}


void MRW_Wr8(U32 addr,U8 data)
{
    S2410_SetPin(DATA0_7_CON ,LOW); 
    S2410_SetPin(DATA8_15_CON ,LOW); 
    S2410_SetPin(DATA16_23_CON ,LOW); 
    S2410_SetPin(DATA24_31_CON ,LOW); 
    S2410_SetAddr(addr);
    S2410_Assert_nGCS(addr);
    S2410_SetDataByte(data);
    JTAG_ShiftDRStateNoTdo(outCellValue); //tCOS

    S2410_SetPin(nWE,LOW); 
    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(nWE,HIGH);
    JTAG_ShiftDRStateNoTdo(outCellValue); //tCOH

    S2410_SetPin(DATA0_7_CON ,HIGH); 
    S2410_SetPin(DATA8_15_CON ,HIGH); 
    S2410_SetPin(DATA16_23_CON ,HIGH); 
    S2410_SetPin(DATA24_31_CON ,HIGH); 
    S2410_Deassert_nGCS(addr);
    JTAG_ShiftDRStateNoTdo(outCellValue); 
}


void MRW_Wr16(U32 addr,U16 data,int en_nBE,U32 bs)
{
    S2410_SetPin(DATA0_7_CON ,LOW); 
    S2410_SetPin(DATA8_15_CON ,LOW); 
    S2410_SetPin(DATA16_23_CON ,LOW); 
    S2410_SetPin(DATA24_31_CON ,LOW); 
    S2410_SetAddr(addr);
    S2410_Assert_nGCS(addr);
    if(en_nBE)
    {
	if(bs&(1<<0))S2410_SetPin(nBE0,LOW); //used as nBE
	if(bs&(1<<1))S2410_SetPin(nBE1,LOW); 
    }
    S2410_SetDataHW(data);
    JTAG_ShiftDRStateNoTdo(outCellValue); //tCOS

    if(!en_nBE)
    {
	if(bs&(1<<0))S2410_SetPin(nBE0,LOW); //used as nWBE
	if(bs&(1<<1))S2410_SetPin(nBE1,LOW); //used as nWBE
    }
    S2410_SetPin(nWE,LOW);
    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(nWE,HIGH);
    if(!en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); //nWBE is deasserted here.
	S2410_SetPin(nBE1,HIGH); 
    }
    JTAG_ShiftDRStateNoTdo(outCellValue); //tCOH

    S2410_SetPin(DATA0_7_CON ,HIGH); //nBE is deasserted here.
    S2410_SetPin(DATA8_15_CON ,HIGH); 
    S2410_SetPin(DATA16_23_CON ,HIGH); 
    S2410_SetPin(DATA24_31_CON ,HIGH); 
    S2410_Deassert_nGCS(addr);
    if(en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); 
	S2410_SetPin(nBE1,HIGH); 
    }
    JTAG_ShiftDRStateNoTdo(outCellValue); 
}


void MRW_Wr32(U32 addr,U32 data,int en_nBE,U32 bs)
{
    S2410_SetPin(DATA0_7_CON ,LOW); 
    S2410_SetPin(DATA8_15_CON ,LOW); 
    S2410_SetPin(DATA16_23_CON ,LOW); 
    S2410_SetPin(DATA24_31_CON ,LOW); 
    S2410_SetAddr(addr);
    S2410_Assert_nGCS(addr);
    if(en_nBE)
    {
	if(bs&(1<<0))S2410_SetPin(nBE0,LOW); //used as nBE
	if(bs&(1<<1))S2410_SetPin(nBE1,LOW); 
	if(bs&(1<<2))S2410_SetPin(nBE2,LOW); 
	if(bs&(1<<3))S2410_SetPin(nBE3,LOW); 
    }
    S2410_SetDataWord(data);
    JTAG_ShiftDRStateNoTdo(outCellValue); //tCOS

    if(!en_nBE)
    {
	if(bs&(1<<0))S2410_SetPin(nBE0,LOW); //used as nWBE
	if(bs&(1<<1))S2410_SetPin(nBE1,LOW); 
	if(bs&(1<<2))S2410_SetPin(nBE2,LOW); 
	if(bs&(1<<3))S2410_SetPin(nBE3,LOW); 
    }
    S2410_SetPin(nWE,LOW);
    JTAG_ShiftDRStateNoTdo(outCellValue); 

    S2410_SetPin(nWE,HIGH);
    if(!en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); //nWBE is deasserted here.
	S2410_SetPin(nBE1,HIGH); 
	S2410_SetPin(nBE2,HIGH); 
	S2410_SetPin(nBE3,HIGH); 
    }
    JTAG_ShiftDRStateNoTdo(outCellValue); //tCOH

    S2410_SetPin(DATA0_7_CON ,HIGH); 
    S2410_SetPin(DATA8_15_CON ,HIGH); 
    S2410_SetPin(DATA16_23_CON ,HIGH); 
    S2410_SetPin(DATA24_31_CON ,HIGH); 
    S2410_Deassert_nGCS(addr);
    if(en_nBE)
    {
	S2410_SetPin(nBE0,HIGH); //nBE is deasserted here.
	S2410_SetPin(nBE1,HIGH); 
	S2410_SetPin(nBE2,HIGH); 
	S2410_SetPin(nBE3,HIGH); 
    }
    JTAG_ShiftDRStateNoTdo(outCellValue); 
}

