/*
 * Intel XScale PXA255/270 processor support.
 *
 * Copyright (c) 2006 Openedhand Ltd.
 * Written by Andrzej Zaborowski <balrog@zabor.org>
 *
 * This code is licenced under the GPL.
 */
#ifndef PXA_H
# define PXA_H	1

# include "arm_pic.h"

/* Interrupt numbers */
# define PXA2XX_PIC_SSP3	0
# define PXA2XX_PIC_USBH2	2
# define PXA2XX_PIC_USBH1	3
# define PXA2XX_PIC_PWRI2C	6
# define PXA25X_PIC_HWUART	7
# define PXA27X_PIC_OST_4_11	7
# define PXA2XX_PIC_GPIO_0	8
# define PXA2XX_PIC_GPIO_1	9
# define PXA2XX_PIC_GPIO_X	10
# define PXA2XX_PIC_I2S 	13
# define PXA25X_PIC_NSSP	16
# define PXA27X_PIC_SSP2	16
# define PXA2XX_PIC_LCD		17
# define PXA2XX_PIC_I2C		18
# define PXA2XX_PIC_STUART	20
# define PXA2XX_PIC_BTUART	21
# define PXA2XX_PIC_FFUART	22
# define PXA2XX_PIC_MMC		23
# define PXA2XX_PIC_SSP		24
# define PXA2XX_PIC_DMA		25
# define PXA2XX_PIC_OST_0	26
# define PXA2XX_PIC_RTC1HZ	30
# define PXA2XX_PIC_RTCALARM	31

/* DMA requests */
# define PXA2XX_RX_RQ_I2S	2
# define PXA2XX_TX_RQ_I2S	3
# define PXA2XX_RX_RQ_BTUART	4
# define PXA2XX_TX_RQ_BTUART	5
# define PXA2XX_RX_RQ_FFUART	6
# define PXA2XX_TX_RQ_FFUART	7
# define PXA2XX_RX_RQ_SSP1	13
# define PXA2XX_TX_RQ_SSP1	14
# define PXA2XX_RX_RQ_SSP2	15
# define PXA2XX_TX_RQ_SSP2	16
# define PXA2XX_RX_RQ_STUART	19
# define PXA2XX_TX_RQ_STUART	20
# define PXA2XX_RX_RQ_MMCI	21
# define PXA2XX_TX_RQ_MMCI	22
# define PXA2XX_USB_RQ(x)	((x) + 24)
# define PXA2XX_RX_RQ_SSP3	66
# define PXA2XX_TX_RQ_SSP3	67

# define PXA2XX_RAM_BASE	0xa0000000

/* pxa2xx_pic.c */
struct pxa2xx_pic_state_s;
struct pxa2xx_pic_state_s *pxa2xx_pic_init(target_phys_addr_t base,
                CPUState *env, int parent_irq, int parent_fiq);

/* pxa2xx_timer.c */
void pxa25x_timer_init(target_phys_addr_t base,
                void *pic, int irq, CPUState *cpustate);
void pxa27x_timer_init(target_phys_addr_t base,
                void *pic, int irq, CPUState *cpustate);

/* pxa2xx_gpio.c */
struct pxa2xx_gpio_info_s;
struct pxa2xx_gpio_info_s *pxa2xx_gpio_init(target_phys_addr_t base,
                CPUState *env, void *pic, int lines);
void pxa2xx_gpio_set(struct pxa2xx_gpio_info_s *s, int line, int level);
void pxa2xx_gpio_handler_set(struct pxa2xx_gpio_info_s *s, int line,
                gpio_handler_t handler, void *opaque);
void pxa2xx_gpio_read_notifier(struct pxa2xx_gpio_info_s *s,
                void (*handler)(void *opaque), void *opaque);

/* pxa2xx_dma.c */
struct pxa2xx_dma_state_s;
struct pxa2xx_dma_state_s *pxa255_dma_init(target_phys_addr_t base,
                void *pic, int irq);
struct pxa2xx_dma_state_s *pxa27x_dma_init(target_phys_addr_t base,
                void *pic, int irq);
void pxa2xx_dma_request(struct pxa2xx_dma_state_s *s, int req_num, int on);

/* pxa2xx_lcd.c */
struct pxa2xx_lcdc_s;
struct pxa2xx_lcdc_s *pxa2xx_lcdc_init(target_phys_addr_t base,
                void *pic, DisplayState *ds);
void pxa2xx_lcd_vsync_cb(struct pxa2xx_lcdc_s *s,
                void (*cb)(void *opaque), void *opaque);
void pxa2xx_lcdc_oritentation(void *opaque, int angle);

/* pxa2xx_mmci.c */
struct pxa2xx_mmci_s;
struct pxa2xx_mmci_s *pxa2xx_mmci_init(target_phys_addr_t base,
                void *pic, void *dma);
void pxa2xx_mmci_handlers(struct pxa2xx_mmci_s *s, void *opaque,
                void (*readonly_cb)(void *, int),
                void (*coverswitch_cb)(void *, int));

/* pxa2xx_pcmcia.c */
struct pxa2xx_pcmcia_s;
struct pxa2xx_pcmcia_s *pxa2xx_pcmcia_init(target_phys_addr_t base);
int pxa2xx_pcmcia_attach(void *opaque, struct pcmcia_card_s *card);
int pxa2xx_pcmcia_dettach(void *opaque);
void pxa2xx_pcmcia_set_irq_cb(void *opaque, void (*set_irq)(void *opaque,
                int line, int level), int irq, int cd_irq, void *pic);

static struct {
    target_phys_addr_t io_base;
    int irq;
} pxa255_serial[] = {
    { 0x40100000, PXA2XX_PIC_FFUART },
    { 0x40200000, PXA2XX_PIC_BTUART },
    { 0x40700000, PXA2XX_PIC_STUART },
    { 0x41600000, PXA25X_PIC_HWUART },
    { 0, 0 }
}, pxa270_serial[] = {
    { 0x40100000, PXA2XX_PIC_FFUART },
    { 0x40200000, PXA2XX_PIC_BTUART },
    { 0x40700000, PXA2XX_PIC_STUART },
    { 0, 0 }
};

static struct {
    target_phys_addr_t io_base;
    int irq;
} pxa27x_ssp[] = {
    { 0x41000000, PXA2XX_PIC_SSP },
    { 0x41700000, PXA27X_PIC_SSP2 },
    { 0x41900000, PXA2XX_PIC_SSP3 },
    { 0, 0 }
};

/* The CPU is also modeled as an interrupt controller.  */
# define PXA2XX_PIC_CPU_IRQ 0
# define PXA2XX_PIC_CPU_FIQ 1

struct pxa2xx_ssp_s;
struct pxa2xx_i2c_s;
struct pxa2xx_i2s_s;

struct pxa2xx_state_s {
    CPUState *env;
    struct pxa2xx_pic_state_s *pic;
    struct pxa2xx_dma_state_s *dma;
    struct pxa2xx_gpio_info_s *gpio;
    struct pxa2xx_lcdc_s *lcd;
    struct pxa2xx_ssp_s **ssp;
    struct pxa2xx_mmci_s *mmc;
    struct pxa2xx_pcmcia_s *pcmcia[2];
    struct pxa2xx_i2c_s *i2c[2];
    struct pxa2xx_i2s_s *i2s;

    /* Power management */
    target_phys_addr_t pm_base;
    uint32_t pm_regs[0x40];

    /* Clock management */
    target_phys_addr_t cm_base;
    uint32_t cm_regs[4];
    uint32_t clkcfg;

    /* Memory management */
    target_phys_addr_t mm_base;
    uint32_t mm_regs[0x1a];

    /* Performance monitoring */
    uint32_t pmnc;

