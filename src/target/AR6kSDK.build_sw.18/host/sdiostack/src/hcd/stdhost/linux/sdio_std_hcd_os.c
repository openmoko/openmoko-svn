/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
@file: sdio_std_hcd_os.c

@abstract: Generic Linux implementation for the Standard SDIO Host Controller Driver

#notes: 
 
@notice: Copyright (c), 2006 Atheros Communications, Inc.


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
 *  Portions of this code were developed with information supplied from the 
 *  SD Card Association Simplified Specifications. The following conditions and disclaimers may apply:
 * 
 *   The following conditions apply to the release of the SD simplified specification (�Simplified
 *   Specification�) by the SD Card Association. The Simplified Specification is a subset of the complete 
 *   SD Specification which is owned by the SD Card Association. This Simplified Specification is provided 
 *   on a non-confidential basis subject to the disclaimers below. Any implementation of the Simplified 
 *   Specification may require a license from the SD Card Association or other third parties.
 *   Disclaimers:
 *   The information contained in the Simplified Specification is presented only as a standard 
 *   specification for SD Cards and SD Host/Ancillary products and is provided "AS-IS" without any 
 *   representations or warranties of any kind. No responsibility is assumed by the SD Card Association for 
 *   any damages, any infringements of patents or other right of the SD Card Association or any third 
 *   parties, which may result from its use. No license is granted by implication, estoppel or otherwise 
 *   under any patent or other rights of the SD Card Association or any third party. Nothing herein shall 
 *   be construed as an obligation by the SD Card Association to disclose or distribute any technical 
 *   information, know-how or other confidential information to any third party.
 * 
 * 
 *  The initial developers of the original code are Seung Yi and Paul Lever
 * 
 *  sdio@atheros.com
 * 
 * 

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* debug level for this module*/
#define DBG_DECLARE 4;
#include <ctsystem.h>
#include "../sdio_std_hcd.h"
#include "sdio_std_hcd_linux_lib.h"
#include <linux/dma-mapping.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
#define ATH_INIT_WORK(_t, _f, _c)      INIT_WORK((_t), (void (*)(void *))(_f), (_c));
#else
#define ATH_INIT_WORK(_t, _f, _c)      INIT_DELAYED_WORK((_t), (_f));
#endif

static void hcd_iocomplete_wqueue_handler(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
void *context); 
#else
struct work_struct *work);
#endif

static void hcd_carddetect_wqueue_handler(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
void *context); 
#else
struct work_struct *work);   
#endif

static void hcd_sdioirq_wqueue_handler(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
void *context); 
#else
struct work_struct *work);   
#endif

static SDIO_STATUS SetupDmaBuffers(PSDHCD_INSTANCE pHcInstance);
static void DeinitializeStdHcdInstance(PSDHCD_INSTANCE pHcInstance);

/* debug print parameter */
module_param(debuglevel, int, 0644);
MODULE_PARM_DESC(debuglevel, "debuglevel 0-7, controls debug prints");

    /* defaults for all std hosts, various attributes will be cleared based
     * on values from the capabilities register */
#define DEFAULT_ATTRIBUTES (SDHCD_ATTRIB_BUS_1BIT      | \
                            SDHCD_ATTRIB_BUS_4BIT      | \
                            SDHCD_ATTRIB_MULTI_BLK_IRQ | \
                            SDHCD_ATTRIB_AUTO_CMD12    | \
                            SDHCD_ATTRIB_POWER_SWITCH  | \
                            SDHCD_ATTRIB_BUS_MMC8BIT   | \
                            SDHCD_ATTRIB_SD_HIGH_SPEED | \
                            SDHCD_ATTRIB_MMC_HIGH_SPEED)
                            
static UINT32 hcdattributes = DEFAULT_ATTRIBUTES;
module_param(hcdattributes, int, 0644);
MODULE_PARM_DESC(hcdattributes, "STD Host Attributes");
static INT BaseClock = 0;
module_param(BaseClock, int, 0444);
MODULE_PARM_DESC(BaseClock, "BaseClock Hz when not present in configuration");
static UINT32 timeout = HOST_REG_TIMEOUT_CONTROL_DEFAULT;
module_param(timeout, int, 0644);
MODULE_PARM_DESC(timeout, "STD Host data timeout control");
static UINT32 ClockSpinLimit = HCD_COMMAND_MIN_POLLING_CLOCK;
module_param(ClockSpinLimit, int, 0644);
MODULE_PARM_DESC(ClockSpinLimit, "STD Host command clock spin time");

static UINT32 IdleBusClockRate = 0;  /* currently disabled by default */
module_param(IdleBusClockRate, int, 0644);
MODULE_PARM_DESC(IdleBusClockRate, "STD Host idle clock rate when in 4-bit mode");

static UINT32 CardDetectDebounce = SD_SLOT_DEBOUNCE_MS;  /* default to 1 second */
module_param(CardDetectDebounce, int, 0644);
MODULE_PARM_DESC(CardDetectDebounce, "STD Host card detect debounce interval (MS)");


typedef struct _STDHCD_DEV {
    SDLIST       CoreList;           /* the list of core contexts */
    spinlock_t   CoreListLock;       /* protection for the list */  
}STDHCD_DEV, *PSTDHCD_DEV;

STDHCD_DEV StdDevices;

void  InitStdHostLib()
{
    ZERO_POBJECT(&StdDevices);
    SDLIST_INIT(&StdDevices.CoreList);
    spin_lock_init(&StdDevices.CoreListLock); 
}

void  DeinitStdHostLib()
{
    
    
}

PSDHCD_CORE_CONTEXT CreateStdHostCore(PVOID pBusContext)
{
    PSDHCD_CORE_CONTEXT pStdCore = NULL;
    
    do {
        pStdCore = KernelAlloc(sizeof(SDHCD_CORE_CONTEXT));  
        if (NULL == pStdCore) {
            break;    
        }
        ZERO_POBJECT(pStdCore);
        SDLIST_INIT(&pStdCore->SlotList);
        spin_lock_init(&pStdCore->SlotListLock); 
        pStdCore->pBusContext = pBusContext;
        
            /* add it */
        spin_lock_irq(&StdDevices.CoreListLock);               
        SDListInsertHead(&StdDevices.CoreList, &pStdCore->List); 
        spin_unlock_irq(&StdDevices.CoreListLock);     
    } while (FALSE);
    
    return pStdCore;
}

void DeleteStdHostCore(PSDHCD_CORE_CONTEXT pStdCore)
{   
    spin_lock_irq(&StdDevices.CoreListLock);   
        /* remove */            
    SDListRemove(&pStdCore->List);
    spin_unlock_irq(&StdDevices.CoreListLock);     
    
    KernelFree(pStdCore);
}

/* find the std core associated with this bus context */
PSDHCD_CORE_CONTEXT GetStdHostCore(PVOID pBusContext)
{
    PSDLIST             pListItem;
    PSDHCD_CORE_CONTEXT pStdCore = NULL;
    
    spin_lock_irq(&StdDevices.CoreListLock);    
    
    do {
        if (SDLIST_IS_EMPTY(&StdDevices.CoreList)) {
            break;    
        }        
          
        SDITERATE_OVER_LIST(&StdDevices.CoreList, pListItem) {
            pStdCore = CONTAINING_STRUCT(pListItem, SDHCD_CORE_CONTEXT, List);
            if (pStdCore->pBusContext == pBusContext) {
                    /* found it */
                break;   
            } 
            pStdCore = NULL; 
        }
        
    } while (FALSE);
    
    spin_unlock_irq(&StdDevices.CoreListLock);    
    return pStdCore;
}

/* create a standard host memory instance */
PSDHCD_INSTANCE CreateStdHcdInstance(POS_DEVICE pOSDevice, 
                                     UINT       SlotNumber, 
                                     PTEXT      pName)
{
    PSDHCD_INSTANCE pHcInstance = NULL;
    BOOL            success = FALSE;
    
    do {
            /* allocate an instance for this new device */
        pHcInstance =  (PSDHCD_INSTANCE)KernelAlloc(sizeof(SDHCD_INSTANCE));
        
        if (pHcInstance == NULL) {
            DBG_PRINT(SDDBG_ERROR, ("SDIO STD Host: CreateStdHcdInstance - no memory for instance\n"));
            break;
        }
        
        ZERO_POBJECT(pHcInstance);
        SET_SDIO_STACK_VERSION(&pHcInstance->Hcd);
        
        pHcInstance->OsSpecific.SlotNumber = SlotNumber;
        spin_lock_init(&pHcInstance->OsSpecific.Lock);
        spin_lock_init(&pHcInstance->OsSpecific.RegAccessLock);
            /* initialize work items */
        ATH_INIT_WORK(&(pHcInstance->OsSpecific.iocomplete_work), hcd_iocomplete_wqueue_handler, pHcInstance);
        ATH_INIT_WORK(&(pHcInstance->OsSpecific.carddetect_work), hcd_carddetect_wqueue_handler, pHcInstance);
        ATH_INIT_WORK(&(pHcInstance->OsSpecific.sdioirq_work), hcd_sdioirq_wqueue_handler, pHcInstance);
            /* allocate space for the name */ 
        pHcInstance->Hcd.pName = (PTEXT)KernelAlloc(strlen(pName)+1);
        if (NULL == pHcInstance->Hcd.pName) {
            break;    
        }        
        strcpy(pHcInstance->Hcd.pName,pName);
            /* set OS device for DMA allocations and mapping */
        pHcInstance->Hcd.pDevice = pOSDevice;
        pHcInstance->Hcd.Attributes = hcdattributes;
        pHcInstance->Hcd.MaxBlocksPerTrans = SDIO_SD_MAX_BLOCKS;
        pHcInstance->Hcd.pContext = pHcInstance;
        pHcInstance->Hcd.pRequest = HcdRequest;
        pHcInstance->Hcd.pConfigure = HcdConfig;
        pHcInstance->Hcd.pModule = THIS_MODULE;
        pHcInstance->BaseClock = BaseClock;
        pHcInstance->TimeOut = timeout;
        pHcInstance->ClockSpinLimit = ClockSpinLimit;
        pHcInstance->IdleBusClockRate = IdleBusClockRate;
        pHcInstance->CardDetectDebounceMS = CardDetectDebounce;
        success = TRUE;
    } while (FALSE);
    
    if (!success && (pHcInstance != NULL)) {
        DeleteStdHcdInstance(pHcInstance);
    }
    
    return pHcInstance;
}