    /* Real-Time clock */
    target_phys_addr_t rtc_base;
    uint32_t rttr;
    uint32_t rtsr;
    uint32_t rtar;
    uint32_t rdar1;
    uint32_t rdar2;
    uint32_t ryar1;
    uint32_t ryar2;
    uint32_t swar1;
    uint32_t swar2;
    uint32_t piar;
    uint32_t last_rcnr;
    uint32_t last_rdcr;
    uint32_t last_rycr;
    uint32_t last_swcr;
    uint32_t last_rtcpicr;
    int64_t last_hz;
    int64_t last_sw;
    int64_t last_pi;
    QEMUTimer *rtc_hz;
    QEMUTimer *rtc_rdal1;
    QEMUTimer *rtc_rdal2;
    QEMUTimer *rtc_swal1;
    QEMUTimer *rtc_swal2;
    QEMUTimer *rtc_pi;
};

# define PMCR	0x00	/* Power Manager Control Register */
# define PSSR	0x04	/* Power Manager Sleep Status Register */
# define PSPR	0x08	/* Power Manager Scratch-Pad Register */
# define PWER	0x0c	/* Power Manager Wake-Up Enable Register */
# define PRER	0x10	/* Power Manager Rising-Edge Detect Enable Register */
# define PFER	0x14	/* Power Manager Falling-Edge Detect Enable Register */
# define PEDR	0x18	/* Power Manager Edge-Detect Status Register */
# define PCFR	0x1c	/* Power Manager General Configuration Register */
# define PGSR0	0x20	/* Power Manager GPIO Sleep-State Register 0 */
# define PGSR1	0x24	/* Power Manager GPIO Sleep-State Register 1 */
# define PGSR2	0x28	/* Power Manager GPIO Sleep-State Register 2 */
# define PGSR3	0x2c	/* Power Manager GPIO Sleep-State Register 3 */
# define RCSR	0x30	/* Reset Controller Status Register */
# define PSLR	0x34	/* Power Manager Sleep Configuration Register */
# define PTSR	0x38	/* Power Manager Standby Configuration Register */
# define PVCR	0x40	/* Power Manager Voltage Change Control Register */
# define PUCR	0x4c	/* Power Manager USIM Card Control/Status Register */
# define PKWR	0x50	/* Power Manager Keyboard Wake-Up Enable Register */
# define PKSR	0x54	/* Power Manager Keyboard Level-Detect Status */
# define PCMD0	0x80	/* Power Manager I2C Command Register File 0 */
# define PCMD31	0xfc	/* Power Manager I2C Command Register File 31 */

static uint32_t pxa2xx_i2c_read(void *, target_phys_addr_t);
static void pxa2xx_i2c_write(void *, target_phys_addr_t, uint32_t);

static uint32_t pxa2xx_pm_read(void *opaque, target_phys_addr_t addr)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    if (addr > s->pm_base + PCMD31) {
        /* Special case: PWRI2C registers appear in the same range.  */
        return pxa2xx_i2c_read(s->i2c[1], addr);
    }
    addr -= s->pm_base;

    switch (addr) {
    case PMCR ... PCMD31:
        if (addr & 3)
            goto fail;

        return s->pm_regs[addr >> 2];
    default:
    fail:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void pxa2xx_pm_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    if (addr > s->pm_base + PCMD31) {
        /* Special case: PWRI2C registers appear in the same range.  */
        pxa2xx_i2c_write(s->i2c[1], addr, value);
        return;
    }
    addr -= s->pm_base;

    switch (addr) {
    case PMCR:
        s->pm_regs[addr >> 2] &= 0x15 & ~(value & 0x2a);
        s->pm_regs[addr >> 2] |= value & 0x15;
        break;

    case PSSR:	/* Read-clean registers */
    case RCSR:
    case PKSR:
        s->pm_regs[addr >> 2] &= ~value;
        break;

    default:	/* Read-write registers */
        if (addr >= PMCR && addr <= PCMD31 && !(addr & 3)) {
            s->pm_regs[addr >> 2] = value;
            break;
        }

        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
}

static CPUReadMemoryFunc *pxa2xx_pm_readfn[] = {
    pxa2xx_pm_read,
    pxa2xx_pm_read,
    pxa2xx_pm_read,
};

static CPUWriteMemoryFunc *pxa2xx_pm_writefn[] = {
    pxa2xx_pm_write,
    pxa2xx_pm_write,
    pxa2xx_pm_write,
};

# define CCCR	0x00	/* Core Clock Configuration Register */
# define CKEN	0x04	/* Clock Enable Register */
# define OSCC	0x08	/* Oscillator Configuration Register */
# define CCSR	0x0c	/* Core Clock Status Register */

static uint32_t pxa2xx_cm_read(void *opaque, target_phys_addr_t addr)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    addr -= s->cm_base;

    switch (addr) {
    case CCCR:
    case CKEN:
    case OSCC:
        return s->cm_regs[addr >> 2];

    case CCSR:
        return s->cm_regs[CCCR >> 2] | (3 << 28);

    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void pxa2xx_cm_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    addr -= s->cm_base;

    switch (addr) {
    case CCCR:
    case CKEN:
        s->cm_regs[addr >> 2] = value;
        break;

    case OSCC:
        s->cm_regs[addr >> 2] &= ~0x6e;
        s->cm_regs[addr >> 2] |= value & 0x6e;
        break;

    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
}

static CPUReadMemoryFunc *pxa2xx_cm_readfn[] = {
    pxa2xx_cm_read,
    pxa2xx_cm_read,
    pxa2xx_cm_read,
};

static CPUWriteMemoryFunc *pxa2xx_cm_writefn[] = {
    pxa2xx_cm_write,
    pxa2xx_cm_write,
    pxa2xx_cm_write,
};

static uint32_t pxa2xx_clkpwr_read(void *opaque, int op2, int reg, int crm)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;

    switch (reg) {
    case 6:	/* Clock Configuration Register */
        return s->clkcfg;

    case 7:	/* Power Mode Register */
        return 0;

    default:
        printf("%s: Bad register 0x%x\n", __FUNCTION__, reg);
        break;
    }
    return 0;
}

static void pxa2xx_clkpwr_write(void *opaque, int op2, int reg, int crm,
                uint32_t value)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    static const char *pwrmode[8] = {
        "Normal", "Idle", "Deep-idle", "Standby",
        "Sleep", "reserved (!)", "reserved (!)", "Deep-sleep",
    };

    switch (reg) {
    case 6:	/* Clock Configuration Register */
        s->clkcfg = value & 0xf;
        if (value & 2)
            printf("%s: CPU frequency change attempt\n", __FUNCTION__);
        break;

    case 7:	/* Power Mode Register */
        if (value & 8)
            printf("%s: CPU voltage change attempt\n", __FUNCTION__);
        switch (value & 7) {
        case 0:
            /* Do nothing */
            break;

        case 1:
            /* Idle */
            if (!(s->cm_regs[CCCR] & (1 << 31))) {	/* CPDIS */
                cpu_interrupt(s->env, CPU_INTERRUPT_HALT);
                break;
            }
            /* Fall through.  */

        case 2:
            /* Deep-Idle */
            cpu_interrupt(s->env, CPU_INTERRUPT_HALT);
            s->pm_regs[RCSR >> 2] |= 0x8;	/* Set GPR */
            goto message;

        case 3:
            cpu_reset(s->env);
            s->env->cp15.c1_sys = 0;
            s->env->cp15.c1_coproc = 0;
            s->env->cp15.c2 = 0;
            s->env->cp15.c3 = 0;
            s->pm_regs[PSSR >> 2] |= 0x8;	/* Set STS */
            s->pm_regs[RCSR >> 2] |= 0x8;	/* Set GPR */

            /*
             * The scratch-pad register is almost universally used
             * for storing the return address on suspend.  For the
             * lack of a resuming bootloader, perform a jump
             * directly to that address.
             */
            memset(s->env->regs, 0, 4 * 15);
            s->env->regs[15] = s->pm_regs[PSPR >> 2];

#if 0
            buffer = 0xe59ff000;	/* ldr     pc, [pc, #0] */
            cpu_physical_memory_write(0, &buffer, 4);
            buffer = s->pm_regs[PSPR >> 2];
            cpu_physical_memory_write(8, &buffer, 4);
#endif

            /* Suspend */
            cpu_interrupt(cpu_single_env, CPU_INTERRUPT_HALT);

            goto message;

        default:
        message:
            printf("%s: machine entered %s mode\n", __FUNCTION__,
                            pwrmode[value & 7]);
        }
        break;

    default:
        printf("%s: Bad register 0x%x\n", __FUNCTION__, reg);
        break;
    }
}