/*
 * AddStdHcdInstance - add the std host controller instance
*/
SDIO_STATUS AddStdHcdInstance(PSDHCD_CORE_CONTEXT pStdCore, 
                              PSDHCD_INSTANCE pHcInstance, 
                              UINT Flags,
                              PPLAT_OVERRIDE_CALLBACK pCallBack,                                
                              SDDMA_DESCRIPTION       *pSDMADescrip,
                              SDDMA_DESCRIPTION       *pADMADescrip)
{
    
    SDIO_STATUS status = SDIO_STATUS_SUCCESS;
    
    do { 
                
        if (!SDIO_SUCCESS((status = HcdInitialize(pHcInstance)))) {
            DBG_PRINT(SDDBG_ERROR, ("SDIO STD HOST: StartStdHcdInstance - failed to init HW, status =%d\n",status));  
            break;
        }    
            /* mark that the hardware was initialized */
        pHcInstance->InitStateMask |= SDHC_HW_INIT;
                  
        pHcInstance->Hcd.pDmaDescription = NULL;
        
        if (!(Flags & START_HCD_FLAGS_FORCE_NO_DMA)) {
                /* check DMA parameters discovered by HcdInitialize */
            if (!(Flags & START_HCD_FLAGS_FORCE_SDMA) && 
                  (pHcInstance->Caps & HOST_REG_CAPABILITIES_ADMA) &&
                  (pADMADescrip != NULL)) {
                DBG_PRINT(SDDBG_TRACE, ("SDIO STD HOST: StartStdHcdInstance - using Advanced DMA\n"));
                    /* copy the DMA description for advanced DMA */
                memcpy(&pHcInstance->DmaDescription, pADMADescrip, sizeof(SDDMA_DESCRIPTION));
                    /* set DMA description */
                pHcInstance->Hcd.pDmaDescription = &pHcInstance->DmaDescription;
            } else if ((pHcInstance->Caps & HOST_REG_CAPABILITIES_DMA) &&
                       (pSDMADescrip != NULL)) {
                DBG_PRINT(SDDBG_TRACE, ("SDIO STD HOST: StartStdHcdInstance - using Simple DMA\n"));
                    /* copy the DMA description for advanced DMA */
                memcpy(&pHcInstance->DmaDescription, pSDMADescrip, sizeof(SDDMA_DESCRIPTION));
                    /* set DMA description */
                pHcInstance->Hcd.pDmaDescription = &pHcInstance->DmaDescription;
            }
        }
        
        if (IS_HCD_ADMA(pHcInstance)) {
                /* setup DMA buffers for scatter-gather descriptor tables used in advanced DMA */
            status = SetupDmaBuffers(pHcInstance);
            if (!SDIO_SUCCESS(status)) {
                DBG_PRINT(SDDBG_ERROR, ("SDIO STD Host : StartStdHcdInstance - failed to setup DMA buffer\n"));
                break;
            }
        }  
        
        if (pCallBack != NULL) {
                /* allow the platform to override any settings */
            status = pCallBack(pHcInstance);
            if (!SDIO_SUCCESS(status)) {
                break;    
            }
        }             
            /* add this instance to our list, we will start the HCDs later */
            /* protect the devicelist */
        spin_lock_irq(&pStdCore->SlotListLock);               
        SDListInsertHead(&pStdCore->SlotList, &pHcInstance->List); 
        pStdCore->SlotCount++;
        spin_unlock_irq(&pStdCore->SlotListLock);     
        
    } while (FALSE);
        
    
    if (SDIO_SUCCESS(status)) {
        DBG_PRINT(SDDBG_ERROR, ("SDIO STD Host - ready! \n"));
    } else {
            /* undo everything */
        DeinitializeStdHcdInstance(pHcInstance);
    }
    
    return status;  
}

INT GetCurrentHcdInstanceCount(PSDHCD_CORE_CONTEXT pStdCore)
{
    return pStdCore->SlotCount;   
}