/* Performace Monitoring registers */
# define CPPMNC		0	/* Performance Monitor Control Register */
# define CPCCNT		1	/* Clock Counter Register */
# define CPINTEN	4	/* Interrupt Enable Register */
# define CPFLAG		5	/* Overflow Flag Register */
# define CPEVTSEL	8	/* Event Selection Register */

# define CPPMN0		0	/* Performance Count Register 0 */
# define CPPMN1		1	/* Performance Count Register 1 */
# define CPPMN2		2	/* Performance Count Register 2 */
# define CPPMN3		3	/* Performance Count Register 3 */

static uint32_t pxa2xx_perf_read(void *opaque, int op2, int reg, int crm)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;

    switch (reg) {
    case CPPMNC:
        return s->pmnc;
    case CPCCNT:
        if (s->pmnc & 1)
            return qemu_get_clock(vm_clock);
        else
            return 0;
    case CPINTEN:
    case CPFLAG:
    case CPEVTSEL:
        return 0;

    default:
        printf("%s: Bad register 0x%x\n", __FUNCTION__, reg);
        break;
    }
    return 0;
}

static void pxa2xx_perf_write(void *opaque, int op2, int reg, int crm,
                uint32_t value)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;

    switch (reg) {
    case CPPMNC:
        s->pmnc = value;
        break;

    case CPCCNT:
    case CPINTEN:
    case CPFLAG:
    case CPEVTSEL:
        break;

    default:
        printf("%s: Bad register 0x%x\n", __FUNCTION__, reg);
        break;
    }
}

static uint32_t pxa2xx_cp14_read(void *opaque, int op2, int reg, int crm)
{
    switch (crm) {
    case 0:
        return pxa2xx_clkpwr_read(opaque, op2, reg, crm);
    case 1:
        return pxa2xx_perf_read(opaque, op2, reg, crm);
    case 2:
        switch (reg) {
        case CPPMN0:
        case CPPMN1:
        case CPPMN2:
        case CPPMN3:
            return 0;
        }
        /* Fall through */
    default:
        printf("%s: Bad register 0x%x\n", __FUNCTION__, reg);
        break;
    }
    return 0;
}

static void pxa2xx_cp14_write(void *opaque, int op2, int reg, int crm,
                uint32_t value)
{
    switch (crm) {
    case 0:
        pxa2xx_clkpwr_write(opaque, op2, reg, crm, value);
        break;
    case 1:
        pxa2xx_perf_write(opaque, op2, reg, crm, value);
        break;
    case 2:
        switch (reg) {
        case CPPMN0:
        case CPPMN1:
        case CPPMN2:
        case CPPMN3:
            return;
        }
        /* Fall through */
    default:
        printf("%s: Bad register 0x%x\n", __FUNCTION__, reg);
        break;
    }
}

# define MDCNFG		0x00	/* SDRAM Configuration Register */
# define MDREFR		0x04	/* SDRAM Refresh Control Register */
# define MSC0		0x08	/* Static Memory Control Register 0 */
# define MSC1		0x0c	/* Static Memory Control Register 1 */
# define MSC2		0x10	/* Static Memory Control Register 2 */
# define MECR		0x14	/* Expansion Memory Bus Config Register */
# define SXCNFG		0x1c	/* Synchronous Static Memory Config Register */
# define MCMEM0		0x28	/* PC Card Memory Socket 0 Timing Register */
# define MCMEM1		0x2c	/* PC Card Memory Socket 1 Timing Register */
# define MCATT0		0x30	/* PC Card Attribute Socket 0 Register */
# define MCATT1		0x34	/* PC Card Attribute Socket 1 Register */
# define MCIO0		0x38	/* PC Card I/O Socket 0 Timing Register */
# define MCIO1		0x3c	/* PC Card I/O Socket 1 Timing Register */
# define MDMRS		0x40	/* SDRAM Mode Register Set Config Register */
# define BOOT_DEF	0x44	/* Boot-time Default Configuration Register */
# define ARB_CNTL	0x48	/* Arbiter Control Register */
# define BSCNTR0	0x4c	/* Memory Buffer Strength Control Register 0 */
# define BSCNTR1	0x50	/* Memory Buffer Strength Control Register 1 */
# define LCDBSCNTR	0x54	/* LCD Buffer Strength Control Register */
# define MDMRSLP	0x58	/* Low Power SDRAM Mode Set Config Register */
# define BSCNTR2	0x5c	/* Memory Buffer Strength Control Register 2 */
# define BSCNTR3	0x60	/* Memory Buffer Strength Control Register 3 */
# define SA1110		0x64	/* SA-1110 Memory Compatibility Register */