static void DeinitializeStdHcdInstance(PSDHCD_INSTANCE pHcInstance)
{
        /* wait for any of our work items to run */
    flush_scheduled_work();  
    
    if (pHcInstance->InitStateMask & SDHC_REGISTERED) {
        SDIO_UnregisterHostController(&pHcInstance->Hcd);
        pHcInstance->InitStateMask &= ~SDHC_REGISTERED;
    }      
    
    if (pHcInstance->InitStateMask & SDHC_HW_INIT) {
        HcdDeinitialize(pHcInstance);
        pHcInstance->InitStateMask &= ~SDHC_HW_INIT;
    }        
        /* free any DMA resources */
    if (pHcInstance->OsSpecific.hDmaBuffer != (DMA_ADDRESS)NULL) {
        dma_free_coherent(pHcInstance->Hcd.pDevice, 
                          SDHCD_ADMA_DESCRIPTOR_SIZE,
                          pHcInstance->OsSpecific.pDmaBuffer,
                          pHcInstance->OsSpecific.hDmaBuffer);
        pHcInstance->OsSpecific.hDmaBuffer = (DMA_ADDRESS)NULL;
        pHcInstance->OsSpecific.pDmaBuffer = NULL;
    }     
    
}
void DeleteStdHcdInstance(PSDHCD_INSTANCE pHcInstance)
{
    if (pHcInstance->Hcd.pName != NULL) {
        KernelFree(pHcInstance->Hcd.pName);
        pHcInstance->Hcd.pName = NULL;
    }            
        
    KernelFree(pHcInstance);    
}


/*
 * RemoveStdHcdInstance - remove the hcd instance
*/
PSDHCD_INSTANCE RemoveStdHcdInstance(PSDHCD_CORE_CONTEXT pStdCore)
{
    PSDHCD_INSTANCE pHcInstanceToRemove = NULL;
    PSDLIST pListItem;
    
    DBG_PRINT(SDDBG_TRACE, ("+SDIO STD HCD: RemoveStdHcdInstance\n"));
    
        /* protect the devicelist */
    spin_lock_irq(&pStdCore->SlotListLock);
    
    do {        
        pListItem = SDListRemoveItemFromHead(&pStdCore->SlotList);
        
        if (NULL == pListItem) {
            break;    
        }
    
        pHcInstanceToRemove = CONTAINING_STRUCT(pListItem,SDHCD_INSTANCE,List);
        
        pStdCore->SlotCount--;
    
    } while (FALSE);
    
    spin_unlock_irq(&pStdCore->SlotListLock);
    
    if (pHcInstanceToRemove != NULL) {
        DBG_PRINT(SDDBG_TRACE, (" SDIO STD HCD: Deinitializing 0x%X \n",(UINT)pHcInstanceToRemove));
        DeinitializeStdHcdInstance(pHcInstanceToRemove);   
    }           
           
    DBG_PRINT(SDDBG_TRACE, ("-SDIO STD HCD: RemoveStdHcdInstance\n"));
    
        /* return the instance we found */
    return pHcInstanceToRemove;
}

/*
 * SetupDmaBuffers - allocate required DMA buffers
 * 
*/
static SDIO_STATUS SetupDmaBuffers(PSDHCD_INSTANCE pHcInstance)
{
    if (pHcInstance->Hcd.pDmaDescription == NULL) {
        DBG_ASSERT(FALSE);
        return SDIO_STATUS_NO_RESOURCES;
    }    
    if (pHcInstance->Hcd.pDmaDescription->Flags & SDDMA_DESCRIPTION_FLAG_SGDMA) {
        /* we are only supporting scatter-gather DMA in this driver */
        /* allocate a DMA buffer large enough for the command buffers and the data buffers */
        pHcInstance->OsSpecific.pDmaBuffer =  dma_alloc_coherent(pHcInstance->Hcd.pDevice, 
                                                  SDHCD_ADMA_DESCRIPTOR_SIZE, 
                                                  &pHcInstance->OsSpecific.hDmaBuffer, 
                                                  GFP_DMA);
        DBG_PRINT(SDDBG_TRACE, ("SDIO STD Host : SetupDmaBuffers - pDmaBuffer: 0x%X, hDmaBuffer: 0x%X\n",
                                (UINT)pHcInstance->OsSpecific.pDmaBuffer , (UINT)pHcInstance->OsSpecific.hDmaBuffer ));
        if (pHcInstance->OsSpecific.pDmaBuffer == NULL) {
            DBG_PRINT(SDDBG_ERROR, ("SDIO STD Host : SetupDmaBuffers - unable to get DMA buffer\n"));
            return SDIO_STATUS_NO_RESOURCES;
        }        
        return SDIO_STATUS_SUCCESS;
    } else {
        DBG_PRINT(SDDBG_TRACE, ("SDIO STD Host : SetupDmaBuffers - invalid DMA type\n"));
        return SDIO_STATUS_INVALID_PARAMETER;
    }
    
}