static uint32_t pxa2xx_mm_read(void *opaque, target_phys_addr_t addr)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    addr -= s->mm_base;

    switch (addr) {
    case MDCNFG ... SA1110:
        if ((addr & 3) == 0)
            return s->mm_regs[addr >> 2];

    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void pxa2xx_mm_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    addr -= s->mm_base;

    switch (addr) {
    case MDCNFG ... SA1110:
        if ((addr & 3) == 0) {
            s->mm_regs[addr >> 2] = value;
            break;
        }

    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
}

static CPUReadMemoryFunc *pxa2xx_mm_readfn[] = {
    pxa2xx_mm_read,
    pxa2xx_mm_read,
    pxa2xx_mm_read,
};

static CPUWriteMemoryFunc *pxa2xx_mm_writefn[] = {
    pxa2xx_mm_write,
    pxa2xx_mm_write,
    pxa2xx_mm_write,
};

/* Synchronous Serial Ports */
struct pxa2xx_ssp_s {
    target_phys_addr_t base;
    int irq;
    struct pxa2xx_pic_state_s *pic;
    int enable;

    uint32_t sscr[2];
    uint32_t sspsp;
    uint32_t ssto;
    uint32_t ssitr;
    uint32_t sssr;
    uint8_t sstsa;
    uint8_t ssrsa;
    uint8_t ssacd;

    uint32_t rx_fifo[16];
    int rx_level;
    int rx_start;

    uint32_t (*readfn)(void *opaque);
    void (*writefn)(void *opaque, uint32_t value);
    void *opaque;
};

# define SSCR0	0x00	/* SSP Control Register 0 */
# define SSCR1	0x04	/* SSP Control Register 1 */
# define SSSR	0x08	/* SSP Status Register */
# define SSITR	0x0c	/* SSP Interrupt Test Register */
# define SSDR	0x10	/* SSP Data Register */
# define SSTO	0x28	/* SSP Time-Out Register */
# define SSPSP	0x2c	/* SSP Programmable Serial Protocol Register */
# define SSTSA	0x30	/* SSP TX Time Slot Active Register */
# define SSRSA	0x34	/* SSP RX Time Slot Active Register */
# define SSTSS	0x38	/* SSP Time Slot Status Register */
# define SSACD	0x3c	/* SSP Audio Clock Divider Register */

/* Bitfields for above registers */
# define SSCR0_SPI(x)	(((x) & 0x30) == 0x00)
# define SSCR0_SSP(x)	(((x) & 0x30) == 0x10)
# define SSCR0_UWIRE(x)	(((x) & 0x30) == 0x20)
# define SSCR0_PSP(x)	(((x) & 0x30) == 0x30)
# define SSCR0_SSE	(1 << 7)
# define SSCR0_RIM	(1 << 22)
# define SSCR0_TIM	(1 << 23)
# define SSCR0_MOD	(1 << 31)
# define SSCR0_DSS(x)	(((((x) >> 16) & 0x10) | ((x) & 0xf)) + 1)
# define SSCR1_RIE	(1 << 0)
# define SSCR1_TIE	(1 << 1)
# define SSCR1_LBM	(1 << 2)
# define SSCR1_MWDS	(1 << 5)
# define SSCR1_TFT(x)	((((x) >> 6) & 0xf) + 1)
# define SSCR1_RFT(x)	((((x) >> 10) & 0xf) + 1)
# define SSCR1_EFWR	(1 << 14)
# define SSCR1_PINTE	(1 << 18)
# define SSCR1_TINTE	(1 << 19)
# define SSCR1_RSRE	(1 << 20)
# define SSCR1_TSRE	(1 << 21)
# define SSCR1_EBCEI	(1 << 29)
# define SSITR_INT	(7 << 5)
# define SSSR_TNF	(1 << 2)
# define SSSR_RNE	(1 << 3)
# define SSSR_TFS	(1 << 5)
# define SSSR_RFS	(1 << 6)
# define SSSR_ROR	(1 << 7)
# define SSSR_PINT	(1 << 18)
# define SSSR_TINT	(1 << 19)
# define SSSR_EOC	(1 << 20)
# define SSSR_TUR	(1 << 21)
# define SSSR_BCE	(1 << 23)
# define SSSR_RW	0x00bc0080

static void pxa2xx_ssp_int_update(struct pxa2xx_ssp_s *s)
{
    int level = 0;

    level |= s->ssitr & SSITR_INT;
    level |= (s->sssr & SSSR_BCE)  &&  (s->sscr[1] & SSCR1_EBCEI);
    level |= (s->sssr & SSSR_TUR)  && !(s->sscr[0] & SSCR0_TIM);
    level |= (s->sssr & SSSR_EOC)  &&  (s->sssr & (SSSR_TINT | SSSR_PINT));
    level |= (s->sssr & SSSR_TINT) &&  (s->sscr[1] & SSCR1_TINTE);
    level |= (s->sssr & SSSR_PINT) &&  (s->sscr[1] & SSCR1_PINTE);
    level |= (s->sssr & SSSR_ROR)  && !(s->sscr[0] & SSCR0_RIM);
    level |= (s->sssr & SSSR_RFS)  &&  (s->sscr[1] & SSCR1_RIE);
    level |= (s->sssr & SSSR_TFS)  &&  (s->sscr[1] & SSCR1_TIE);
    pic_set_irq_new(s->pic, s->irq, !!level);
}

static void pxa2xx_ssp_fifo_update(struct pxa2xx_ssp_s *s)
{
    s->sssr &= ~(0xf << 12);	/* Clear RFL */
    s->sssr &= ~(0xf << 8);	/* Clear TFL */
    s->sssr &= ~SSSR_TNF;
    if (s->enable) {
        s->sssr |= ((s->rx_level - 1) & 0xf) << 12;
        if (s->rx_level >= SSCR1_RFT(s->sscr[1]))
            s->sssr |= SSSR_RFS;
        else
            s->sssr &= ~SSSR_RFS;
        if (0 <= SSCR1_TFT(s->sscr[1]))
            s->sssr |= SSSR_TFS;
        else
            s->sssr &= ~SSSR_TFS;
        if (s->rx_level)
            s->sssr |= SSSR_RNE;
        else
            s->sssr &= ~SSSR_RNE;
        s->sssr |= SSSR_TNF;
    }

    pxa2xx_ssp_int_update(s);
}

static uint32_t pxa2xx_ssp_read(void *opaque, target_phys_addr_t addr)
{
    struct pxa2xx_ssp_s *s = (struct pxa2xx_ssp_s *) opaque;
    uint32_t retval;
    addr -= s->base;

    switch (addr) {
    case SSCR0:
        return s->sscr[0];
    case SSCR1:
        return s->sscr[1];
    case SSPSP:
        return s->sspsp;
    case SSTO:
        return s->ssto;
    case SSITR:
        return s->ssitr;
    case SSSR:
        return s->sssr | s->ssitr;
    case SSDR:
        if (!s->enable)
            return 0xffffffff;
        if (s->rx_level < 1) {
            printf("%s: SSP Rx Underrun\n", __FUNCTION__);
            return 0xffffffff;
        }
        s->rx_level --;
        retval = s->rx_fifo[s->rx_start ++];
        s->rx_start &= 0xf;
        pxa2xx_ssp_fifo_update(s);
        return retval;
    case SSTSA:
        return s->sstsa;
    case SSRSA:
        return s->ssrsa;
    case SSTSS:
        return 0;
    case SSACD:
        return s->ssacd;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void pxa2xx_ssp_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct pxa2xx_ssp_s *s = (struct pxa2xx_ssp_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case SSCR0:
        s->sscr[0] = value & 0xc7ffffff;
        s->enable = value & SSCR0_SSE;
        if (value & SSCR0_MOD)
            printf("%s: Attempt to use network mode\n", __FUNCTION__);
        if (s->enable && SSCR0_DSS(value) < 4)
            printf("%s: Wrong data size: %i bits\n", __FUNCTION__,
                            SSCR0_DSS(value));
        if (!(value & SSCR0_SSE)) {
            s->sssr = 0;
            s->ssitr = 0;
            s->rx_level = 0;
        }
        pxa2xx_ssp_fifo_update(s);
        break;

    case SSCR1:
        s->sscr[1] = value;
        if (value & (SSCR1_LBM | SSCR1_EFWR))
            printf("%s: Attempt to use SSP test mode\n", __FUNCTION__);
        pxa2xx_ssp_fifo_update(s);
        break;

    case SSPSP:
        s->sspsp = value;
        break;

    case SSTO:
        s->ssto = value;
        break;

    case SSITR:
        s->ssitr = value & SSITR_INT;
        pxa2xx_ssp_int_update(s);
        break;

    case SSSR:
        s->sssr &= ~(value & SSSR_RW);
        pxa2xx_ssp_int_update(s);
        break;

    case SSDR:
        if (SSCR0_UWIRE(s->sscr[0])) {
            if (s->sscr[1] & SSCR1_MWDS)
                value &= 0xffff;
            else
                value &= 0xff;
        } else
            /* Note how 32bits overflow does no harm here */
            value &= (1 << SSCR0_DSS(s->sscr[0])) - 1;

        /* Data goes from here to the Tx FIFO and is shifted out from
         * there directly to the slave, no need to buffer it.
         */
        if (s->enable) {
            if (s->writefn)
                s->writefn(s->opaque, value);

            if (s->rx_level < 0x10) {
                if (s->readfn)
                    s->rx_fifo[(s->rx_start + s->rx_level ++) & 0xf] =
                            s->readfn(s->opaque);
                else
                    s->rx_fifo[(s->rx_start + s->rx_level ++) & 0xf] = 0x0;
            } else
                s->sssr |= SSSR_ROR;
        }
        pxa2xx_ssp_fifo_update(s);
        break;

    case SSTSA:
        s->sstsa = value;
        break;

    case SSRSA:
        s->ssrsa = value;
        break;

    case SSACD:
        s->ssacd = value;
        break;

    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
}

static inline void pxa2xx_ssp_attach(struct pxa2xx_ssp_s *port,
                uint32_t (*readfn)(void *opaque),
                void (*writefn)(void *opaque, uint32_t value), void *opaque)
{
    if (!port) {
        printf("%s: no such SSP\n", __FUNCTION__);
        exit(-1);
    }

    port->opaque = opaque;
    port->readfn = readfn;
    port->writefn = writefn;
}

static CPUReadMemoryFunc *pxa2xx_ssp_readfn[] = {
    pxa2xx_ssp_read,
    pxa2xx_ssp_read,
    pxa2xx_ssp_read,
};

static CPUWriteMemoryFunc *pxa2xx_ssp_writefn[] = {
    pxa2xx_ssp_write,
    pxa2xx_ssp_write,
    pxa2xx_ssp_write,
};

# define RCNR		0x00	/* RTC Counter Register */
# define RTAR		0x04	/* RTC Alarm Register */
# define RTSR		0x08	/* RTC Status Register */
# define RTTR		0x0c	/* RTC Timer Trim Register */
# define RDCR		0x10	/* RTC Day Counter Register */
# define RYCR		0x14	/* RTC Year Counter Register */
# define RDAR1		0x18	/* RTC Wristwatch Day Alarm Register 1 */
# define RYAR1		0x1c	/* RTC Wristwatch Year Alarm Register 1 */
# define RDAR2		0x20	/* RTC Wristwatch Day Alarm Register 2 */
# define RYAR2		0x24	/* RTC Wristwatch Year Alarm Register 2 */
# define SWCR		0x28	/* RTC Stopwatch Counter Register */
# define SWAR1		0x2c	/* RTC Stopwatch Alarm Register 1 */
# define SWAR2		0x30	/* RTC Stopwatch Alarm Register 2 */
# define RTCPICR	0x34	/* RTC Periodic Interrupt Counter Register */
# define PIAR		0x38	/* RTC Periodic Interrupt Alarm Register */

static inline void pxa2xx_rtc_int_update(struct pxa2xx_state_s *s)
{
    pic_set_irq_new(s->pic, PXA2XX_PIC_RTCALARM, !!(s->rtsr & 0x2553));
}

static void pxa2xx_rtc_hzupdate(struct pxa2xx_state_s *s)
{
    int64_t rt = qemu_get_clock(rt_clock);
    s->last_rcnr += ((rt - s->last_hz) << 15) /
            (1000 * ((s->rttr & 0xffff) + 1));
    s->last_rdcr += ((rt - s->last_hz) << 15) /
            (1000 * ((s->rttr & 0xffff) + 1));
    s->last_hz = rt;
}

static void pxa2xx_rtc_swupdate(struct pxa2xx_state_s *s)
{
    int64_t rt = qemu_get_clock(rt_clock);
    if (s->rtsr & (1 << 12))
        s->last_swcr += (rt - s->last_sw) / 10;
    s->last_sw = rt;
}

static void pxa2xx_rtc_piupdate(struct pxa2xx_state_s *s)
{
    int64_t rt = qemu_get_clock(rt_clock);
    if (s->rtsr & (1 << 15))
        s->last_swcr += rt - s->last_pi;
    s->last_pi = rt;
}

static inline void pxa2xx_rtc_alarm_update(struct pxa2xx_state_s *s,
                uint32_t rtsr)
{
    if ((rtsr & (1 << 2)) && !(rtsr & (1 << 0)))
        qemu_mod_timer(s->rtc_hz, s->last_hz +
                (((s->rtar - s->last_rcnr) * 1000 *
                  ((s->rttr & 0xffff) + 1)) >> 15));
    else
        qemu_del_timer(s->rtc_hz);

    if ((rtsr & (1 << 5)) && !(rtsr & (1 << 4)))
        qemu_mod_timer(s->rtc_rdal1, s->last_hz +
                (((s->rdar1 - s->last_rdcr) * 1000 *
                  ((s->rttr & 0xffff) + 1)) >> 15)); /* TODO: fixup */
    else
        qemu_del_timer(s->rtc_rdal1);

    if ((rtsr & (1 << 7)) && !(rtsr & (1 << 6)))
        qemu_mod_timer(s->rtc_rdal2, s->last_hz +
                (((s->rdar2 - s->last_rdcr) * 1000 *
                  ((s->rttr & 0xffff) + 1)) >> 15)); /* TODO: fixup */
    else
        qemu_del_timer(s->rtc_rdal2);

    if ((rtsr & 0x1200) == 0x1200 && !(rtsr & (1 << 8)))
        qemu_mod_timer(s->rtc_swal1, s->last_sw +
                        (s->swar1 - s->last_swcr) * 10); /* TODO: fixup */
    else
        qemu_del_timer(s->rtc_swal1);

    if ((rtsr & 0x1800) == 0x1800 && !(rtsr & (1 << 10)))
        qemu_mod_timer(s->rtc_swal2, s->last_sw +
                        (s->swar2 - s->last_swcr) * 10); /* TODO: fixup */
    else
        qemu_del_timer(s->rtc_swal2);

    if ((rtsr & 0xc000) == 0xc000 && !(rtsr & (1 << 13)))
        qemu_mod_timer(s->rtc_pi, s->last_pi +
                        (s->piar & 0xffff) - s->last_rtcpicr);
    else
        qemu_del_timer(s->rtc_pi);
}

static inline void pxa2xx_rtc_hz_tick(void *opaque)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    s->rtsr |= (1 << 0);
    pxa2xx_rtc_alarm_update(s, s->rtsr);
    pxa2xx_rtc_int_update(s);
}

static inline void pxa2xx_rtc_rdal1_tick(void *opaque)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    s->rtsr |= (1 << 4);
    pxa2xx_rtc_alarm_update(s, s->rtsr);
    pxa2xx_rtc_int_update(s);
}

static inline void pxa2xx_rtc_rdal2_tick(void *opaque)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    s->rtsr |= (1 << 6);
    pxa2xx_rtc_alarm_update(s, s->rtsr);
    pxa2xx_rtc_int_update(s);
}