/*
 * QueueEventResponse - queues an event in a process context back to the bus driver
 * 
*/
SDIO_STATUS QueueEventResponse(PSDHCD_INSTANCE pHcInstance, INT WorkItemID)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
     struct work_struct *work;
#else
    struct delayed_work *work;
#endif

    if (pHcInstance->ShuttingDown) {
        return SDIO_STATUS_CANCELED;
    }

    switch (WorkItemID) {
        case WORK_ITEM_IO_COMPLETE:
            work = &pHcInstance->OsSpecific.iocomplete_work;
            break;
        case WORK_ITEM_CARD_DETECT:
            work = &pHcInstance->OsSpecific.carddetect_work;
            break;
        case WORK_ITEM_SDIO_IRQ:
            work = &pHcInstance->OsSpecific.sdioirq_work;
            break;
        default:
            DBG_ASSERT(FALSE);
            return SDIO_STATUS_ERROR;
            break;  
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
    if (schedule_work(work) > 0)
#else
    if (schedule_delayed_work(work,0) > 0) 
#endif
    {
        if (WORK_ITEM_IO_COMPLETE == WorkItemID) {            
            pHcInstance->RequestCompleteQueued = TRUE;
        }
        return SDIO_STATUS_SUCCESS;
    } else {
        return SDIO_STATUS_PENDING;
    }
}

/*
 * hcd_iocomplete_wqueue_handler - the work queue for io completion
*/
static void hcd_iocomplete_wqueue_handler(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
void *context)
 {
     PSDHCD_INSTANCE pHcInstance = (PSDHCD_INSTANCE)context;
#else
struct work_struct *work)
{
    PSDHCD_INSTANCE pHcInstance =
      container_of( work, SDHCD_INSTANCE, OsSpecific.iocomplete_work.work );
#endif
 
    pHcInstance->RequestCompleteQueued = FALSE;
    
    if (!pHcInstance->ShuttingDown) {
        SDIO_HandleHcdEvent(&pHcInstance->Hcd, EVENT_HCD_TRANSFER_DONE);
    }
}

/*
 * hcd_carddetect_handler - the work queue for card detect debouncing
*/
static void hcd_carddetect_wqueue_handler(
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20)
struct work_struct *work)
{
    PSDHCD_INSTANCE context =
      container_of( work, SDHCD_INSTANCE, OsSpecific.carddetect_work.work );
#else
void *context)
 {
#endif

    ProcessDeferredCardDetect((PSDHCD_INSTANCE)context);
}

/*
 * hcd_sdioirq_handler - the work queue for handling SDIO IRQ
*/
static void hcd_sdioirq_wqueue_handler(
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
void *context)
 {
PSDHCD_INSTANCE pHcInstance = (PSDHCD_INSTANCE)context;
#else
struct work_struct *work)
{
    PSDHCD_INSTANCE pHcInstance  =
      container_of( work, SDHCD_INSTANCE, OsSpecific.sdioirq_work.work );
#endif

    DBG_PRINT(STD_HOST_TRACE_SDIO_INT, ("SDIO STD HOST: hcd_sdioirq_wqueue_handler \n"));
    if (!pHcInstance->ShuttingDown) {
        SDIO_HandleHcdEvent(&pHcInstance->Hcd, EVENT_HCD_SDIO_IRQ_PENDING);
    }
}