static inline void pxa2xx_rtc_swal1_tick(void *opaque)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    s->rtsr |= (1 << 8);
    pxa2xx_rtc_alarm_update(s, s->rtsr);
    pxa2xx_rtc_int_update(s);
}

static inline void pxa2xx_rtc_swal2_tick(void *opaque)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    s->rtsr |= (1 << 10);
    pxa2xx_rtc_alarm_update(s, s->rtsr);
    pxa2xx_rtc_int_update(s);
}

static inline void pxa2xx_rtc_pi_tick(void *opaque)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    s->rtsr |= (1 << 13);
    pxa2xx_rtc_piupdate(s);
    s->last_rtcpicr = 0;
    pxa2xx_rtc_alarm_update(s, s->rtsr);
    pxa2xx_rtc_int_update(s);
}

static uint32_t pxa2xx_rtc_read(void *opaque, target_phys_addr_t addr)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    addr -= s->rtc_base;

    switch (addr) {
    case RTTR:
        return s->rttr;
    case RTSR:
        return s->rtsr;
    case RTAR:
        return s->rtar;
    case RDAR1:
        return s->rdar1;
    case RDAR2:
        return s->rdar2;
    case RYAR1:
        return s->ryar1;
    case RYAR2:
        return s->ryar2;
    case SWAR1:
        return s->swar1;
    case SWAR2:
        return s->swar2;
    case PIAR:
        return s->piar;
    case RCNR:
        return s->last_rcnr + ((qemu_get_clock(rt_clock) - s->last_hz) << 15) /
                (1000 * ((s->rttr & 0xffff) + 1));
    case RDCR:
        return s->last_rdcr + ((qemu_get_clock(rt_clock) - s->last_hz) << 15) /
                (1000 * ((s->rttr & 0xffff) + 1));
    case RYCR:
        return s->last_rycr;
    case SWCR:
        if (s->rtsr & (1 << 12))
            return s->last_swcr + (qemu_get_clock(rt_clock) - s->last_sw) / 10;
        else
            return s->last_swcr;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void pxa2xx_rtc_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    addr -= s->rtc_base;

    switch (addr) {
    case RTTR:
        if (!(s->rttr & (1 << 31))) {
            pxa2xx_rtc_hzupdate(s);
            s->rttr = value;
            pxa2xx_rtc_alarm_update(s, s->rtsr);
        }
        break;

    case RTSR:
        if ((s->rtsr ^ value) & (1 << 15))
            pxa2xx_rtc_piupdate(s);

        if ((s->rtsr ^ value) & (1 << 12))
            pxa2xx_rtc_swupdate(s);

        if (((s->rtsr ^ value) & 0x4aac) | (value & ~0xdaac))
            pxa2xx_rtc_alarm_update(s, value);

        s->rtsr = (value & 0xdaac) | (s->rtsr & ~(value & ~0xdaac));
        pxa2xx_rtc_int_update(s);
        break;

    case RTAR:
        s->rtar = value;
        pxa2xx_rtc_alarm_update(s, s->rtsr);
        break;

    case RDAR1:
        s->rdar1 = value;
        pxa2xx_rtc_alarm_update(s, s->rtsr);
        break;

    case RDAR2:
        s->rdar2 = value;
        pxa2xx_rtc_alarm_update(s, s->rtsr);
        break;

    case RYAR1:
        s->ryar1 = value;
        pxa2xx_rtc_alarm_update(s, s->rtsr);
        break;

    case RYAR2:
        s->ryar2 = value;
        pxa2xx_rtc_alarm_update(s, s->rtsr);
        break;

    case SWAR1:
        pxa2xx_rtc_swupdate(s);
        s->swar1 = value;
        s->last_swcr = 0;
        pxa2xx_rtc_alarm_update(s, s->rtsr);
        break;

    case SWAR2:
        s->swar2 = value;
        pxa2xx_rtc_alarm_update(s, s->rtsr);
        break;

    case PIAR:
        s->piar = value;
        pxa2xx_rtc_alarm_update(s, s->rtsr);
        break;

    case RCNR:
        pxa2xx_rtc_hzupdate(s);
        s->last_rcnr = value;
        pxa2xx_rtc_alarm_update(s, s->rtsr);
        break;

    case RDCR:
        pxa2xx_rtc_hzupdate(s);
        s->last_rdcr = value;
        pxa2xx_rtc_alarm_update(s, s->rtsr);
        break;

    case RYCR:
        s->last_rycr = value;
        break;

    case SWCR:
        pxa2xx_rtc_swupdate(s);
        s->last_swcr = value;
        pxa2xx_rtc_alarm_update(s, s->rtsr);
        break;

    case RTCPICR:
        pxa2xx_rtc_piupdate(s);
        s->last_rtcpicr = value & 0xffff;
        pxa2xx_rtc_alarm_update(s, s->rtsr);
        break;

    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static void pxa2xx_rtc_reset(struct pxa2xx_state_s *s)
{
    struct tm *tm;
    time_t ti;
    int wom;

    s->rttr = 0x7fff;
    s->rtsr = 0;

    time(&ti);
    if (rtc_utc)
        tm = gmtime(&ti);
    else
        tm = localtime(&ti);
    wom = ((tm->tm_mday - 1) / 7) + 1;

    s->last_rcnr = (uint32_t) ti;
    s->last_rdcr = (wom << 20) | ((tm->tm_wday + 1) << 17) |
            (tm->tm_hour << 12) | (tm->tm_min << 6) | tm->tm_sec;
    s->last_rycr = ((tm->tm_year + 1900) << 9) |
            ((tm->tm_mon + 1) << 5) | tm->tm_mday;
    s->last_swcr = (tm->tm_hour << 19) |
            (tm->tm_min << 13) | (tm->tm_sec << 7);
    s->last_rtcpicr = 0;
    s->last_hz = s->last_sw = s->last_pi = qemu_get_clock(rt_clock);

    s->rtc_hz    = qemu_new_timer(rt_clock, pxa2xx_rtc_hz_tick,    s);
    s->rtc_rdal1 = qemu_new_timer(rt_clock, pxa2xx_rtc_rdal1_tick, s);
    s->rtc_rdal2 = qemu_new_timer(rt_clock, pxa2xx_rtc_rdal2_tick, s);
    s->rtc_swal1 = qemu_new_timer(rt_clock, pxa2xx_rtc_swal1_tick, s);
    s->rtc_swal2 = qemu_new_timer(rt_clock, pxa2xx_rtc_swal2_tick, s);
    s->rtc_pi    = qemu_new_timer(rt_clock, pxa2xx_rtc_pi_tick,    s);
}

static CPUReadMemoryFunc *pxa2xx_rtc_readfn[] = {
    pxa2xx_rtc_read,
    pxa2xx_rtc_read,
    pxa2xx_rtc_read,
};

static CPUWriteMemoryFunc *pxa2xx_rtc_writefn[] = {
    pxa2xx_rtc_write,
    pxa2xx_rtc_write,
    pxa2xx_rtc_write,
};

/* I2C Interface */
struct pxa2xx_i2c_s {
    target_phys_addr_t base;
    int irq;
    struct pxa2xx_pic_state_s *pic;
    struct i2c_master_s master;
    struct i2c_slave_s slave;

    uint16_t control;
    uint16_t status;
    uint8_t ibmr;
};

# define IBMR	0x80	/* I2C Bus Monitor Register */
# define IDBR	0x88	/* I2C Data Buffer Register */
# define ICR	0x90	/* I2C Control Register */
# define ISR	0x98	/* I2C Status Register */
# define ISAR	0xa0	/* I2C Slave Address Register */

static void pxa2xx_i2c_update(struct pxa2xx_i2c_s *s)
{
    uint16_t level = 0;
    level |= s->status & s->control & (1 << 10);		/* BED */
    level |= (s->status & (1 << 7)) && (s->control & (1 << 9));	/* IRF */
    level |= (s->status & (1 << 6)) && (s->control & (1 << 8));	/* ITE */
    level |= s->status & (1 << 9);				/* SAD */
    pic_set_irq_new(s->pic, s->irq, !!level);
}

static void pxa2xx_i2c_start(void *opaque, int dir)
{
    struct pxa2xx_i2c_s *s = (struct pxa2xx_i2c_s *) opaque;
    s->status |= (1 << 9);				/* set SAD */
    if (dir)
        s->status |= 1 << 0;				/* set RWM */
    else
        s->status &= ~(1 << 0);				/* clear RWM */
}

static void pxa2xx_i2c_stop(void *opaque)
{
    struct pxa2xx_i2c_s *s = (struct pxa2xx_i2c_s *) opaque;
    s->status |= (1 << 4);				/* set SSD */
}

static int pxa2xx_i2c_tx(void *opaque, uint8_t *message, int len)
{
    struct pxa2xx_i2c_s *s = (struct pxa2xx_i2c_s *) opaque;
    if ((s->control & (1 << 14)) || !(s->control & (1 << 6)))
        return 1;

    if (len) {
        if (s->status & (1 << 0)) {			/* RWM */
            s->status |= 1 << 6;			/* set ITE */
            s->master.data = message[0];		/* TODO */
	} else {
            s->status |= 1 << 7;			/* set IRF */
            message[0] = s->master.data;		/* TODO */
        }
    }
    pxa2xx_i2c_update(s);

    return !s->master.ack;
}

static uint32_t pxa2xx_i2c_read(void *opaque, target_phys_addr_t addr)
{
    struct pxa2xx_i2c_s *s = (struct pxa2xx_i2c_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case ICR:
        return s->control;
    case ISR:
        return s->status;
    case ISAR:
        return s->slave.address;
    case IDBR:
        return s->master.data;
    case IBMR:
        if (s->status & (1 << 2))
            s->ibmr ^= 3;	/* Fake SCL and SDA pin changes */
        else
            s->ibmr = 0;
        return s->ibmr;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void pxa2xx_i2c_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct pxa2xx_i2c_s *s = (struct pxa2xx_i2c_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case ICR:
        s->control = value & 0xfff7;
        if ((value & (1 << 3)) && (value & (1 << 6))) {	/* TB and IUE */
            /* TODO: slave mode */
            if (value & (1 << 0)) {			/* START condition */
                if (s->master.data & 1)
                    s->status |= 1 << 0;		/* set RWM */
                else
                    s->status &= ~(1 << 0);		/* clear RWM */
                s->status |= 1 << 2;			/* set UB */
            }

            i2c_master_submit(&s->master,
                            value & (1 << 0),		/* START condition */
                            value & (1 << 1));		/* STOP condition */

            if (s->master.ack) {
                if (s->status & (1 << 0))		/* RWM */
                    s->status |= 1 << 7;		/* set IRF */
                else
                    s->status |= 1 << 6;		/* set ITE */
                s->status &= ~(1 << 1);			/* clear ACKNAK */
            } else {
                s->status |= 1 << 6;			/* set ITE */
                s->status |= 1 << 10;			/* set BED */
                s->status |= 1 << 1;			/* set ACKNAK */
                s->status &= ~(1 << 2);			/* clear UB */
            }
            if (value & (1 << 1))			/* STOP condition */
                s->status &= ~(1 << 2);			/* clear UB */
        }
        if (value & (1 << 4))				/* MA */
            s->status &= ~(1 << 2);			/* clear UB */
        if (!(value & (1 << 3)) && (value & (1 << 6)))	/* !TB and IUE */
            s->status &= ~(1 << 2);			/* clear UB */
        pxa2xx_i2c_update(s);
        break;

    case ISR:
        s->status &= ~(value & 0x07f0);
        pxa2xx_i2c_update(s);
        break;

    case ISAR:
        if (s->master.bus)
            i2c_slave_attach(s->master.bus, value & 0x7f, &s->slave);
        break;

    case IDBR:
        s->master.data = value & 0xff;
        break;

    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *pxa2xx_i2c_readfn[] = {
    pxa2xx_i2c_read,
    pxa2xx_i2c_read,
    pxa2xx_i2c_read,
};

static CPUWriteMemoryFunc *pxa2xx_i2c_writefn[] = {
    pxa2xx_i2c_write,
    pxa2xx_i2c_write,
    pxa2xx_i2c_write,
};

static struct pxa2xx_i2c_s *pxa2xx_i2c_init(target_phys_addr_t base, int irq,
                struct pxa2xx_pic_state_s *pic)
{
    int iomemtype;
    struct pxa2xx_i2c_s *s = (struct pxa2xx_i2c_s *)
            qemu_mallocz(sizeof(struct pxa2xx_i2c_s));

    s->base = base;
    s->irq = irq;
    s->pic = pic;
    s->slave.tx = pxa2xx_i2c_tx;
    s->slave.start = pxa2xx_i2c_start;
    s->slave.stop = pxa2xx_i2c_stop;
    s->slave.opaque = s;

    iomemtype = cpu_register_io_memory(0, pxa2xx_i2c_readfn,
                    pxa2xx_i2c_writefn, s);
    cpu_register_physical_memory(s->base & 0xfffff000, 0xfff, iomemtype);

    return s;
}

/* PXA Inter-IC Sound Controller */
struct pxa2xx_i2s_s {
    target_phys_addr_t base;
    int irq;
    struct pxa2xx_dma_state_s *dma;
    struct pxa2xx_pic_state_s *pic;
    void (*data_req)(void *, int, int);

    uint32_t control[2];
    uint32_t status;
    uint32_t mask;
    uint32_t clk;

    int enable;
    int rx_len;
    int tx_len;
    void (*codec_out)(void *, uint32_t);
    uint32_t (*codec_in)(void *);
    void *opaque;

    int fifo_len;
    uint32_t fifo[16];
};

static void pxa2xx_i2s_reset(struct pxa2xx_i2s_s *i2s)
{
    i2s->rx_len = 0;
    i2s->tx_len = 0;
    i2s->fifo_len = 0;
    i2s->clk = 0x1a;
    i2s->control[0] = 0x00;
    i2s->control[1] = 0x00;
    i2s->status = 0x00;
    i2s->mask = 0x00;
}

# define SACR_TFTH(val)	((val >> 8) & 0xf)
# define SACR_RFTH(val)	((val >> 12) & 0xf)
# define SACR_DREC(val)	(val & (1 << 3))
# define SACR_DPRL(val)	(val & (1 << 4))

static inline void pxa2xx_i2s_update(struct pxa2xx_i2s_s *i2s)
{
    int rfs, tfs;
    rfs = SACR_RFTH(i2s->control[0]) < i2s->rx_len &&
            !SACR_DREC(i2s->control[1]);
    tfs = (i2s->tx_len || i2s->fifo_len < SACR_TFTH(i2s->control[0])) &&
            i2s->enable && !SACR_DPRL(i2s->control[1]);

    pxa2xx_dma_request(i2s->dma, PXA2XX_RX_RQ_I2S, rfs);
    pxa2xx_dma_request(i2s->dma, PXA2XX_TX_RQ_I2S, tfs);

    i2s->status &= 0xe0;
    if (i2s->rx_len)
        i2s->status |= 1 << 1;			/* RNE */
    if (i2s->enable)
        i2s->status |= 1 << 2;			/* BSY */
    if (tfs)
        i2s->status |= 1 << 3;			/* TFS */
    if (rfs)
        i2s->status |= 1 << 4;			/* RFS */
    if (!(i2s->tx_len && i2s->enable))
        i2s->status |= i2s->fifo_len << 8;	/* TFL */
    i2s->status |= MAX(i2s->rx_len, 0xf) << 12;	/* RFL */

    pic_set_irq_new(i2s->pic, i2s->irq, i2s->status & i2s->mask);
}

# define SACR0	0x00	/* Serial Audio Global Control Register */
# define SACR1	0x04	/* Serial Audio I2S/MSB-Justified Control Register */
# define SASR0	0x0c	/* Serial Audio Interface and FIFO Status Register */
# define SAIMR	0x14	/* Serial Audio Interrupt Mask Register */
# define SAICR	0x18	/* Serial Audio Interrupt Clear Register */
# define SADIV	0x60	/* Serial Audio Clock Divider Register */
# define SADR	0x80	/* Serial Audio Data Register */

static uint32_t pxa2xx_i2s_read(void *opaque, target_phys_addr_t addr)
{
    struct pxa2xx_i2s_s *s = (struct pxa2xx_i2s_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case SACR0:
        return s->control[0];
    case SACR1:
        return s->control[1];
    case SASR0:
        return s->status;
    case SAIMR:
        return s->mask;
    case SAICR:
        return 0;
    case SADIV:
        return s->clk;
    case SADR:
        if (s->rx_len > 0) {
            s->rx_len --;
            pxa2xx_i2s_update(s);
            return s->codec_in(s->opaque);
        }
        return 0;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void pxa2xx_i2s_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct pxa2xx_i2s_s *s = (struct pxa2xx_i2s_s *) opaque;
    uint32_t *sample;
    addr -= s->base;

    switch (addr) {
    case SACR0:
        if (value & (1 << 3))				/* RST */
            pxa2xx_i2s_reset(s);
        s->control[0] = value & 0xff3d;
        if (!s->enable && (value & 1) && s->tx_len) {	/* ENB */
            for (sample = s->fifo; s->fifo_len > 0; s->fifo_len --, sample ++)
                s->codec_out(s->opaque, *sample);
            s->status &= ~(1 << 7);			/* I2SOFF */
        }
        if (value & (1 << 4))				/* EFWR */
            printf("%s: Attempt to use special function\n", __FUNCTION__);
        s->enable = ((value ^ 4) & 5) == 5;		/* ENB && !RST*/
        pxa2xx_i2s_update(s);
        break;
    case SACR1:
        s->control[1] = value & 0x0039;
        if (value & (1 << 5))				/* ENLBF */
            printf("%s: Attempt to use loopback function\n", __FUNCTION__);
        if (value & (1 << 4))				/* DPRL */
            s->fifo_len = 0;
        pxa2xx_i2s_update(s);
        break;
    case SAIMR:
        s->mask = value & 0x0078;
        pxa2xx_i2s_update(s);
        break;
    case SAICR:
        s->status &= ~(value & (3 << 5));
        pxa2xx_i2s_update(s);
        break;
    case SADIV:
        s->clk = value & 0x007f;
        break;
    case SADR:
        if (s->tx_len && s->enable) {
            s->tx_len --;
            pxa2xx_i2s_update(s);
            s->codec_out(s->opaque, value);
        } else if (s->fifo_len < 16) {
            s->fifo[s->fifo_len ++] = value;
            pxa2xx_i2s_update(s);
        }
        break;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *pxa2xx_i2s_readfn[] = {
    pxa2xx_i2s_read,
    pxa2xx_i2s_read,
    pxa2xx_i2s_read,
};

static CPUWriteMemoryFunc *pxa2xx_i2s_writefn[] = {
    pxa2xx_i2s_write,
    pxa2xx_i2s_write,
    pxa2xx_i2s_write,
};

static void pxa2xx_i2s_data_req(void *opaque, int tx, int rx)
{
    struct pxa2xx_i2s_s *s = (struct pxa2xx_i2s_s *) opaque;
    uint32_t *sample;

    /* Signal FIFO errors */
    if (s->enable && s->tx_len)
        s->status |= 1 << 5;		/* TUR */
    if (s->enable && s->rx_len)
        s->status |= 1 << 6;		/* ROR */

    /* Should be tx - MIN(tx, s->fifo_len) but we don't really need to
     * handle the cases where it makes a difference.  */
    s->tx_len = tx - s->fifo_len;
    s->rx_len = rx;
    /* Note that is s->codec_out wasn't set, we wouldn't get called.  */
    if (s->enable)
        for (sample = s->fifo; s->fifo_len; s->fifo_len --, sample ++)
            s->codec_out(s->opaque, *sample);
    pxa2xx_i2s_update(s);
}

static struct pxa2xx_i2s_s *pxa2xx_i2s_init(target_phys_addr_t base, int irq,
                struct pxa2xx_pic_state_s *pic, struct pxa2xx_dma_state_s *dma)
{
    int iomemtype;
    struct pxa2xx_i2s_s *s = (struct pxa2xx_i2s_s *)
            qemu_mallocz(sizeof(struct pxa2xx_i2s_s));

    s->base = base;
    s->irq = irq;
    s->pic = pic;
    s->dma = dma;
    s->data_req = pxa2xx_i2s_data_req;

    pxa2xx_i2s_reset(s);

    iomemtype = cpu_register_io_memory(0, pxa2xx_i2s_readfn,
                    pxa2xx_i2s_writefn, s);
    cpu_register_physical_memory(s->base & 0xfff00000, 0xfffff, iomemtype);

    return s;
}

static void pxa2xx_reset(int line, int level, void *opaque)
{
    struct pxa2xx_state_s *s = (struct pxa2xx_state_s *) opaque;
    if (level && (s->pm_regs[PCFR >> 2] & 0x10)) {	/* GPR_EN */
        cpu_reset(s->env);
        /* TODO: reset peripherals */
    }
}

/* Initialise a PXA270 integrated chip (ARM based core).  */
static inline struct pxa2xx_state_s *pxa270_init(DisplayState *ds,
                int revision)
{
    struct pxa2xx_state_s *s;
    struct pxa2xx_ssp_s *ssp;
    int iomemtype, i;
    s = (struct pxa2xx_state_s *) qemu_mallocz(sizeof(struct pxa2xx_state_s));

    s->env = cpu_init();
    cpu_arm_set_model(s->env, ARM_CPUID_PXA270 | revision);

    s->pic = pxa2xx_pic_init(0x40d00000, s->env,
                    PXA2XX_PIC_CPU_IRQ, PXA2XX_PIC_CPU_FIQ);

    s->dma = pxa27x_dma_init(0x40000000, s->pic, PXA2XX_PIC_DMA);

    pxa27x_timer_init(0x40a00000, s->pic, PXA2XX_PIC_OST_0, s->env);

    s->gpio = pxa2xx_gpio_init(0x40e00000, s->env, s->pic, 121);

    s->mmc = pxa2xx_mmci_init(0x41100000, s->pic, s->dma);

    for (i = 0; pxa270_serial[i].io_base; i ++)
        if (serial_hds[i])
            serial_mm_init(*(arm_pic_handler *) s->pic, s->pic,
                            pxa270_serial[i].io_base, 2,
                            pxa270_serial[i].irq, serial_hds[i]);

    if (ds)
        s->lcd = pxa2xx_lcdc_init(0x44000000, s->pic, ds);

    s->cm_base = 0x41300000;
    s->cm_regs[CCCR >> 4] = 0x02000210;	/* 416.0 MHz */
    s->clkcfg = 0x00000009;		/* Turbo mode active */
    iomemtype = cpu_register_io_memory(0, pxa2xx_cm_readfn,
                    pxa2xx_cm_writefn, s);
    cpu_register_physical_memory(s->cm_base, 0xfff, iomemtype);

    cpu_arm_set_cp_io(s->env, 14, pxa2xx_cp14_read, pxa2xx_cp14_write, s);

    s->mm_base = 0x48000000;
    s->mm_regs[MDMRS >> 2] = 0x00020002;
    s->mm_regs[MDREFR >> 2] = 0x03ca4000;
    s->mm_regs[MECR >> 2] = 0x00000001;	/* Two PC Card sockets */
    iomemtype = cpu_register_io_memory(0, pxa2xx_mm_readfn,
                    pxa2xx_mm_writefn, s);
    cpu_register_physical_memory(s->mm_base, 0xfff, iomemtype);

    for (i = 0; pxa27x_ssp[i].io_base; i ++);
    s->ssp = (struct pxa2xx_ssp_s **)
            qemu_mallocz(sizeof(struct pxa2xx_ssp_s *) * i);
    ssp = (struct pxa2xx_ssp_s *)
            qemu_mallocz(sizeof(struct pxa2xx_ssp_s) * i);
    for (i = 0; pxa27x_ssp[i].io_base; i ++) {
        s->ssp[i] = &ssp[i];
        ssp[i].base = pxa27x_ssp[i].io_base;
        ssp[i].irq = pxa27x_ssp[i].irq;
        ssp[i].pic = s->pic;

        iomemtype = cpu_register_io_memory(0, pxa2xx_ssp_readfn,
                        pxa2xx_ssp_writefn, &ssp[i]);
        cpu_register_physical_memory(ssp[i].base, 0xfff, iomemtype);
    }

    if (usb_enabled) {
        usb_ohci_init_memio(0x4c000000, 3, -1, s->pic, PXA2XX_PIC_USBH1);
    }

    s->pcmcia[0] = pxa2xx_pcmcia_init(0x20000000);
    s->pcmcia[1] = pxa2xx_pcmcia_init(0x30000000);

    s->rtc_base = 0x40900000;
    iomemtype = cpu_register_io_memory(0, pxa2xx_rtc_readfn,
                    pxa2xx_rtc_writefn, s);
    cpu_register_physical_memory(s->rtc_base, 0xfff, iomemtype);
    pxa2xx_rtc_reset(s);

    s->i2c[0] = pxa2xx_i2c_init(0x40301600, PXA2XX_PIC_I2C, s->pic);
    s->i2c[1] = pxa2xx_i2c_init(0x40f00100, PXA2XX_PIC_PWRI2C, s->pic);

    /* Register PM after PWRI2C to get the register mappings right.  */
    s->pm_base = 0x40f00000;
    iomemtype = cpu_register_io_memory(0, pxa2xx_pm_readfn,
                    pxa2xx_pm_writefn, s);
    cpu_register_physical_memory(s->pm_base, 0xfff, iomemtype);

    s->i2s = pxa2xx_i2s_init(0x40400000, PXA2XX_PIC_I2S, s->pic, s->dma);

    /* GPIO1 resets the processor */
    /* The handler can be overriden by board-specific code */
    pxa2xx_gpio_handler_set(s->gpio, 1, pxa2xx_reset, s);
    return s;
}

#endif	/* PXA_H */