/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  UnmaskIrq - Unmask SD interrupts
  Input:    pHcInstance - host controller
            Mask - mask value
  Output: 
  Return: 
  Notes: 
        
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
UINT16 UnmaskIrq(PSDHCD_INSTANCE pHcInstance, UINT32 Mask, BOOL FromIsr)
{
    UINT16 ints;

    if (FromIsr) {
        
    } else {
        /* protected read-modify-write */
        spin_lock_irq(&pHcInstance->OsSpecific.RegAccessLock);
    }
    
    ints = READ_HOST_REG16(pHcInstance, HOST_REG_INT_SIGNAL_ENABLE);
    ints |= Mask;
    WRITE_HOST_REG16(pHcInstance, HOST_REG_INT_SIGNAL_ENABLE, ints);
    
    if (FromIsr) {
        
    } else {
        spin_unlock_irq(&pHcInstance->OsSpecific.RegAccessLock);
    }
    return ints;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  MaskIrq - Mask SD interrupts
  Input:    pHcInstance - host controller
            Mask - mask value
  Output: 
  Return: 
  Notes: 
        
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
UINT16 MaskIrq(PSDHCD_INSTANCE pHcInstance, UINT32 Mask, BOOL FromIsr)
{
    UINT16 ints;
    /* protected read-modify-write */
    if (FromIsr) {
    
    } else {
        spin_lock_irq(&pHcInstance->OsSpecific.RegAccessLock);
    }
    ints = READ_HOST_REG16(pHcInstance, HOST_REG_INT_SIGNAL_ENABLE);
    ints &= ~Mask;
    WRITE_HOST_REG16(pHcInstance, HOST_REG_INT_SIGNAL_ENABLE, ints);
    if (FromIsr) {
    
    } else {
        spin_unlock_irq(&pHcInstance->OsSpecific.RegAccessLock);
    }
    return ints;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EnableDisableSDIOIRQ - enable SDIO interrupt detection
  Input:    pHcInstance - host controller
            Enable - enable SDIO IRQ detection
            FromIsr - called from ISR
  Output: 
  Return: 
  Notes: 
        
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void EnableDisableSDIOIRQ(PSDHCD_INSTANCE pHcInstance, BOOL Enable, BOOL FromIsr)
{
    UINT16 intsEnables;
   
    if (FromIsr) {
        if (Enable) {
                // isr should never re-enable 
            DBG_ASSERT(FALSE);
        } else {
            MaskIrq(pHcInstance, HOST_REG_INT_STATUS_CARD_INT_STAT_ENABLE,TRUE);
        }
    } else {  
        if (Enable) { 
            UnmaskIrq(pHcInstance, HOST_REG_INT_STATUS_CARD_INT_STAT_ENABLE, FALSE); 
        } else {             
            MaskIrq(pHcInstance, HOST_REG_INT_STATUS_CARD_INT_STAT_ENABLE, FALSE);    
        }
    }           
     
    /* protected read-modify-write */   
    if (FromIsr) { 
        spin_lock(&pHcInstance->OsSpecific.RegAccessLock);
    } else {
        spin_lock_irq(&pHcInstance->OsSpecific.RegAccessLock);
    } 
        
    intsEnables = READ_HOST_REG16(pHcInstance, HOST_REG_INT_STATUS_ENABLE);
    if (Enable) {
        intsEnables |=  HOST_REG_INT_STATUS_CARD_INT_STAT_ENABLE;
    } else { 
        intsEnables &= ~HOST_REG_INT_STATUS_CARD_INT_STAT_ENABLE;
    }
        
    WRITE_HOST_REG16(pHcInstance, HOST_REG_INT_STATUS_ENABLE, intsEnables);   
    
    if (FromIsr) {
        spin_unlock(&pHcInstance->OsSpecific.RegAccessLock);
    } else {
        spin_unlock_irq(&pHcInstance->OsSpecific.RegAccessLock);    
    }
}


SDIO_STATUS SetUpHCDDMA(PSDHCD_INSTANCE pHcInstance, PSDREQUEST pReq)
{
    
    PSDDMA_DESCRIPTOR pReqDescriptor     = (PSDDMA_DESCRIPTOR)pReq->pDataBuffer;  
    UINT32 totalLength = 0;
    
    DBG_PRINT(STD_HOST_TRACE_DATA, ("SDIO STD HOST SetUpHCDDMA (%s) DescCount:%d Blocks:%d, BlockLen:%d\n",
        IS_SDREQ_WRITE_DATA(pReq->Flags) ?  "TX" : "RX",
        pReq->DescriptorCount, pReq->BlockCount, pReq->BlockLen));
    
    if (IS_HCD_ADMA(pHcInstance) && (pReq->DescriptorCount > SDHCD_MAX_ADMA_DESCRIPTOR)) {
        DBG_ASSERT(FALSE);
        return SDIO_STATUS_INVALID_PARAMETER;  
    } else if (IS_HCD_SDMA(pHcInstance) && (pReq->DescriptorCount > SDHCD_MAX_SDMA_DESCRIPTOR)) {
        DBG_ASSERT(FALSE);
        return SDIO_STATUS_INVALID_PARAMETER;  
    }  
    
         /* map this scatter gather entries to address and save for unmap */
    if (IS_SDREQ_WRITE_DATA(pReq->Flags)) {        
        dma_map_sg(pHcInstance->Hcd.pDevice, pReqDescriptor, pReq->DescriptorCount, DMA_TO_DEVICE);
    } else {
        dma_map_sg(pHcInstance->Hcd.pDevice, pReqDescriptor, pReq->DescriptorCount, DMA_FROM_DEVICE);
    }
     
    pHcInstance->OsSpecific.pDmaList = pReqDescriptor;   
    pHcInstance->OsSpecific.SGcount = pReq->DescriptorCount; 
    
    if (IS_HCD_ADMA(pHcInstance)) {
        int ii;
        PSDHCD_SGDMA_DESCRIPTOR  pDescriptor = 
            (PSDHCD_SGDMA_DESCRIPTOR)pHcInstance->OsSpecific.pDmaBuffer;

        DBG_ASSERT(pDescriptor != NULL);
            /* for ADMA build the in memory descriptor table */
        memset(pDescriptor, 0, pReq->DescriptorCount*(sizeof(SDHCD_SGDMA_DESCRIPTOR)));
        
        for (ii = 0; ii < pReq->DescriptorCount; ii++,pDescriptor++) {
            DBG_PRINT(STD_HOST_TRACE_DATA, ("SDIO STD HOST SetUpHCDDMA ADMA Descrp: 0x%X, ReqDescrip: 0x%X, len: %d bytes, addr: 0x%X\n",
             (UINT)pDescriptor, (UINT)&pReqDescriptor[ii], (UINT)sg_dma_len(&pReqDescriptor[ii]), (UINT)sg_dma_address(&pReqDescriptor[ii])));
            SET_DMA_LENGTH(pDescriptor, sg_dma_len(&pReqDescriptor[ii]));
            SET_DMA_ADDRESS(pDescriptor, sg_dma_address(&pReqDescriptor[ii]));
            totalLength += sg_dma_len(&pReqDescriptor[ii]);
            if (ii == (pReq->DescriptorCount-1)) {
                /* last entry, set END, 
                 ****note: we do NOT want an interrupt generated for this last descriptor,
                           the controller will generate interrupts indicating:
                           write CRC acknowledgement and program done -or-
                           read CRC okay */
                SET_DMA_END_OF_TRANSFER(pDescriptor);
            }
        }
    } else {
        DBG_PRINT(STD_HOST_TRACE_DATA, ("SDIO STD HOST SetUpHCDDMA SDMA, ReqDescrip: 0x%X, len: %d bytes, addr: 0x%X\n",
             (UINT)pReqDescriptor, (UINT)sg_dma_len(pReqDescriptor), (UINT)sg_dma_address(pReqDescriptor)));
            /* for simple DMA, setup DMA address */            
        WRITE_HOST_REG32(pHcInstance, HOST_REG_SYSTEM_ADDRESS, sg_dma_address(pReqDescriptor));
            /* since we only support 1 descriptor of up to 512KB size, we set the boundary up to 512KB 
               to prevent the DMA from stopping early , we let block count and length stop the DMA*/
        WRITE_HOST_REG16(pHcInstance, 
                         HOST_REG_BLOCK_SIZE, 
                         READ_HOST_REG16(pHcInstance,HOST_REG_BLOCK_SIZE) | HOST_REG_BLOCK_SIZE_DMA_512K_BOUNDARY);   
        totalLength = sg_dma_len(pReqDescriptor);
    }
    
    if (totalLength != (pReq->BlockCount * pReq->BlockLen)) {
        DBG_PRINT(SDDBG_ERROR, ("SDIO STD HOST DMA Block Length and Count Mismatch SGList Reports:%d bytes, Bus request : Blocks:%d,Bytes Per Block:%d \n",
        totalLength,pReq->BlockCount,pReq->BlockLen));
        return SDIO_STATUS_INVALID_PARAMETER;
    }
    
    if (IS_HCD_ADMA(pHcInstance)) {
            /* program the controller to execute the descriptor list */ 
        WRITE_HOST_REG32(pHcInstance, HOST_REG_ADMA_ADDRESS, (UINT32)pHcInstance->OsSpecific.hDmaBuffer);
            /* unprotect read-modify-write, set 32-bit DMA mode */
        WRITE_HOST_REG8(pHcInstance, HOST_REG_CONTROL,
            (READ_HOST_REG8(pHcInstance, HOST_REG_CONTROL) & ~HOST_REG_CONTROL_DMA_MASK) |
            HOST_REG_CONTROL_DMA_32BIT);
    }
            
    return SDIO_STATUS_PENDING;
}

/*
 * HcdTransferDataDMAEnd - cleanup bus master scatter-gather DMA read/write
*/
void HcdTransferDataDMAEnd(PSDHCD_INSTANCE pHcInstance, PSDREQUEST pReq)
{
    if (pHcInstance->OsSpecific.SGcount > 0) { 
        if (IS_SDREQ_WRITE_DATA(pReq->Flags)) {
            dma_unmap_sg(pHcInstance->Hcd.pDevice, 
                         pHcInstance->OsSpecific.pDmaList, 
                         pHcInstance->OsSpecific.SGcount, 
                         DMA_TO_DEVICE);
        } else {
            dma_unmap_sg(pHcInstance->Hcd.pDevice, 
                         pHcInstance->OsSpecific.pDmaList, 
                         pHcInstance->OsSpecific.SGcount, 
                         DMA_FROM_DEVICE);
        }
        pHcInstance->OsSpecific.SGcount = 0;
    }
}

void DumpDMADescriptorsInfo(PSDHCD_INSTANCE pHcInstance)
{
    if (IS_HCD_ADMA(pHcInstance)) {
        DBG_PRINT(SDDBG_ERROR, ("SDIO STD HOST, ADMA Descriptor Start (PHYS):0x%X \n",
                 (UINT32)pHcInstance->OsSpecific.hDmaBuffer));    
        SDLIB_PrintBuffer((PUCHAR)pHcInstance->OsSpecific.pDmaBuffer, SDHCD_ADMA_DESCRIPTOR_SIZE, "SDIO STD HOST: ALL DMA Descriptors");                           
    }
}

/* handle an interrupting standard host
 * this route checks the slot interrupting register and calls the interrupt routine
 * of the interrupting slot, the caller must the std core structure containing a 
 * list of hcd instances.  This function will use the first hcd instance to read the
 * slot interrupting register.
 * */
BOOL HandleSharedStdHostInterrupt(PSDHCD_CORE_CONTEXT pStdCore)
{
    PSDLIST         pListItem;
    UINT16          interruptingSlots;
    BOOL            handled = FALSE;
    UINT            slotIndex;
    PSDHCD_INSTANCE  pHcInstance;
    
    
        /* this is called at ISR priority, we do not need to protect the list */
    if (SDLIST_IS_EMPTY(&pStdCore->SlotList)) {
        return FALSE; 
    }        
        /* the first controller will do, the interrupt status register
         * is mapped to each slot controller */
    pListItem = SDLIST_GET_ITEM_AT_HEAD(&pStdCore->SlotList);

    pHcInstance = CONTAINING_STRUCT(pListItem, SDHCD_INSTANCE, List);
        
    interruptingSlots = READ_HOST_REG16(pHcInstance, HOST_REG_SLOT_INT_STATUS);
    interruptingSlots &= HOST_REG_SLOT_INT_MASK;
    
    if (0 == interruptingSlots) {
            /* not our interrupt */
        return FALSE;    
    }
    
    DBG_PRINT(STD_HOST_TRACE_INT, ("SDIO STD HOST HandleSharedStdHostInterrupt : slot ints:0x%X \n",interruptingSlots));
    
    slotIndex = 0;
     
    SDITERATE_OVER_LIST(&pStdCore->SlotList, pListItem) {
        pHcInstance = CONTAINING_STRUCT(pListItem, SDHCD_INSTANCE, List);
            /* is it interrupting ? */        
        if ((1 << pHcInstance->OsSpecific.SlotNumber) & interruptingSlots) {
            DBG_PRINT(STD_HOST_TRACE_INT, ("SDIO STD HOST HandleSharedStdHostInterrupt pHcInstance: 0x%X, slot:%d is interrupting \n",
                    (UINT)pHcInstance,pHcInstance->OsSpecific.SlotNumber));
   
                /* this one is interrupting.. */
            TRACE_SIGNAL_DATA_ISR(pHcInstance, TRUE);  
            if (HcdSDInterrupt(pHcInstance)) {
                    /* at least one handled it */
                handled = TRUE;    
            }
            TRACE_SIGNAL_DATA_ISR(pHcInstance, FALSE); 
        }   
    }
        
    return handled;
}

/* start the standard host instances 
 * this function registers the standard host drivers and queues an event to check the slots */
SDIO_STATUS StartStdHostCore(PSDHCD_CORE_CONTEXT pStdCore)
{
    SDIO_STATUS         status = SDIO_STATUS_SUCCESS;
    PSDHCD_INSTANCE     pHcInstance;
    PSDLIST             pListItem;
    INT                 coreStarts = 0;
    
    spin_lock_irq(&pStdCore->SlotListLock);
     
    do {
        
        if (SDLIST_IS_EMPTY(&pStdCore->SlotList)) {
            break;    
        }
         
        SDITERATE_OVER_LIST(&pStdCore->SlotList, pListItem) {
            
            pHcInstance = CONTAINING_STRUCT(pListItem, SDHCD_INSTANCE, List);
            
            spin_unlock_irq(&pStdCore->SlotListLock);
            
                /* register with the SDIO bus driver */
            status = SDIO_RegisterHostController(&pHcInstance->Hcd);
            
            spin_lock_irq(&pStdCore->SlotListLock);
            
            if (!SDIO_SUCCESS(status)) {
                DBG_PRINT(SDDBG_ERROR, ("SDIO STD Host - failed to register with host, status =%d\n",status));
                break;    
            }
            
            coreStarts++;
            
                /* mark that it has been registered */
            pHcInstance->InitStateMask |= SDHC_REGISTERED; 
    
                /* queue a work item to check for a card present at start up
                  this call will unmask the insert/remove interrupts */
            QueueEventResponse(pHcInstance, WORK_ITEM_CARD_DETECT);
        }
    
    } while (FALSE);
    
    spin_unlock_irq(&pStdCore->SlotListLock);
    
    if (0 == coreStarts) {
        return SDIO_STATUS_ERROR;    
    }
    
    return SDIO_STATUS_SUCCESS;
        
}
