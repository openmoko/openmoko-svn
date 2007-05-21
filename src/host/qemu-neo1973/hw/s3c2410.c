/*
 * Samsung S3C2410A RISC Microprocessor support (ARM920T based SoC).
 *
 * Copyright (c) 2007 OpenMoko, Inc.
 * Author: Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This code is licenced under the GNU GPL v2.
 */
#include "vl.h"

/* Interrupt controller */
struct s3c_pic_state_s {
    target_phys_addr_t base;
    qemu_irq *parent_pic;
    qemu_irq *irqs;

    uint32_t srcpnd;
    uint32_t intpnd;
    uint32_t intmsk;
    uint32_t intmod;
    uint32_t priority;
    int intoffset;
    uint32_t subsrcpnd;
    uint32_t intsubmsk;
};

static void s3c_pic_update(struct s3c_pic_state_s *s)
{
    qemu_set_irq(s->parent_pic[ARM_PIC_CPU_FIQ],
                    s->srcpnd & s->intmod);
    qemu_set_irq(s->parent_pic[ARM_PIC_CPU_IRQ],
                    s->intpnd & ~s->intmsk & ~s->intmod);
}

/*
 * Performs interrupt arbitration and notifies the CPU.
 *
 * Since it's a complex logic which cannot be relied on by the OS
 * anyway - first because real hardware doesn't do it accurately,
 * second because it only matters when interrupts occur at the
 * same time which normally can't be predicted - we use a simpler
 * version for non-debug runs.
 */
#ifdef DEBUG
static const uint32_t s3c_arbmsk[6] = {
    0x0000000f,
    0x000003f0,
    0x0000fc00,
    0x003f0000,
    0x0fc00000,
    0xf0000000,
};

# define S3C_ARB_SEL(i)		((s->priority >> (7 + (i << 1))) & 3)
# define S3C_ARB_MODE(i)	((s->priority >> i) & 1)
# define S3C_ARB_SEL_SET(i, v)	\
    s->priority &= ~(3 << (7 + (i << 1))); \
    s->priority |= v << (7 + (i << 1));

static void s3c_pic_arbitrate(struct s3c_pic_state_s *s)
{
    uint32_t pnd = s->srcpnd & ~s->intmsk & ~s->intmod;
    int offset, i, arb;
    if (s->intpnd || !pnd) {
        s3c_pic_update(s);
        return;
    }

    if (pnd & s3c_arbmsk[0]) {
        offset = 0;
        arb = 0;
    } else if (pnd & 0x0ffffff0) {
        i = S3C_ARB_SEL(6);
        i ^= i << 1;
        if (!(pnd & s3c_arbmsk[1 + (i & 3)]))
            if (!(pnd & s3c_arbmsk[1 + (++ i & 3)]))
                if (!(pnd & s3c_arbmsk[1 + (++ i & 3)]))
                    i ++;

        if (S3C_ARB_MODE(6))
            S3C_ARB_SEL_SET(6, ((i + 1) & 3));
        offset = (i & 3) * 6 + 4;
        if (pnd & (1 << offset))
            goto known_offset;
        else if (!(pnd & (0x1f << offset))) {
            offset += 5;
            goto known_offset;
        }
        offset ++;
        arb = (i & 3) + 1;
    } else {
        arb = 5;
        offset = 28;
    }

    pnd >>= offset;
    i = S3C_ARB_SEL(arb);
    i ^= i << 1;
    if (!(pnd & (1 << (i & 3))))
        if (!(pnd & (1 << (++ i & 3))))
            if (!(pnd & (1 << (++ i & 3))))
                i ++;

    if (S3C_ARB_MODE(arb))
        S3C_ARB_SEL_SET(arb, ((i + 1) & 3));
    offset += i & 3;
known_offset:
    s->intoffset = offset;
    s->intpnd = 1 << offset;
    s3c_pic_update(s);
}
#else
inline static void s3c_pic_arbitrate(struct s3c_pic_state_s *s)
{
    uint32_t pnd = s->srcpnd & ~s->intmsk & ~s->intmod;
    if (pnd && !s->intpnd)
        s->intpnd = 1 << (s->intoffset = ffs(pnd) - 1);
    s3c_pic_update(s);
}
#endif

static const int s3c_sub_src_map[] = {
    [S3C_PICS_RXD0 & 31] = S3C_PIC_UART0,
    [S3C_PICS_TXD0 & 31] = S3C_PIC_UART0,
    [S3C_PICS_ERR0 & 31] = S3C_PIC_UART0,
    [S3C_PICS_RXD1 & 31] = S3C_PIC_UART1,
    [S3C_PICS_TXD1 & 31] = S3C_PIC_UART1,
    [S3C_PICS_ERR1 & 31] = S3C_PIC_UART1,
    [S3C_PICS_RXD2 & 31] = S3C_PIC_UART2,
    [S3C_PICS_TXD2 & 31] = S3C_PIC_UART2,
    [S3C_PICS_ERR2 & 31] = S3C_PIC_UART2,
    [S3C_PICS_TC   & 31] = S3C_PIC_ADC,
    [S3C_PICS_ADC  & 31] = S3C_PIC_ADC,
};

static void s3c_pic_subupdate(struct s3c_pic_state_s *s)
{
    int next;
    const int *sub = &s3c_sub_src_map[-1];
    uint32_t pnd = s->subsrcpnd & ~s->intsubmsk;
    while ((next = ffs(pnd))) {
        sub += next;
        pnd >>= next;
        s->srcpnd |= 1 << *sub;
    }
    s3c_pic_arbitrate(s);
}

static void s3c_pic_set_irq(void *opaque, int irq, int req)
{
    struct s3c_pic_state_s *s = (struct s3c_pic_state_s *) opaque;
    uint32_t mask;
    /* This interrupt controller doesn't clear any request signals
     * or register bits automatically.  */
    if (!req)
        return;

    if (irq & 32) {
        irq &= 31;
        s->subsrcpnd |= 1 << irq;
        if (s->intsubmsk & (1 << irq))
            return;
        else
            irq = s3c_sub_src_map[irq];
    }
    s->srcpnd |= (mask = 1 << irq);

    /* A FIQ */
    if (s->intmod & mask)
        qemu_irq_raise(s->parent_pic[ARM_PIC_CPU_FIQ]);
    else if (!s->intpnd && !(s->intmsk & mask)) {
#ifdef DEBUG
        s3c_pic_arbitrate(s);
#else
        s->intpnd = mask;
        s->intoffset = irq;
        qemu_irq_raise(s->parent_pic[ARM_PIC_CPU_IRQ]);
#endif
    }
}

static void s3c_pic_reset(struct s3c_pic_state_s *s)
{
    s->srcpnd = 0;
    s->intpnd = 0;
    s->intmsk = 0xffffffff;
    s->intmod = 0;
    s->priority = 0x7f;
    s->intoffset = 0;
    s->subsrcpnd = 0;
    s->intsubmsk = 0x7ff;
    s3c_pic_update(s);
}

#define S3C_SRCPND	0x00	/* Source Pending register */
#define S3C_INTMOD	0x04	/* Source Mode register */
#define S3C_INTMSK	0x08	/* Interrupt Mask register */
#define S3C_PRIORITY	0x0c	/* Priority register */
#define S3C_INTPND	0x10	/* Interrupt Pending register */
#define S3C_INTOFFSET	0x14	/* Interrupt Offset register */
#define S3C_SUBSRCPND	0x18	/* Sub Source Pending register */
#define S3C_INTSUBMSK	0x1c	/* Interrupt Sub Mask register */

static uint32_t s3c_pic_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_pic_state_s *s = (struct s3c_pic_state_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case S3C_SRCPND:
        return s->srcpnd;
    case S3C_INTPND:
        return s->intpnd;
    case S3C_INTMSK:
        return s->intmsk;
    case S3C_INTMOD:
        return s->intmod;
    case S3C_PRIORITY:
        return s->priority;
    case S3C_INTOFFSET:
        return s->intoffset;
    case S3C_SUBSRCPND:
        return s->subsrcpnd;
    case S3C_INTSUBMSK:
        return s->intsubmsk;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_pic_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_pic_state_s *s = (struct s3c_pic_state_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case S3C_SRCPND:
        s->srcpnd &= ~value;
        if (value & s->intmod)
            s3c_pic_update(s);
        break;
    case S3C_INTPND:
        if (s->intpnd & value) {
            s->intpnd = 0;
            s->intoffset = 0;
            s3c_pic_arbitrate(s);
        }
        break;
    case S3C_INTMSK:
        s->intmsk = value;
        if (s->intpnd & value) {
            s->intpnd = 0;
            s->intoffset = 0;
        }
        s3c_pic_arbitrate(s);
        break;
    case S3C_INTMOD:
        s->intmod = value;
        break;
    case S3C_PRIORITY:
        s->priority = value;
        break;
    case S3C_SUBSRCPND:
        s->subsrcpnd &= ~value;
        break;
    case S3C_INTSUBMSK:
        s->intsubmsk = value;
        s3c_pic_subupdate(s);
        break;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_pic_readfn[] = {
    s3c_pic_read,
    s3c_pic_read,
    s3c_pic_read,
};

static CPUWriteMemoryFunc *s3c_pic_writefn[] = {
    s3c_pic_write,
    s3c_pic_write,
    s3c_pic_write,
};

static void s3c_pic_save(QEMUFile *f, void *opaque)
{
    struct s3c_pic_state_s *s = (struct s3c_pic_state_s *) opaque;
    qemu_put_be32s(f, &s->srcpnd);
    qemu_put_be32s(f, &s->intpnd);
    qemu_put_be32s(f, &s->intmsk);
    qemu_put_be32s(f, &s->intmod);
    qemu_put_be32s(f, &s->priority);
    qemu_put_be32s(f, &s->subsrcpnd);
    qemu_put_be32s(f, &s->intsubmsk);
    qemu_put_be32(f, s->intoffset);
}

static int s3c_pic_load(QEMUFile *f, void *opaque, int version_id)
{
    struct s3c_pic_state_s *s = (struct s3c_pic_state_s *) opaque;
    qemu_get_be32s(f, &s->srcpnd);
    qemu_get_be32s(f, &s->intpnd);
    qemu_get_be32s(f, &s->intmsk);
    qemu_get_be32s(f, &s->intmod);
    qemu_get_be32s(f, &s->priority);
    qemu_get_be32s(f, &s->subsrcpnd);
    qemu_get_be32s(f, &s->intsubmsk);
    s->intoffset = qemu_get_be32(f);
    s3c_pic_update(s);
    return 0;
}

struct s3c_pic_state_s *s3c_pic_init(target_phys_addr_t base,
                qemu_irq *arm_pic)
{
    int iomemtype;
    struct s3c_pic_state_s *s = (struct s3c_pic_state_s *)
            qemu_mallocz(sizeof(struct s3c_pic_state_s));

    s->base = base;
    s->parent_pic = arm_pic;
    s->irqs = qemu_allocate_irqs(s3c_pic_set_irq, s, S3C_PIC_MAX);

    s3c_pic_reset(s);

    iomemtype = cpu_register_io_memory(0, s3c_pic_readfn,
                    s3c_pic_writefn, s);
    cpu_register_physical_memory(s->base, 0xffffff, iomemtype);

    register_savevm("s3c24xx_pic", 0, 0, s3c_pic_save, s3c_pic_load, s);

    return s;
}

qemu_irq *s3c_pic_get(struct s3c_pic_state_s *s)
{
    return s->irqs;
}

/* Memory controller */
#define S3C_BWSCON	0x00	/* Bus Width & Wait Control register */
#define S3C_BANKCON0	0x04	/* Bank 0 Control register */
#define S3C_BANKCON1	0x08	/* Bank 1 Control register */
#define S3C_BANKCON2	0x0c	/* Bank 2 Control register */
#define S3C_BANKCON3	0x10	/* Bank 3 Control register */
#define S3C_BANKCON4	0x14	/* Bank 4 Control register */
#define S3C_BANKCON5	0x18	/* Bank 5 Control register */
#define S3C_BANKCON6	0x1c	/* Bank 6 Control register */
#define S3C_BANKCON7	0x20	/* Bank 7 Control register */
#define S3C_REFRESH	0x24	/* SDRAM Refresh Control register */
#define S3C_BANKSIZE	0x28	/* Flexible Bank Size register */
#define S3C_MRSRB6	0x2c	/* Bank 6 Mode Set register */
#define S3C_MRSRB7	0x30	/* Bank 6 Mode Set register */

static void s3c_mc_reset(struct s3c_state_s *s)
{
    s->mc_regs[S3C_BWSCON >> 2] = 0x0000000;
    s->mc_regs[S3C_BANKCON0 >> 2] = 0x0700;
    s->mc_regs[S3C_BANKCON1 >> 2] = 0x0700;
    s->mc_regs[S3C_BANKCON2 >> 2] = 0x0700;
    s->mc_regs[S3C_BANKCON3 >> 2] = 0x0700;
    s->mc_regs[S3C_BANKCON4 >> 2] = 0x0700;
    s->mc_regs[S3C_BANKCON5 >> 2] = 0x0700;
    s->mc_regs[S3C_BANKCON6 >> 2] = 0x18008;
    s->mc_regs[S3C_BANKCON7 >> 2] = 0x18008;
    s->mc_regs[S3C_REFRESH >> 2] = 0xac0000;
    s->mc_regs[S3C_BANKSIZE >> 2] = 0x2;
    s->mc_regs[S3C_MRSRB6 >> 2] = 0x00;
    s->mc_regs[S3C_MRSRB7 >> 2] = 0x00;
}

static uint32_t s3c_mc_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_state_s *s = (struct s3c_state_s *) opaque;
    addr -= s->mc_base;

    switch (addr >> 2) {
    case S3C_BWSCON ... S3C_MRSRB7:
        return s->mc_regs[addr >> 2];
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_mc_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_state_s *s = (struct s3c_state_s *) opaque;
    addr -= s->mc_base;

    switch (addr >> 2) {
    case S3C_BWSCON ... S3C_MRSRB7:
        s->mc_regs[addr >> 2] = value;
        break;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_mc_readfn[] = {
    s3c_mc_read,
    s3c_mc_read,
    s3c_mc_read,
};

static CPUWriteMemoryFunc *s3c_mc_writefn[] = {
    s3c_mc_write,
    s3c_mc_write,
    s3c_mc_write,
};

static void s3c_mc_save(QEMUFile *f, void *opaque)
{
    struct s3c_state_s *s = (struct s3c_state_s *) opaque;
    int i;
    for (i = 0; i < 13; i ++)
        qemu_put_be32s(f, &s->mc_regs[i]);
}

static int s3c_mc_load(QEMUFile *f, void *opaque, int version_id)
{
    struct s3c_state_s *s = (struct s3c_state_s *) opaque;
    int i;
    for (i = 0; i < 13; i ++)
        qemu_get_be32s(f, &s->mc_regs[i]);
    return 0;
}

/* NAND Flash controller */
#define S3C_NFCONF	0x00	/* NAND Flash Configuration register */
#define S3C_NFCMD	0x04	/* NAND Flash Command Set register */
#define S3C_NFADDR	0x08	/* NAND Flash Address Set register */
#define S3C_NFDATA	0x0c	/* NAND Flash Data register */
#define S3C_NFSTAT	0x10	/* NAND Flash Operation Status register */
#define S3C_NFECC	0x14	/* NAND Flash ECC register */

static void s3c_nand_reset(struct s3c_state_s *s)
{
    s->nfconf = 0;
    s->nfcmd = 0;
    s->nfaddr = 0;
    ecc_reset(&s->nfecc);
}

static uint32_t s3c_nand_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_state_s *s = (struct s3c_state_s *) opaque;
    int rb, shr = 0;
    if (!s->nand)
        return 0;
    addr -= s->nand_base;

    switch (addr) {
    case S3C_NFCONF:
        return s->nfconf;
    case S3C_NFCMD:
        return s->nfcmd;
    case S3C_NFADDR:
        return s->nfaddr;
    case S3C_NFDATA:
        if (s->nfconf & (1 << 15))
            return ecc_digest(&s->nfecc, nand_getio(s->nand));
        break;
    case S3C_NFSTAT:
        nand_getpins(s->nand, &rb);
        return rb;
    case S3C_NFECC + 2: shr += 8;
    case S3C_NFECC + 1: shr += 8;
    case S3C_NFECC:
#define ECC(shr, b, shl)	((s->nfecc.lp[b] << (shl - shr)) & (1 << shl))
        return (~(
            ECC(0, 1, 0) | ECC(0, 0, 1) | ECC(1, 1, 2) | ECC(1, 0, 3) |
            ECC(2, 1, 4) | ECC(2, 0, 5) | ECC(3, 1, 6) | ECC(3, 0, 7) |
            ECC(4, 1, 8) | ECC(4, 0, 9) | ECC(5, 1, 10) | ECC(5, 0, 11) |
            ECC(6, 1, 12) | ECC(6, 0, 13) | ECC(7, 1, 14) | ECC(7, 0, 15) |
            ECC(8, 1, 16) | ECC(8, 0, 17) | (s->nfecc.cp << 18)) >> shr) &
            0xff;
#undef ECC
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_nand_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_state_s *s = (struct s3c_state_s *) opaque;
    if (!s->nand)
        return;
    addr -= s->nand_base;

    switch (addr) {
    case S3C_NFCONF:
        s->nfconf = value & 0x9fff;
        if (value & (1 << 12))
            ecc_reset(&s->nfecc);
        break;
    case S3C_NFCMD:
        s->nfcmd = value & 0xff;
        if (s->nfconf & (1 << 15)) {
            nand_setpins(s->nand, 1, 0, (s->nfconf >> 11) & 1, s->nfwp, 0);
            nand_setio(s->nand, s->nfcmd);
            nand_setpins(s->nand, 0, 0, (s->nfconf >> 11) & 1, s->nfwp, 0);
        }
        break;
    case S3C_NFADDR:
        s->nfaddr = value & 0xff;
        if (s->nfconf & (1 << 15)) {
            nand_setpins(s->nand, 0, 1, (s->nfconf >> 11) & 1, s->nfwp, 0);
            nand_setio(s->nand, s->nfaddr);
            nand_setpins(s->nand, 0, 0, (s->nfconf >> 11) & 1, s->nfwp, 0);
        }
        break;
    case S3C_NFDATA:
        if (s->nfconf & (1 << 15))
            nand_setio(s->nand, ecc_digest(&s->nfecc, value & 0xff));
        break;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

void s3c_nand_register(struct s3c_state_s *s, struct nand_flash_s *chip)
{
    s->nand = chip;
}

void s3c_nand_setwp(struct s3c_state_s *s, int wp)
{
    s->nfwp = wp;
}

static CPUReadMemoryFunc *s3c_nand_readfn[] = {
    s3c_nand_read,
    s3c_nand_read,
    s3c_nand_read,
};

static CPUWriteMemoryFunc *s3c_nand_writefn[] = {
    s3c_nand_write,
    s3c_nand_write,
    s3c_nand_write,
};

static void s3c_nand_save(QEMUFile *f, void *opaque)
{
    struct s3c_state_s *s = (struct s3c_state_s *) opaque;
    qemu_put_be16s(f, &s->nfconf);
    qemu_put_8s(f, &s->nfcmd);
    qemu_put_8s(f, &s->nfaddr);
    qemu_put_be32(f, s->nfwp);
    ecc_put(f, &s->nfecc);
}

static int s3c_nand_load(QEMUFile *f, void *opaque, int version_id)
{
    struct s3c_state_s *s = (struct s3c_state_s *) opaque;
    qemu_get_be16s(f, &s->nfconf);
    qemu_get_8s(f, &s->nfcmd);
    qemu_get_8s(f, &s->nfaddr);
    s->nfwp = qemu_get_be32(f);
    ecc_get(f, &s->nfecc);
    return 0;
}

/* Clock & power management */
#define S3C_LOCKTIME	0x00	/* PLL Lock Time Count register */
#define S3C_MPLLCON	0x04	/* MPLL Configuration register */
#define S3C_UPLLCON	0x08	/* UPLL Configuration register */
#define S3C_CLKCON	0x0c	/* Clock Generator Control register */
#define S3C_CLKSLOW	0x10	/* Slow Clock Control register */
#define S3C_CLKDIVN	0x14	/* Clock Divider Control register */

static void s3c_clkpwr_reset(struct s3c_state_s *s)
{
    s->clkpwr_regs[S3C_LOCKTIME >> 2] = 0x00ffffff;
    s->clkpwr_regs[S3C_MPLLCON >> 2] = 0x0005c080;
    s->clkpwr_regs[S3C_UPLLCON >> 2] = 0x00028080;
    s->clkpwr_regs[S3C_CLKCON >> 2] = 0x0007fff0;
    s->clkpwr_regs[S3C_CLKSLOW >> 2] = 0x00000004;
    s->clkpwr_regs[S3C_CLKDIVN >> 2] = 0x00000000;
}

static uint32_t s3c_clkpwr_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_state_s *s = (struct s3c_state_s *) opaque;
    addr -= s->clkpwr_base;

    switch (addr) {
    case S3C_LOCKTIME ... S3C_CLKDIVN:
        return s->clkpwr_regs[addr >> 2];
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_clkpwr_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_state_s *s = (struct s3c_state_s *) opaque;
    addr -= s->clkpwr_base;

    switch (addr) {
    case S3C_LOCKTIME:
    case S3C_MPLLCON:
    case S3C_UPLLCON:
    case S3C_CLKDIVN:
        s->clkpwr_regs[addr >> 2] = value;
        break;
    case S3C_CLKCON:
        if (value & (1 << 3)) {
            cpu_interrupt(s->env, CPU_INTERRUPT_HALT);
            printf("%s: processor powered off\n", __FUNCTION__);
            s3c_gpio_setpwrstat(s->io, 2);
            cpu_reset(s->env);
            s->env->regs[15] = 0;	/* XXX */
        } else
            if (value & (1 << 2))	/* Normal IDLE mode */
                cpu_interrupt(s->env, CPU_INTERRUPT_HALT);
        if ((s->clkpwr_regs[addr >> 2] ^ value) & 1)
            printf("%s: SPECIAL mode %s\n", __FUNCTION__,
                            (value & 1) ? "on" : "off");
        s->clkpwr_regs[addr >> 2] = value;
        break;
    case S3C_CLKSLOW:
        if ((s->clkpwr_regs[addr >> 2] ^ value) & (1 << 4))
            printf("%s: SLOW mode %s\n", __FUNCTION__,
                            (value & (1 << 4)) ? "on" : "off");
        s->clkpwr_regs[addr >> 2] = value;
        break;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_clkpwr_readfn[] = {
    s3c_clkpwr_read,
    s3c_clkpwr_read,
    s3c_clkpwr_read,
};

static CPUWriteMemoryFunc *s3c_clkpwr_writefn[] = {
    s3c_clkpwr_write,
    s3c_clkpwr_write,
    s3c_clkpwr_write,
};

static void s3c_clkpwr_save(QEMUFile *f, void *opaque)
{
    struct s3c_state_s *s = (struct s3c_state_s *) opaque;
    int i;
    for (i = 0; i < 6; i ++)
        qemu_put_be32s(f, &s->clkpwr_regs[i]);
}

static int s3c_clkpwr_load(QEMUFile *f, void *opaque, int version_id)
{
    struct s3c_state_s *s = (struct s3c_state_s *) opaque;
    int i;
    for (i = 0; i < 6; i ++)
        qemu_get_be32s(f, &s->clkpwr_regs[i]);
    return 0;
}

/* DMA controller */
#define S3C_DMA_CH_N	4

struct s3c_dma_ch_state_s;
struct s3c_dma_state_s {	/* Modelled as an interrupt controller */
    target_phys_addr_t base;
    qemu_irq *drqs;
    struct s3c_dma_ch_state_s {
        qemu_irq intr;
        int curr_tc;
        int req;
        int running;
        uint32_t con;
        uint32_t isrc;
        uint32_t isrcc;
        uint32_t idst;
        uint32_t idstc;
        uint32_t csrc;
        uint32_t cdst;
        uint32_t mask;
    } ch[S3C_DMA_CH_N];
};

static void s3c_dma_ch_run(struct s3c_dma_state_s *s,
                struct s3c_dma_ch_state_s *ch)
{
    int width, burst, t;
    uint8_t buffer[4];
    width = 1 << ((ch->con >> 20) & 3);				/* DSZ */
    burst = (ch->con & (1 << 28)) ? 4 : 1;			/* TSZ */

    while (!ch->running && ch->req && (ch->mask & (1 << 1))) {	/* ON_OFF */
        if (width > sizeof(buffer)) {
            printf("%s: wrong access width\n", __FUNCTION__);
            return;
        }
        ch->curr_tc = ch->con & 0xfffff;			/* TC */
        ch->running = 1;
        while (ch->curr_tc --) {
            for (t = 0; t < burst; t ++) {
                cpu_physical_memory_read(ch->csrc, buffer, width);
                cpu_physical_memory_write(ch->cdst, buffer, width);

                if (!(ch->isrcc & 1))				/* INT */
                    ch->csrc += width;
                if (!(ch->idstc & 1))				/* INT */
                    ch->cdst += width;
            }

            if (!(ch->con & (1 << 27)))				/* SERVMODE */
                break;
        }
        ch->running = 0;
        if (!ch->curr_tc && (ch->con & (1 << 29)))		/* INT */
            qemu_irq_raise(ch->intr);

        if (ch->con & (1 << 22)) {				/* RELOAD */
            if (!(ch->con & (1 << 23))) {			/* SWHW_SEL */
                printf("%s: auto-reload software controlled transfer\n",
                                __FUNCTION__);
                break;
            }
            ch->csrc = ch->isrc;				/* S_ADDR */
            ch->cdst = ch->idst;				/* D_ADDR */
            ch->curr_tc = ch->con & 0xfffff;			/* TC */
        }
        if (!(ch->con & (1 << 23))) {				/* SWHW_SEL */
            ch->req = 0;
        }
        if (!ch->curr_tc && !(ch->con & (1 << 22)))		/* RELOAD */
            ch->mask &= ~(1 << 1);				/* ON_OFF */
    }
}

static void s3c_dma_reset(struct s3c_dma_state_s *s)
{
    int i;
    for (i = 0; i < S3C_DMA_CH_N; i ++) {
        s->ch[i].curr_tc = 0;
        s->ch[i].csrc = 0;
        s->ch[i].isrc = 0;
        s->ch[i].isrcc = 0;
        s->ch[i].cdst = 0;
        s->ch[i].idst = 0;
        s->ch[i].idstc = 0;
        s->ch[i].con = 0;
        s->ch[i].csrc = 0;
        s->ch[i].cdst = 0;
        s->ch[i].mask = 0;
    }
}

static void s3c_dma_dreq(void *opaque, int line, int req)
{
    struct s3c_dma_state_s *s = (struct s3c_dma_state_s *) opaque;
    struct s3c_dma_ch_state_s *ch = &s->ch[line >> 4];

    if (ch->con & (1 << 23))					/* SWHW_SEL */
        if (((ch->con >> 24) & 7) == (line & 7)) {		/* HWSRCSEL */
            ch->req = req;
            s3c_dma_ch_run(s, ch);
        }
}

#define S3C_DISRC	0x00	/* DMA Initial Source register */
#define S3C_DISRCC	0x04	/* DMA Initial Source Control register */
#define S3C_DIDST	0x08	/* DMA Initial Destination register */
#define S3C_DIDSTC	0x0c	/* DMA Initial Destination Control register */
#define S3C_DCON	0x10	/* DMA Control register */
#define S3C_DSTAT	0x14	/* DMA Count register */
#define S3C_DCSRC	0x18	/* DMA Current Source register */
#define S3C_DCDST	0x1c	/* DMA Current Destination register */
#define S3C_DMASKTRIG	0x20	/* DMA Mask Trigger register */

static uint32_t s3c_dma_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_dma_state_s *s = (struct s3c_dma_state_s *) opaque;
    struct s3c_dma_ch_state_s *ch = 0;
    addr -= s->base;
    if (addr >= 0 && addr <= (S3C_DMA_CH_N << 6)) {
        ch = &s->ch[addr >> 6];
        addr &= 0x3f;
    }

    switch (addr) {
    case S3C_DISRC:
        return ch->isrc;
    case S3C_DISRCC:
        return ch->isrcc;
    case S3C_DIDST:
        return ch->idst;
    case S3C_DIDSTC:
        return ch->idstc;
    case S3C_DCON:
        return ch->con;
    case S3C_DSTAT:
        return ch->curr_tc;
    case S3C_DCSRC:
        return ch->csrc;
    case S3C_DCDST:
        return ch->cdst;
    case S3C_DMASKTRIG:
        return ch->mask;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_dma_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_dma_state_s *s = (struct s3c_dma_state_s *) opaque;
    struct s3c_dma_ch_state_s *ch = 0;
    addr -= s->base;
    if (addr >= 0 && addr <= (S3C_DMA_CH_N << 6)) {
        ch = &s->ch[addr >> 6];
        addr &= 0x3f;
    }

    switch (addr) {
    case S3C_DCON:
        ch->con = value;
        break;
    case S3C_DISRC:
        ch->csrc = ch->isrc = value;
        break;
    case S3C_DISRCC:
        ch->isrcc = value;
        break;
    case S3C_DIDST:
        ch->cdst = ch->idst = value;
        break;
    case S3C_DIDSTC:
        ch->idstc = value;
        break;
    case S3C_DMASKTRIG:
        ch->mask = value;
        if (value & (1 << 2)) {					/* STOP */
            ch->curr_tc = 0;
            ch->mask &= ~(1 << 1);				/* ON_OFF */
        } else if (!(ch->con & (1 << 23))) {			/* SWHW_SEL */
            ch->req = value & 1;				/* SW_TRIG */
            s3c_dma_ch_run(s, ch);
        }
        break;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_dma_readfn[] = {
    s3c_dma_read,
    s3c_dma_read,
    s3c_dma_read,
};

static CPUWriteMemoryFunc *s3c_dma_writefn[] = {
    s3c_dma_write,
    s3c_dma_write,
    s3c_dma_write,
};

static void s3c_dma_save(QEMUFile *f, void *opaque)
{
    struct s3c_dma_state_s *s = (struct s3c_dma_state_s *) opaque;
    int i;
    for (i = 0; i < S3C_DMA_CH_N; i ++) {
        qemu_put_be32(f, s->ch[i].curr_tc);
        qemu_put_be32(f, s->ch[i].req);
        qemu_put_be32s(f, &s->ch[i].con);
        qemu_put_be32s(f, &s->ch[i].isrc);
        qemu_put_be32s(f, &s->ch[i].isrcc);
        qemu_put_be32s(f, &s->ch[i].idst);
        qemu_put_be32s(f, &s->ch[i].idstc);
        qemu_put_be32s(f, &s->ch[i].csrc);
        qemu_put_be32s(f, &s->ch[i].cdst);
        qemu_put_be32s(f, &s->ch[i].mask);
    }
}

static int s3c_dma_load(QEMUFile *f, void *opaque, int version_id)
{
    struct s3c_dma_state_s *s = (struct s3c_dma_state_s *) opaque;
    int i;
    for (i = 0; i < S3C_DMA_CH_N; i ++) {
        s->ch[i].curr_tc = qemu_get_be32(f);
        s->ch[i].req = qemu_get_be32(f);
        qemu_get_be32s(f, &s->ch[i].con);
        qemu_get_be32s(f, &s->ch[i].isrc);
        qemu_get_be32s(f, &s->ch[i].isrcc);
        qemu_get_be32s(f, &s->ch[i].idst);
        qemu_get_be32s(f, &s->ch[i].idstc);
        qemu_get_be32s(f, &s->ch[i].csrc);
        qemu_get_be32s(f, &s->ch[i].cdst);
        qemu_get_be32s(f, &s->ch[i].mask);
    }
    return 0;
}

struct s3c_dma_state_s *s3c_dma_init(target_phys_addr_t base, qemu_irq *pic)
{
    int iomemtype;
    struct s3c_dma_state_s *s = (struct s3c_dma_state_s *)
            qemu_mallocz(sizeof(struct s3c_dma_state_s));

    s->base = base;
    s->ch[0].intr = pic[0];
    s->ch[1].intr = pic[1];
    s->ch[2].intr = pic[2];
    s->ch[3].intr = pic[3];
    s->drqs = qemu_allocate_irqs(s3c_dma_dreq, s, S3C_RQ_MAX);

    s3c_dma_reset(s);

    iomemtype = cpu_register_io_memory(0, s3c_dma_readfn,
                    s3c_dma_writefn, s);
    cpu_register_physical_memory(s->base, 0xffffff, iomemtype);

    register_savevm("s3c24xx_dma", 0, 0, s3c_dma_save, s3c_dma_load, s);

    return s;
}

qemu_irq *s3c_dma_get(struct s3c_dma_state_s *s)
{
    return s->drqs;
}

/* PWM timers controller */
struct s3c_timer_state_s;
struct s3c_timers_state_s {
    target_phys_addr_t base;
    qemu_irq *dma;
    DisplayState *ds;
    struct s3c_timer_state_s {
        QEMUTimer *t;
        struct s3c_timers_state_s *s;
        int n;
        int running;
        uint32_t divider;
        uint16_t count;
        int64_t reload;
        qemu_irq irq;
        gpio_handler_t cmp_cb;
        void *cmp_opaque;
    } timer[5];

    uint16_t compareb[4];
    uint16_t countb[5];
    uint32_t config[2];
    uint32_t control;
};

static const int s3c_tm_bits[] = { 0, 8, 12, 16, 20 };

static uint16_t s3c_timers_get(struct s3c_timers_state_s *s, int tm)
{
    uint16_t elapsed;
    if (!s->timer[tm].running)
        return s->timer[tm].count;

    elapsed = muldiv64(qemu_get_clock(vm_clock) - s->timer[tm].reload,
                    s->timer[tm].divider, ticks_per_sec);
    if (unlikely(elapsed > s->timer[tm].count))
        return s->timer[tm].count;

    return s->timer[tm].count - elapsed;
}

static void s3c_timers_stop(struct s3c_timers_state_s *s, int tm)
{
    s->timer[tm].count = s3c_timers_get(s, tm);
    s->timer[tm].running = 0;
}

static void s3c_timers_start(struct s3c_timers_state_s *s, int tm)
{
    if (s->timer[tm].running)
        return;

    s->timer[tm].divider = S3C_PCLK_FREQ >>
            (((s->config[1] >> (tm * 4)) & 3) + 1);
    if (tm < 2)
        s->timer[tm].divider /= ((s->config[0] >> 0) & 0xff) + 1;
    else
        s->timer[tm].divider /= ((s->config[0] >> 8) & 0xff) + 1;
    s->timer[tm].running = 1;
    s->timer[tm].reload = qemu_get_clock(vm_clock);
    qemu_mod_timer(s->timer[tm].t,
                    s->timer[tm].reload + muldiv64(s->timer[tm].count,
                            ticks_per_sec, s->timer[tm].divider));
}

static void s3c_timers_reset(struct s3c_timers_state_s *s)
{
    int i;
    s->config[0] = 0x00000000;
    s->config[1] = 0x00000000;
    s->control = 0x00000000;

    for (i = 0; i < 5; i ++) {
        if (s->timer[i].running)
            s3c_timers_stop(s, i);
        s->countb[i] = 0x0000;
        s->timer[i].count = 0;
    }
    for (i = 0; i < 4; i ++)
        s->compareb[i] = 0x0000;
}

static void s3c_timers_tick(void *opaque)
{
    struct s3c_timer_state_s *t = (struct s3c_timer_state_s *) opaque;
    struct s3c_timers_state_s *s = t->s;
    if (!t->running)
        return;

    if (((s->config[1] >> 20) & 0xf) == t->n + 1) {
        qemu_irq_raise(s->dma[S3C_RQ_TIMER0]);	/* TODO */
        qemu_irq_raise(s->dma[S3C_RQ_TIMER1]);
        qemu_irq_raise(s->dma[S3C_RQ_TIMER2]);
    } else
        qemu_irq_raise(t->irq);

    t->running = 0;
    t->count = 0;

    if (s->control & (1 << ((t->n == 4) ? 22 : (s3c_tm_bits[t->n] + 3)))) {
        /* Auto-reload */
        t->count = s->countb[t->n];
        s3c_timers_start(s, t->n);
    } else
        s->control &= ~(1 << s3c_tm_bits[t->n]);
}

#define S3C_TCFG0	0x00	/* Timer Configuration register 0 */
#define S3C_TCFG1	0x04	/* Timer Configuration register 1 */
#define S3C_TCON	0x08	/* Timer Control register */
#define S3C_TCNTB0	0x0c	/* Timer 0 Count Buffer register */
#define S3C_TCMPB0	0x10	/* Timer 0 Compare Buffer register */
#define S3C_TCNTO0	0x14	/* Timer 0 Count Observation register */
#define S3C_TCNTB1	0x18	/* Timer 1 Count Buffer register */
#define S3C_TCMPB1	0x1c	/* Timer 1 Compare Buffer register */
#define S3C_TCNTO1	0x20	/* Timer 1 Count Observation register */
#define S3C_TCNTB2	0x24	/* Timer 2 Count Buffer register */
#define S3C_TCMPB2	0x28	/* Timer 2 Compare Buffer register */
#define S3C_TCNTO2	0x2c	/* Timer 2 Count Observation register */
#define S3C_TCNTB3	0x30	/* Timer 3 Count Buffer register */
#define S3C_TCMPB3	0x34	/* Timer 3 Compare Buffer register */
#define S3C_TCNTO3	0x38	/* Timer 3 Count Observation register */
#define S3C_TCNTB4	0x3c	/* Timer 4 Count Buffer register */
#define S3C_TCNTO4	0x40	/* Timer 4 Count Observation register */

static uint32_t s3c_timers_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_timers_state_s *s = (struct s3c_timers_state_s *) opaque;
    int tm = 0;
    addr -= s->base;

    switch (addr) {
    case S3C_TCFG0:
        return s->config[0];
    case S3C_TCFG1:
        return s->config[1];
    case S3C_TCON:
        return s->control;
    case S3C_TCMPB3:    tm ++;
    case S3C_TCMPB2:    tm ++;
    case S3C_TCMPB1:    tm ++;
    case S3C_TCMPB0:
        return s->compareb[tm];
    case S3C_TCNTB4:    tm ++;
    case S3C_TCNTB3:    tm ++;
    case S3C_TCNTB2:    tm ++;
    case S3C_TCNTB1:    tm ++;
    case S3C_TCNTB0:
        return s->countb[tm];
    case S3C_TCNTO4:    tm ++;
    case S3C_TCNTO3:    tm ++;
    case S3C_TCNTO2:    tm ++;
    case S3C_TCNTO1:    tm ++;
    case S3C_TCNTO0:
        return s3c_timers_get(s, tm);
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_timers_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_timers_state_s *s = (struct s3c_timers_state_s *) opaque;
    int tm = 0;
    addr -= s->base;

    switch (addr) {
    case S3C_TCFG0:
        s->config[0] = value & 0x00ffffff;
        break;
    case S3C_TCFG1:
        s->config[1] = value & 0x00ffffff;
        break;
    case S3C_TCON:
        for (tm = 0; tm < 5; tm ++) {
            if (value & (2 << (s3c_tm_bits[tm]))) {
                if (s->timer[tm].running) {
                    s3c_timers_stop(s, tm);
                    s->timer[tm].count = s->countb[tm];
                    s3c_timers_start(s, tm);
                } else
                    s->timer[tm].count = s->countb[tm];
            }
            if (((value >> s3c_tm_bits[tm]) & 1) ^ s->timer[tm].running) {
                if (s->timer[tm].running)
                    s3c_timers_stop(s, tm);
                else
                    s3c_timers_start(s, tm);
            }
        }

        s->control = value & 0x007fff1f;
        break;
    case S3C_TCMPB3:    tm ++;
    case S3C_TCMPB2:    tm ++;
    case S3C_TCMPB1:    tm ++;
    case S3C_TCMPB0:
        s->compareb[tm] = value & 0xffff;
        if (s->timer[tm].cmp_cb)
            s->timer[tm].cmp_cb(tm, s->compareb[tm], s->timer[tm].cmp_opaque);
        break;
    case S3C_TCNTB4:    tm ++;
    case S3C_TCNTB3:    tm ++;
    case S3C_TCNTB2:    tm ++;
    case S3C_TCNTB1:    tm ++;
    case S3C_TCNTB0:
        s->countb[tm] = value & 0xffff;
        break;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_timers_readfn[] = {
    s3c_timers_read,
    s3c_timers_read,
    s3c_timers_read,
};

static CPUWriteMemoryFunc *s3c_timers_writefn[] = {
    s3c_timers_write,
    s3c_timers_write,
    s3c_timers_write,
};

static void s3c_timers_save(QEMUFile *f, void *opaque)
{
    struct s3c_timers_state_s *s = (struct s3c_timers_state_s *) opaque;
    int i;
    for (i = 0; i < 5; i ++) {
        qemu_put_be32(f, s->timer[i].running);
        s3c_timers_stop(s, i);
        qemu_put_be32s(f, &s->timer[i].divider);
        qemu_put_be16s(f, &s->timer[i].count);
        qemu_put_be64s(f, &s->timer[i].reload);
    }

    for (i = 0; i < 4; i ++)
        qemu_put_be16s(f, &s->compareb[i]);
    for (i = 0; i < 5; i ++)
        qemu_put_be16s(f, &s->countb[i]);
    for (i = 0; i < 2; i ++)
        qemu_put_be32s(f, &s->config[i]);
    qemu_put_be32s(f, &s->control);
}

static int s3c_timers_load(QEMUFile *f, void *opaque, int version_id)
{
    struct s3c_timers_state_s *s = (struct s3c_timers_state_s *) opaque;
    int i;
    for (i = 0; i < 5; i ++) {
        s->timer[i].running = qemu_get_be32(f);
        qemu_get_be32s(f, &s->timer[i].divider);
        qemu_get_be16s(f, &s->timer[i].count);
        qemu_get_be64s(f, &s->timer[i].reload);
    }

    for (i = 0; i < 4; i ++)
        qemu_get_be16s(f, &s->compareb[i]);
    for (i = 0; i < 5; i ++)
        qemu_get_be16s(f, &s->countb[i]);
    for (i = 0; i < 2; i ++)
        qemu_get_be32s(f, &s->config[i]);
    qemu_get_be32s(f, &s->control);

    for (i = 0; i < 5; i ++)
        s3c_timers_start(s, i);

    return 0;
}

struct s3c_timers_state_s *s3c_timers_init(target_phys_addr_t base,
                qemu_irq *pic, qemu_irq *dma)
{
    int i, iomemtype;
    struct s3c_timers_state_s *s = (struct s3c_timers_state_s *)
            qemu_mallocz(sizeof(struct s3c_timers_state_s));

    s->base = base;
    s->dma = dma;

    s3c_timers_reset(s);

    for (i = 0; i < 5; i ++) {
        s->timer[i].t = qemu_new_timer(vm_clock,
                        s3c_timers_tick, &s->timer[i]);
        s->timer[i].s = s;
        s->timer[i].n = i;
        s->timer[i].cmp_cb = 0;
        s->timer[i].irq = pic[i];
    }

    iomemtype = cpu_register_io_memory(0, s3c_timers_readfn,
                    s3c_timers_writefn, s);
    cpu_register_physical_memory(s->base, 0xffffff, iomemtype);

    register_savevm("s3c24xx_timers", 0, 0,
                    s3c_timers_save, s3c_timers_load, s);

    return s;
}

void s3c_timers_cmp_handler_set(void *opaque, int line,
                gpio_handler_t handler, void *cmp_opaque)
{
    struct s3c_timers_state_s *s = (struct s3c_timers_state_s *) opaque;
    if (line > 4 || line < 0) {
        printf("%s: Bad timer number %i.\n", __FUNCTION__, line);
        exit(-1);
    }
    s->timer[line].cmp_cb = handler;
    s->timer[line].cmp_opaque = cmp_opaque;
}

/* UART */
struct s3c_uart_state_s {
    target_phys_addr_t base;
    qemu_irq *irq;
    qemu_irq *dma;
    uint8_t data;
    uint8_t rxfifo[16];
    int rxstart;
    int rxlen;
#define UART_MAX_CHR	4
    int chr_num;
    CharDriverState *chr[UART_MAX_CHR];

    uint8_t lcontrol;
    uint8_t fcontrol;
    uint8_t mcontrol;
    uint16_t control;
    uint16_t brdiv;
    uint8_t errstat;
};

static void s3c_uart_reset(struct s3c_uart_state_s *s)
{
    s->lcontrol = 0x00;
    s->fcontrol = 0x00;
    s->mcontrol = 0x00;
    s->control = 0x0000;
    s->errstat = 0;

    s->rxstart = 0;
    s->rxlen = 0;
}

static void s3c_uart_err(struct s3c_uart_state_s *s, int err)
{
    s->errstat |= err;
    if (s->control & (1 << 6))
        qemu_irq_raise(s->irq[2]);
}

inline static void s3c_uart_full(struct s3c_uart_state_s *s, int pulse)
{
    if (s->fcontrol & 1)			/* FIFOEnable */
        if (s->rxlen < (((s->fcontrol >> 4) & 3) + 1) * 4)
            return;

    switch ((s->control >> 0) & 3) {		/* ReceiveMode */
    case 1:
        if ((s->control & (1 << 8)) || pulse)	/* RxInterruptType */
            qemu_irq_raise(s->irq[0]);
        break;
    case 2:
    case 3:
        qemu_irq_raise(s->dma[0]);
        break;
    }
}

inline static void s3c_uart_empty(struct s3c_uart_state_s *s, int pulse)
{
    switch ((s->control >> 2) & 3) {		/* TransmitMode */
    case 1:
        if ((s->control & (1 << 9)) || pulse)	/* TxInterruptType */
            qemu_irq_raise(s->irq[1]);
        break;
    case 2:
    case 3:
        qemu_irq_raise(s->dma[0]);
        break;
    }
}

inline static void s3c_uart_update(struct s3c_uart_state_s *s)
{
    s3c_uart_empty(s, 0);
    s3c_uart_full(s, 0);
}

static void s3c_uart_params_update(struct s3c_uart_state_s *s)
{
    QEMUSerialSetParams ssp;
    int i;
    if (!s->chr)
        return;

    /* XXX Calculate PCLK frequency from clock manager registers */
    ssp.speed = (S3C_PCLK_FREQ >> 4) / (s->brdiv + 1);

    switch ((s->lcontrol >> 3) & 7) {
    case 4:
    case 6:
        ssp.parity = 'O';
        break;
    case 5:
    case 7:
        ssp.parity = 'E';
        break;
    default:
        ssp.parity = 'N';
    }

    ssp.data_bits = 5 + (s->lcontrol & 3);

    ssp.stop_bits = (s->lcontrol & (1 << 2)) ? 2 : 1;

    for (i = 0; i < s->chr_num; i ++)
        qemu_chr_ioctl(s->chr[i], CHR_IOCTL_SERIAL_SET_PARAMS, &ssp);
}

static int s3c_uart_is_empty(void *opaque)
{
    struct s3c_uart_state_s *s = (struct s3c_uart_state_s *) opaque;
    if (s->fcontrol & 1)			/* FIFOEnable */
        return 16 - s->rxlen;
    else
        return 1 - s->rxlen;
}

static void s3c_uart_rx(void *opaque, const uint8_t *buf, int size)
{
    struct s3c_uart_state_s *s = (struct s3c_uart_state_s *) opaque;
    int left;
    if (s->fcontrol & 1) {			/* FIFOEnable */
        if (s->rxlen + size > 16) {
            size = 16 - s->rxlen;
            s3c_uart_err(s, 1);
        }

        left = 16 - ((s->rxstart + s->rxlen) & 15);
        if (size > left) {
            memcpy(s->rxfifo + ((s->rxstart + s->rxlen) & 15), buf, left);
            memcpy(s->rxfifo, buf + left, size - left);
        } else
            memcpy(s->rxfifo + ((s->rxstart + s->rxlen) & 15), buf, size);
        s->rxlen += size;
    } else {
        if (s->rxlen + size > 1)
            s3c_uart_err(s, 1);
        s->rxlen = 1;
        s->data = buf[0];
    }
    s3c_uart_full(s, 1);
}

/* S3C2410 UART doesn't seem to understand break conditions.  */
static void s3c_uart_event(void *opaque, int event)
{
}

#define S3C_ULCON	0x00	/* UART Line Control register */
#define S3C_UCON	0x04	/* UART Control register */
#define S3C_UFCON	0x08	/* UART FIFO Control register */
#define S3C_UMCON	0x0c	/* UART Modem Control register */
#define S3C_UTRSTAT	0x10	/* UART Tx/Rx Status register */
#define S3C_UERSTAT	0x14	/* UART Error Status register */
#define S3C_UFSTAT	0x18	/* UART FIFO Status register */
#define S3C_UMSTAT	0x1c	/* UART Modem Status register */
#define S3C_UTXH	0x20	/* UART Transmit Buffer register */
#define S3C_URXH	0x24	/* UART Receive Buffer register */
#define S3C_UBRDIV	0x28	/* UART Baud Rate Divisor register */

static uint32_t s3c_uart_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_uart_state_s *s = (struct s3c_uart_state_s *) opaque;
    uint8_t ret;
    addr -= s->base;

    switch (addr) {
    case S3C_ULCON:
        return s->lcontrol;
    case S3C_UCON:
        return s->control;
    case S3C_UFCON:
        return s->fcontrol;
    case S3C_UMCON:
        return s->mcontrol;
    case S3C_UTRSTAT:
        return 6 | !!s->rxlen;
    case S3C_UERSTAT:
        /* XXX: UERSTAT[3] is Reserved but Linux thinks it is BREAK */
        ret = s->errstat;
        s->errstat = 0;
        s3c_uart_update(s);
        return ret;
    case S3C_UFSTAT:
        s3c_uart_update(s);
        return s->rxlen ? s->rxlen | (1 << 8) : 0;
    case S3C_UMSTAT:
        s3c_uart_update(s);
        return 0x11;
    case S3C_URXH:
        s3c_uart_update(s);
        if (s->rxlen) {
            s->rxlen --;
            if (s->fcontrol & 1) {		/* FIFOEnable */
                ret = s->rxfifo[s->rxstart ++];
                s->rxstart &= 15;
            } else
                ret = s->data;
            return ret;
        }
        return 0;
    case S3C_UBRDIV:
        return s->brdiv;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_uart_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_uart_state_s *s = (struct s3c_uart_state_s *) opaque;
    uint8_t ch;
    int i, afc;
    addr -= s->base;

    switch (addr) {
    case S3C_ULCON:
        if ((s->lcontrol ^ value) & (1 << 6))
            printf("%s: UART Infra-red mode %s\n", __FUNCTION__,
                            (value & (1 << 6)) ? "on" : "off");
        s->lcontrol = value;
        s3c_uart_params_update(s);
        s3c_uart_update(s);
        break;
    case S3C_UCON:
        /* XXX: UCON[4] is Reserved but Linux thinks it is BREAK */
        if ((s->control ^ value) & (1 << 5))
            printf("%s: UART loopback test mode %s\n", __FUNCTION__,
                            (value & (1 << 5)) ? "on" : "off");
        s->control = value & 0x7ef;
        s3c_uart_update(s);
        break;
    case S3C_UFCON:
        if (value & (1 << 1))			/* RxReset */
            s->rxlen = 0;
        s->fcontrol = value & 0xf1;
        s3c_uart_update(s);
        break;
    case S3C_UMCON:
        if ((s->mcontrol ^ value) & (1 << 4)) {
            afc = (value >> 4) & 1;
            for (i = 0; i < s->chr_num; i ++)
                qemu_chr_ioctl(s->chr[i], CHR_IOCTL_MODEM_HANDSHAKE, &afc);
        }
        s->mcontrol = value & 0x11;
        s3c_uart_update(s);
        break;
    case S3C_UTXH:
        ch = value & 0xff;
        for (i = 0; i < s->chr_num; i ++)
            qemu_chr_write(s->chr[i], &ch, 1);
        s3c_uart_empty(s, 1);
        s3c_uart_update(s);
        break;
    case S3C_UBRDIV:
        s->brdiv = value & 0xffff;
        s3c_uart_params_update(s);
        s3c_uart_update(s);
        break;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_uart_readfn[] = {
    s3c_uart_read,
    s3c_uart_read,
    s3c_uart_read,
};

static CPUWriteMemoryFunc *s3c_uart_writefn[] = {
    s3c_uart_write,
    s3c_uart_write,
    s3c_uart_write,
};

static void s3c_uart_save(QEMUFile *f, void *opaque)
{
    struct s3c_uart_state_s *s = (struct s3c_uart_state_s *) opaque;
    qemu_put_8s(f, &s->data);
    qemu_put_buffer(f, s->rxfifo, sizeof(s->rxfifo));
    qemu_put_be32(f, s->rxstart);
    qemu_put_be32(f, s->rxlen);
    qemu_put_8s(f, &s->lcontrol);
    qemu_put_8s(f, &s->fcontrol);
    qemu_put_8s(f, &s->mcontrol);
    qemu_put_be16s(f, &s->control);
    qemu_put_be16s(f, &s->brdiv);
    qemu_put_8s(f, &s->errstat);
}

static int s3c_uart_load(QEMUFile *f, void *opaque, int version_id)
{
    struct s3c_uart_state_s *s = (struct s3c_uart_state_s *) opaque;
    qemu_get_8s(f, &s->data);
    qemu_get_buffer(f, s->rxfifo, sizeof(s->rxfifo));
    s->rxstart = qemu_get_be32(f);
    s->rxlen = qemu_get_be32(f);
    qemu_get_8s(f, &s->lcontrol);
    qemu_get_8s(f, &s->fcontrol);
    qemu_get_8s(f, &s->mcontrol);
    qemu_get_be16s(f, &s->control);
    qemu_get_be16s(f, &s->brdiv);
    qemu_get_8s(f, &s->errstat);

    return 0;
}

struct s3c_uart_state_s *s3c_uart_init(target_phys_addr_t base,
                qemu_irq *irqs, qemu_irq *dma)
{
    int iomemtype;
    struct s3c_uart_state_s *s = (struct s3c_uart_state_s *)
            qemu_mallocz(sizeof(struct s3c_uart_state_s));

    s->base = base;
    s->irq = irqs;
    s->dma = dma;

    s3c_uart_reset(s);

    iomemtype = cpu_register_io_memory(0, s3c_uart_readfn,
                    s3c_uart_writefn, s);
    cpu_register_physical_memory(s->base, 0xfff, iomemtype);

    register_savevm("s3c24xx_uart", base, 0, s3c_uart_save, s3c_uart_load, s);

    return s;
}

void s3c_uart_attach(struct s3c_uart_state_s *s, CharDriverState *chr)
{
    if (s->chr_num >= UART_MAX_CHR)
        cpu_abort(cpu_single_env, "%s: Too many devices\n", __FUNCTION__);
    s->chr[s->chr_num ++] = chr;

    qemu_chr_add_handlers(chr, s3c_uart_is_empty,
                    s3c_uart_rx, s3c_uart_event, s);
}

/* ADC & Touchscreen interface */
struct s3c_adc_state_s {
    target_phys_addr_t base;
    qemu_irq irq;
    qemu_irq tcirq;
    QEMUTimer *convt;
    QEMUTimer *tst;
    int x;
    int y;
    int down;
    int enable;
    int input[8];
    int in_idx;
    int noise;

    uint16_t control;
    uint16_t ts;
    uint16_t delay;
    int16_t xdata;
    int16_t ydata;
};

static void s3c_adc_reset(struct s3c_adc_state_s *s)
{
    s->down = 0;
    s->control = 0x3fc4;
    s->ts = 0x58;
    s->delay = 0xff;
    s->enable = 1;
}

static void s3c_adc_start(struct s3c_adc_state_s *s)
{
    if (!s->enable || (s->ts & 7) == 0)
        return;
    s->control &= ~(1 << 15);
    s->in_idx = (s->control >> 3) & 7;
    qemu_mod_timer(s->convt, qemu_get_clock(vm_clock) + (ticks_per_sec >> 5));
}

static void s3c_adc_done(void *opaque)
{
    struct s3c_adc_state_s *s = (struct s3c_adc_state_s *) opaque;
    s->xdata = s->input[s->in_idx] & 0x3ff;
    s->control |= 1 << 15;
    qemu_irq_raise(s->irq);
}

static void s3c_adc_tick(void *opaque)
{
    struct s3c_adc_state_s *s = (struct s3c_adc_state_s *) opaque;
    if (s->down) {
        if ((s->ts & 3) == 3 && s->enable)
            qemu_irq_raise(s->tcirq);
        else if (s->enable && ((s->ts & (1 << 2)) || (s->ts & 3))) {
            s->xdata = (s->x >> 5) | (1 << 14) | ((s->ts & 3) << 12);
            s->ydata = (s->y >> 5) | (1 << 14) | ((s->ts & 3) << 12);
            s->xdata ^= s->noise >> 1;
            s->ydata ^= s->noise >> 2;
            qemu_irq_raise(s->irq);
            s->noise ++;
            s->noise &= 7;
        }
        qemu_mod_timer(s->tst, qemu_get_clock(vm_clock) +
                        (ticks_per_sec >> 5));
    }
}

static void s3c_adc_event(void *opaque,
                int x, int y, int z, int buttons_state)
{
    struct s3c_adc_state_s *s = (struct s3c_adc_state_s *) opaque;
    s->down = !!buttons_state;
    s->x = x;
    s->y = y;
    s3c_adc_tick(s);
}

#define S3C_ADCCON	0x00	/* ADC Control register */
#define S3C_ADCTSC	0x04	/* ADC Touchscreen Control register */
#define S3C_ADCDLY	0x08	/* ADC Start or Interval Delay register */
#define S3C_ADCDAT0	0x0c	/* ADC Conversion Data register 0 */
#define S3C_ADCDAT1	0x10	/* ADC Conversion Data register 1 */

static uint32_t s3c_adc_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_adc_state_s *s = (struct s3c_adc_state_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case S3C_ADCCON:
        return s->control;
    case S3C_ADCTSC:
        return s->ts;
    case S3C_ADCDLY:
        return s->delay;
    case S3C_ADCDAT0:
        if (s->control & 2)
            s3c_adc_start(s);
        return ((!s->down) << 15) | s->xdata;
    case S3C_ADCDAT1:
        return ((!s->down) << 15) | s->ydata;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_adc_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_adc_state_s *s = (struct s3c_adc_state_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case S3C_ADCCON:
        s->control = (s->control & 0x8000) | (value & 0x7ffe);
        s->enable = !(value & 4);
        if ((value & 1) && !(value & 2))
            s3c_adc_start(s);
        if (!s->enable)
            qemu_del_timer(s->convt);
        s3c_adc_tick(s);
        break;

    case S3C_ADCTSC:
        s->ts = value & 0xff;
        break;

    case S3C_ADCDLY:
        s->delay = value & 0xffff;

    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_adc_readfn[] = {
    s3c_adc_read,
    s3c_adc_read,
    s3c_adc_read,
};

static CPUWriteMemoryFunc *s3c_adc_writefn[] = {
    s3c_adc_write,
    s3c_adc_write,
    s3c_adc_write,
};

static void s3c_adc_save(QEMUFile *f, void *opaque)
{
    struct s3c_adc_state_s *s = (struct s3c_adc_state_s *) opaque;
    int i;
    qemu_put_be32(f, s->enable);
    for (i = 0; i < 8; i ++)
        qemu_put_be32(f, s->input[i]);
    qemu_put_be32(f, s->in_idx);
    qemu_put_be32(f, s->noise);

    qemu_put_be16s(f, &s->control);
    qemu_put_be16s(f, &s->ts);
    qemu_put_be16s(f, &s->delay);
    qemu_put_be16s(f, &s->xdata);
    qemu_put_be16s(f, &s->ydata);
}

static int s3c_adc_load(QEMUFile *f, void *opaque, int version_id)
{
    struct s3c_adc_state_s *s = (struct s3c_adc_state_s *) opaque;
    int i;
    s->enable = qemu_get_be32(f);
    for (i = 0; i < 8; i ++)
        s->input[i] = qemu_get_be32(f);
    s->in_idx = qemu_get_be32(f);
    s->noise = qemu_get_be32(f);

    qemu_get_be16s(f, &s->control);
    qemu_get_be16s(f, &s->ts);
    qemu_get_be16s(f, &s->delay);
    qemu_get_be16s(f, &s->xdata);
    qemu_get_be16s(f, &s->ydata);

    if (s->enable && (s->ts & 7) && !(s->control & (1 << 15)))
        s3c_adc_start(s);

    return 0;
}

struct s3c_adc_state_s *s3c_adc_init(target_phys_addr_t base, qemu_irq irq,
                qemu_irq tcirq)
{
    int iomemtype;
    struct s3c_adc_state_s *s = (struct s3c_adc_state_s *)
            qemu_mallocz(sizeof(struct s3c_adc_state_s));

    s->base = base;
    s->irq = irq;
    s->tcirq = tcirq;
    s->convt = qemu_new_timer(vm_clock, s3c_adc_done, s);
    s->tst = qemu_new_timer(vm_clock, s3c_adc_tick, s);

    s3c_adc_reset(s);

    iomemtype = cpu_register_io_memory(0, s3c_adc_readfn,
                    s3c_adc_writefn, s);
    cpu_register_physical_memory(s->base, 0xffffff, iomemtype);

    /* We want absolute coordinates */
    qemu_add_mouse_event_handler(s3c_adc_event, s, 1,
                    "QEMU S3C2410-driven Touchscreen");

    register_savevm("s3c24xx_adc", 0, 0, s3c_adc_save, s3c_adc_load, s);

    return s;
}

/* IIC-bus serial interface */
struct s3c_i2c_state_s {
    i2c_slave slave;
    i2c_bus *bus;
    target_phys_addr_t base;
    qemu_irq irq;

    uint8_t control;
    uint8_t status;
    uint8_t data;
    uint8_t addy;
    int busy;
    int newstart;
};

static void s3c_i2c_irq(struct s3c_i2c_state_s *s)
{
    s->control |= 1 << 4;
    if (s->control & (1 << 5))
        qemu_irq_raise(s->irq);
}

static void s3c_i2c_reset(struct s3c_i2c_state_s *s)
{
    s->control = 0x00;
    s->status = 0x00;
    s->busy = 0;
    s->newstart = 0;
}

static void s3c_i2c_event(i2c_slave *i2c, enum i2c_event event)
{
    struct s3c_i2c_state_s *s = (struct s3c_i2c_state_s *) i2c;
    if (!(s->status & (1 << 4)))
        return;

    switch (event) {
    case I2C_START_RECV:
    case I2C_START_SEND:
        s->status |= 1 << 2;
        s3c_i2c_irq(s);
        break;
    case I2C_FINISH:
        s->status &= ~6;
        break;
    case I2C_NACK:
        s->status |= 1 << 0;
        break;
    default:
        break;
    }
}

static int s3c_i2c_tx(i2c_slave *i2c, uint8_t data)
{
    struct s3c_i2c_state_s *s = (struct s3c_i2c_state_s *) i2c;
    if (!(s->status & (1 << 4)))
        return 1;

    if ((s->status >> 6) == 0)
        s->data = data;						/* TODO */
    s->status &= ~(1 << 0);
    s3c_i2c_irq(s);

    return !(s->control & (1 << 7));
}

static int s3c_i2c_rx(i2c_slave *i2c)
{
    struct s3c_i2c_state_s *s = (struct s3c_i2c_state_s *) i2c;
    if (!(s->status & (1 << 4)))
        return 1;

    if ((s->status >> 6) == 1) {
        s->status &= ~(1 << 0);
        s3c_i2c_irq(s);
        return s->data;
    }

    return 0x00;
}

static void s3c_master_work(void *opaque)
{
    struct s3c_i2c_state_s *s = (struct s3c_i2c_state_s *) opaque;
    int start = 0, stop = 0, ack = 1;
    if (s->control & (1 << 4))				/* Interrupt pending */
        return;
    if ((s->status & 0x90) != 0x90)			/* Master */
        return;
    stop = ~s->status & (1 << 5);
    if (s->newstart && s->status & (1 << 5)) {		/* START */
        s->busy = 1;
        start = 1;
    }
    s->newstart = 0;
    if (!s->busy)
        return;

    if (start)
        ack = !i2c_start_transfer(s->bus, s->data >> 1, (~s->status >> 6) & 1);
    else if (stop)
        i2c_end_transfer(s->bus);
    else if (s->status & (1 << 6))
        ack = !i2c_send(s->bus, s->data);
    else {
        s->data = i2c_recv(s->bus);

        if (!(s->control & (1 << 7)))			/* ACK */
            i2c_nack(s->bus);
    }

    if (!(s->status & (1 << 5))) {
        s->busy = 0;
        return;
    }
    s->status &= ~1;
    s->status |= !ack;
    if (!ack)
        s->busy = 0;
    s3c_i2c_irq(s);
}

#define S3C_IICCON	0x00	/* IIC-Bus Control register */
#define S3C_IICSTAT	0x04	/* IIC-Bus Control / Status register */
#define S3C_IICADD	0x08	/* IIC-Bus Address register */
#define S3C_IICDS	0x0c	/* IIC-Bus Tx / Rx Data Shift register */

static uint32_t s3c_i2c_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_i2c_state_s *s = (struct s3c_i2c_state_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case S3C_IICCON:
        return s->control;
    case S3C_IICSTAT:
        return s->status & ~(1 << 5);			/* Busy signal */
    case S3C_IICADD:
        return s->addy;
    case S3C_IICDS:
        return s->data;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_i2c_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_i2c_state_s *s = (struct s3c_i2c_state_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case S3C_IICCON:
        s->control = (s->control | 0xef) & value;
        if (s->busy)
            s3c_master_work(s);
        break;

    case S3C_IICSTAT:
        s->status &= 0x0f;
        s->status |= value & 0xf0;
        if (s->status & (1 << 5))
            s->newstart = 1;
        s3c_master_work(s);
        break;

    case S3C_IICADD:
        s->addy = value & 0x7f;
        i2c_set_slave_address(&s->slave, s->addy);
        break;

    case S3C_IICDS:
        s->data = value & 0xff;
        break;

    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_i2c_readfn[] = {
    s3c_i2c_read,
    s3c_i2c_read,
    s3c_i2c_read,
};

static CPUWriteMemoryFunc *s3c_i2c_writefn[] = {
    s3c_i2c_write,
    s3c_i2c_write,
    s3c_i2c_write,
};

static void s3c_i2c_save(QEMUFile *f, void *opaque)
{
    struct s3c_i2c_state_s *s = (struct s3c_i2c_state_s *) opaque;
    qemu_put_8s(f, &s->control);
    qemu_put_8s(f, &s->status);
    qemu_put_8s(f, &s->data);
    qemu_put_8s(f, &s->addy);

    qemu_put_be32(f, s->busy);
    qemu_put_be32(f, s->newstart);

    i2c_bus_save(f, s->bus);
    i2c_slave_save(f, &s->slave);
}

static int s3c_i2c_load(QEMUFile *f, void *opaque, int version_id)
{
    struct s3c_i2c_state_s *s = (struct s3c_i2c_state_s *) opaque;
    qemu_get_8s(f, &s->control);
    qemu_get_8s(f, &s->status);
    qemu_get_8s(f, &s->data);
    qemu_get_8s(f, &s->addy);

    s->busy = qemu_get_be32(f);
    s->newstart = qemu_get_be32(f);

    i2c_bus_load(f, s->bus);
    i2c_slave_load(f, &s->slave);
    return 0;
}

struct s3c_i2c_state_s *s3c_i2c_init(target_phys_addr_t base, qemu_irq irq)
{
    int iomemtype;
    struct s3c_i2c_state_s *s = (struct s3c_i2c_state_s *)
            qemu_mallocz(sizeof(struct s3c_i2c_state_s));

    s->base = base;
    s->irq = irq;
    s->slave.event = s3c_i2c_event;
    s->slave.send = s3c_i2c_tx;
    s->slave.recv = s3c_i2c_rx;
    s->bus = i2c_init_bus();

    s3c_i2c_reset(s);

    iomemtype = cpu_register_io_memory(0, s3c_i2c_readfn,
                    s3c_i2c_writefn, s);
    cpu_register_physical_memory(s->base, 0xffffff, iomemtype);

    register_savevm("s3c24xx_i2c", 0, 0, s3c_i2c_save, s3c_i2c_load, s);

    return s;
}

i2c_bus *s3c_i2c_bus(struct s3c_i2c_state_s *s)
{
    return s->bus;
}

/* Serial Peripheral Interface */
struct s3c_spi_state_s {
    target_phys_addr_t base;

    struct {
        qemu_irq irq;
        qemu_irq drq;
        qemu_irq miso;

        uint8_t control;
        uint8_t pin;
        uint8_t pre;

        int cs_pin;
        int clk_pin;
        int mosi_pin;
        uint8_t txbuf;
        uint8_t rxbuf;
        int bit;
    } chan[2];

    uint8_t (*txrx[2])(void *opaque, uint8_t value);
    uint8_t (*btxrx[2])(void *opaque, uint8_t value);
    void *opaque[2];
};

static void s3c_spi_update(struct s3c_spi_state_s *s)
{
    int i;
    for (i = 0; i < 2; i ++) {
        switch ((s->chan[i].control >> 5) & 3) {		/* SMOD */
        case 1:
            qemu_irq_raise(s->chan[i].irq);
            break;
        case 2:
            qemu_irq_raise(s->chan[i].drq);
            break;
        }
    }
}

static void s3c_spi_reset(struct s3c_spi_state_s *s)
{
    memset(s->chan, 0, sizeof(s->chan));
    s->chan[0].pin = 0x02;
    s->chan[1].pin = 0x02;
    s3c_spi_update(s);
}

#define S3C_SPCON0	0x00	/* SPI channel 0 control register */
#define S3C_SPSTA0	0x04	/* SPI channel 0 status register */
#define S3C_SPPIN0	0x08	/* SPI channel 0 pin control register */
#define S3C_SPPRE0	0x0c	/* SPI channel 0 baudrate prescaler register */
#define S3C_SPTDAT0	0x10	/* SPI channel 0 Tx data register */
#define S3C_SPRDAT0	0x14	/* SPI channel 0 Rx data register */
#define S3C_SPCON1	0x20	/* SPI channel 1 control register */
#define S3C_SPSTA1	0x24	/* SPI channel 1 status register */
#define S3C_SPPIN1	0x28	/* SPI channel 1 pin control register */
#define S3C_SPPRE1	0x2c	/* SPI channel 1 baudrate prescaler register */
#define S3C_SPTDAT1	0x30	/* SPI channel 1 Tx data register */
#define S3C_SPRDAT1	0x34	/* SPI channel 1 Rx data register */

static uint32_t s3c_spi_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_spi_state_s *s = (struct s3c_spi_state_s *) opaque;
    int ch;
    addr -= s->base;
    ch = addr >> 5;

    switch (addr) {
    case S3C_SPCON0:
    case S3C_SPCON1:
        return s->chan[ch].control;

    case S3C_SPSTA0:
    case S3C_SPSTA1:
        return 0x01;

    case S3C_SPPIN0:
    case S3C_SPPIN1:
        return s->chan[ch].pin;

    case S3C_SPPRE0:
    case S3C_SPPRE1:
        return s->chan[ch].pre;

    case S3C_SPTDAT0:
    case S3C_SPTDAT1:
        return s->chan[ch + 2].txbuf;

    case S3C_SPRDAT0:
    case S3C_SPRDAT1:
        if (s->txrx[ch] && (s->chan[ch].control & 0x19) == 0x19)
            s->chan[ch].rxbuf = s->txrx[ch](s->opaque[ch], 'Q');
        s3c_spi_update(s);
        return s->chan[ch].rxbuf;

    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_spi_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_spi_state_s *s = (struct s3c_spi_state_s *) opaque;
    int ch;
    addr -= s->base;
    ch = addr >> 5;

    switch (addr) {
    case S3C_SPCON0:
    case S3C_SPCON1:
        s->chan[ch].control = value & 0x7f;
        s3c_spi_update(s);
        break;

    case S3C_SPPIN0:
    case S3C_SPPIN1:
        s->chan[ch].pin = value & 0x07;
        break;

    case S3C_SPPRE0:
    case S3C_SPPRE1:
        s->chan[ch].pre = value & 0xff;
        break;

    case S3C_SPTDAT0:
    case S3C_SPTDAT1:
        s->chan[ch].txbuf = value & 0xff;
        if (s->txrx[ch] && (s->chan[ch].control & 0x19) == 0x18)
            s->chan[ch].rxbuf = s->txrx[ch](s->opaque[ch], value & 0xff);
        s3c_spi_update(s);
        break;

    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_spi_readfn[] = {
    s3c_spi_read,
    s3c_spi_read,
    s3c_spi_read,
};

static CPUWriteMemoryFunc *s3c_spi_writefn[] = {
    s3c_spi_write,
    s3c_spi_write,
    s3c_spi_write,
};

static void s3c_spi_save(QEMUFile *f, void *opaque)
{
    struct s3c_spi_state_s *s = (struct s3c_spi_state_s *) opaque;
    int i;
    for (i = 0; i < 2; i ++) {
        qemu_put_8s(f, &s->chan[i].control);
        qemu_put_8s(f, &s->chan[i].pin);
        qemu_put_8s(f, &s->chan[i].pre);

        qemu_put_8s(f, &s->chan[i].txbuf);
        qemu_put_8s(f, &s->chan[i].rxbuf);
        qemu_put_be32(f, s->chan[i].cs_pin);
        qemu_put_be32(f, s->chan[i].clk_pin);
        qemu_put_be32(f, s->chan[i].mosi_pin);
        qemu_put_be32(f, s->chan[i].bit);
    }
}

static int s3c_spi_load(QEMUFile *f, void *opaque, int version_id)
{
    struct s3c_spi_state_s *s = (struct s3c_spi_state_s *) opaque;
    int i;
    for (i = 0; i < 2; i ++) {
        qemu_get_8s(f, &s->chan[i].control);
        qemu_get_8s(f, &s->chan[i].pin);
        qemu_get_8s(f, &s->chan[i].pre);

        qemu_get_8s(f, &s->chan[i].txbuf);
        qemu_get_8s(f, &s->chan[i].rxbuf);
        s->chan[i].cs_pin = qemu_get_be32(f);
        s->chan[i].clk_pin = qemu_get_be32(f);
        s->chan[i].mosi_pin = qemu_get_be32(f);
        s->chan[i].bit = qemu_get_be32(f);
    }

    return 0;
}

static void s3c_spi_bitbang_cs(void *opaque, int line, int level)
{
    struct s3c_spi_state_s *s = (struct s3c_spi_state_s *) opaque;
    int ch = line;
    if (s->chan[ch].cs_pin || level) {
        if (s->chan[ch].bit && s->txrx[ch] && !s->btxrx[ch]) {
            s->chan[ch].txbuf <<= 8 - s->chan[ch].bit;
            s->chan[ch].rxbuf = s->txrx[ch](s->opaque[ch], s->chan[ch].txbuf);
        }
    } else if (!s->chan[ch].cs_pin || !level)
        s->chan[ch].bit = 0;

    /* SSn is active low.  */
    s->chan[ch].cs_pin = !level;
}

static void s3c_spi_bitbang_clk(void *opaque, int line, int level)
{
    struct s3c_spi_state_s *s = (struct s3c_spi_state_s *) opaque;
    int ch = line;
    if (!s->chan[ch].cs_pin)
        goto done;

    /* Detect CLK rising edge */
    if (s->chan[ch].clk_pin || !level)
        goto done;

    if (s->btxrx[ch]) {
        qemu_set_irq(s->chan[ch].miso,
                        s->btxrx[ch](s->opaque[ch], s->chan[ch].mosi_pin));
        goto done;
    }

    s->chan[ch].txbuf <<= 1;
    s->chan[ch].txbuf |= s->chan[ch].mosi_pin;

    qemu_set_irq(s->chan[ch].miso, (s->chan[ch].rxbuf >> 7) & 1);
    s->chan[ch].rxbuf <<= 1;

    if (++ s->chan[ch].bit == 8) {
        if (s->txrx[ch])
            s->chan[ch].rxbuf = s->txrx[ch](s->opaque[ch], s->chan[ch].txbuf);
        s->chan[ch].bit = 0;
    }

done:
    s->chan[ch].clk_pin = level;
}

static void s3c_spi_bitbang_mosi(void *opaque, int line, int level)
{
    struct s3c_spi_state_s *s = (struct s3c_spi_state_s *) opaque;
    int ch = line;
    s->chan[ch].mosi_pin = level;
}

static const struct {
    int cs, clk, miso, mosi;
} s3c_spi_pins[2] = {
    { S3C_GPG(2), S3C_GPE(13), S3C_GPE(11), S3C_GPE(12) },
    { S3C_GPG(3), S3C_GPG(7),  S3C_GPG(5),  S3C_GPG(6)  },
};

static void s3c_spi_bitbang_init(struct s3c_spi_state_s *s,
                struct s3c_gpio_state_s *gpio)
{
    int i;
    qemu_irq *cs = qemu_allocate_irqs(s3c_spi_bitbang_cs, s, 2);
    qemu_irq *clk = qemu_allocate_irqs(s3c_spi_bitbang_clk, s, 2);
    qemu_irq *mosi = qemu_allocate_irqs(s3c_spi_bitbang_mosi, s, 2);

    for (i = 0; i < 2; i ++) {
        s3c_gpio_out_set(gpio, s3c_spi_pins[i].cs, cs[i]);
        s3c_gpio_out_set(gpio, s3c_spi_pins[i].clk, clk[i]);
        s->chan[i].miso = s3c_gpio_in_get(gpio)[s3c_spi_pins[i].miso];
        s3c_gpio_out_set(gpio, s3c_spi_pins[i].mosi, mosi[i]);
    }
}

struct s3c_spi_state_s *s3c_spi_init(target_phys_addr_t base,
                qemu_irq irq0, qemu_irq drq0, qemu_irq irq1, qemu_irq drq1,
                struct s3c_gpio_state_s *gpio)
{
    int iomemtype;
    struct s3c_spi_state_s *s = (struct s3c_spi_state_s *)
            qemu_mallocz(sizeof(struct s3c_spi_state_s));

    s->base = base;
    s->chan[0].irq = irq0;
    s->chan[0].drq = drq0;
    s->chan[1].irq = irq1;
    s->chan[1].drq = drq1;

    s3c_spi_reset(s);

    iomemtype = cpu_register_io_memory(0, s3c_spi_readfn,
                    s3c_spi_writefn, s);
    cpu_register_physical_memory(s->base, 0xffffff, iomemtype);

    s3c_spi_bitbang_init(s, gpio);

    register_savevm("s3c24xx_spi", 0, 0, s3c_spi_save, s3c_spi_load, s);

    return s;
}

void s3c_spi_attach(struct s3c_spi_state_s *s, int ch,
                uint8_t (*txrx)(void *opaque, uint8_t value),
                uint8_t (*btxrx)(void *opaque, uint8_t value), void *opaque)
{
    if (ch & ~1)
        cpu_abort(cpu_single_env, "%s: No channel %i\n", __FUNCTION__, ch);
    s->txrx[ch] = txrx;
    s->btxrx[ch] = btxrx;
    s->opaque[ch] = opaque;
}

/* IIS-BUS interface */
static inline void s3c_i2s_update(struct s3c_i2s_state_s *s)
{
    s->tx_en =
            (s->control & (1 << 0)) && !(s->control & (1 << 3)) &&
            (s->mode & (1 << 7)) && (s->fcontrol & (1 << 13));
    s->rx_en =
            (s->control & (1 << 0)) && !(s->control & (1 << 2)) &&
            (s->mode & (1 << 6)) && (s->fcontrol & (1 << 12));
    s->control &= ~0xc0;
    /* The specs are unclear about the FIFO-ready flags logic.
     * Implement semantics that make most sense.  */
    if (s->tx_en && s->tx_len)
        s->control |= (1 << 7);
    if (s->rx_en && s->rx_len)
        s->control |= (1 << 6);

    qemu_set_irq(s->dma[S3C_RQ_I2SSDO], (s->control >> 5) &
                    (s->control >> 7) & (s->fcontrol >> 15) & 1);
    qemu_set_irq(s->dma[S3C_RQ_I2SSDI0], (s->control >> 4) &
                    (s->control >> 6) & (s->fcontrol >> 14) & 1);
    qemu_set_irq(s->dma[S3C_RQ_I2SSDI1], (s->control >> 4) &
                    (s->control >> 6) & (s->fcontrol >> 14) & 1);
}

static void s3c_i2s_reset(struct s3c_i2s_state_s *s)
{
    s->control = 0x100;
    s->mode = 0x000;
    s->prescaler = 0x000;
    s->fcontrol = 0x0000;
    s->tx_len = 0;
    s->rx_len = 0;
    s3c_i2s_update(s);
}

#define S3C_IISCON	0x00	/* IIS Control register */
#define S3C_IISMOD	0x04	/* IIS Mode register */
#define S3C_IISPSR	0x08	/* IIS Prescaler register */
#define S3C_IISFCON	0x0c	/* IIS FIFO Interface register */
#define S3C_IISFIFO	0x10	/* IIS FIFO register */

static uint32_t s3c_i2s_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_i2s_state_s *s = (struct s3c_i2s_state_s *) opaque;
    uint32_t ret;
    addr -= s->base;

    switch (addr) {
    case S3C_IISCON:
        return s->control;
    case S3C_IISMOD:
        return s->mode;
    case S3C_IISPSR:
        return s->prescaler;
    case S3C_IISFCON:
        return s->fcontrol |
                (MAX(32 - s->tx_len, 0) << 6) |
                MIN(s->rx_len, 32);
    case S3C_IISFIFO:
        if (s->rx_len > 0) {
            s->rx_len --;
            s3c_i2s_update(s);
            s->cycle ^= 1;
            if (s->cycle) {
                s->buffer = (uint16_t) (ret = s->codec_in(s->opaque));
                return ret >> 16;
            } else
                return s->buffer;
        }
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_i2s_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_i2s_state_s *s = (struct s3c_i2s_state_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case S3C_IISCON:
        s->control = (s->control & 0x100) | (value & 0x03f);
        s3c_i2s_update(s);
        break;
    case S3C_IISMOD:
        s->mode = value & 0x1ff;
        s3c_i2s_update(s);
        break;
    case S3C_IISPSR:
        s->prescaler = value & 0x3ff;
        break;
    case S3C_IISFCON:
        s->fcontrol = value & 0xf000;
        s3c_i2s_update(s);
        break;
    case S3C_IISFIFO:
        if (s->tx_len && s->tx_en) {
            s->tx_len --;
            s3c_i2s_update(s);
            if (s->cycle)
                s->codec_out(s->opaque, value | ((uint32_t) s->buffer << 16));
            else
                s->buffer = (uint16_t) value;
            s->cycle ^= 1;
        }
        break;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_i2s_readfn[] = {
    s3c_i2s_read,
    s3c_i2s_read,
    s3c_i2s_read,
};

static CPUWriteMemoryFunc *s3c_i2s_writefn[] = {
    s3c_i2s_write,
    s3c_i2s_write,
    s3c_i2s_write,
};

static void s3c_i2s_save(QEMUFile *f, void *opaque)
{
    struct s3c_i2s_state_s *s = (struct s3c_i2s_state_s *) opaque;
    qemu_put_be16s(f, &s->control);
    qemu_put_be16s(f, &s->mode);
    qemu_put_be16s(f, &s->prescaler);
    qemu_put_be16s(f, &s->fcontrol);

    qemu_put_be32(f, s->tx_en);
    qemu_put_be32(f, s->rx_en);
    qemu_put_be32(f, s->tx_len);
    qemu_put_be32(f, s->rx_len);
    qemu_put_be16(f, s->buffer);
    qemu_put_be32(f, s->cycle);
}

static int s3c_i2s_load(QEMUFile *f, void *opaque, int version_id)
{
    struct s3c_i2s_state_s *s = (struct s3c_i2s_state_s *) opaque;
    qemu_get_be16s(f, &s->control);
    qemu_get_be16s(f, &s->mode);
    qemu_get_be16s(f, &s->prescaler);
    qemu_get_be16s(f, &s->fcontrol);

    s->tx_en = qemu_get_be32(f);
    s->rx_en = qemu_get_be32(f);
    s->tx_len = qemu_get_be32(f);
    s->rx_len = qemu_get_be32(f);
    s->buffer = qemu_get_be16(f);
    s->cycle = qemu_get_be32(f);

    return 0;
}

static void s3c_i2s_data_req(void *opaque, int tx, int rx)
{
    struct s3c_i2s_state_s *s = (struct s3c_i2s_state_s *) opaque;
    s->tx_len = tx;
    s->rx_len = rx;
    s3c_i2s_update(s);
}

struct s3c_i2s_state_s *s3c_i2s_init(target_phys_addr_t base, qemu_irq *dma)
{
    int iomemtype;
    struct s3c_i2s_state_s *s = (struct s3c_i2s_state_s *)
            qemu_mallocz(sizeof(struct s3c_i2s_state_s));

    s->base = base;
    s->dma = dma;
    s->data_req = s3c_i2s_data_req;

    s3c_i2s_reset(s);

    iomemtype = cpu_register_io_memory(0, s3c_i2s_readfn,
                    s3c_i2s_writefn, s);
    cpu_register_physical_memory(s->base, 0xffffff, iomemtype);

    register_savevm("s3c24xx_iis", 0, 0, s3c_i2s_save, s3c_i2s_load, s);

    return s;
}

/* Watchdog Timer */
struct s3c_wdt_state_s {
    target_phys_addr_t base;
    qemu_irq irq;
    uint16_t control;
    uint16_t data;
    uint16_t count;
    QEMUTimer *tm;
    int64_t timestamp;
};

static void s3c_wdt_start(struct s3c_wdt_state_s *s)
{
    int enable = s->control & (1 << 5);
    int prescaler = (s->control >> 8) + 1;
    int divider = prescaler << (((s->control >> 3) & 3) + 4);
    if (enable) {
        s->timestamp = qemu_get_clock(vm_clock);
        qemu_mod_timer(s->tm, s->timestamp + muldiv64(divider * s->count,
                                ticks_per_sec, S3C_PCLK_FREQ));
    } else
        qemu_del_timer(s->tm);
}

static void s3c_wdt_stop(struct s3c_wdt_state_s *s)
{
    int prescaler = (s->control >> 8) + 1;
    int divider = prescaler << (((s->control >> 3) & 3) + 4);
    int diff;

    diff = muldiv64(qemu_get_clock(vm_clock) - s->timestamp, S3C_PCLK_FREQ,
                    ticks_per_sec) / divider;
    s->count -= MIN(s->count, diff);
    s->timestamp = qemu_get_clock(vm_clock);
}

static void s3c_wdt_reset(struct s3c_wdt_state_s *s)
{
    s->control = 0x8021;
    s->data = 0x8000;
    s->count = 0x8000;
    s3c_wdt_start(s);
}

static void s3c_wdt_timeout(void *opaque)
{
    struct s3c_wdt_state_s *s = (struct s3c_wdt_state_s *) opaque;
    if (s->control & (1 << 0)) {
        qemu_system_reset_request();
        return;
    }
    if (s->control & (1 << 2))
        qemu_irq_raise(s->irq);
    s->count = s->data;
    s3c_wdt_start(s);
}

#define S3C_WTCON	0x00	/* Watchdog timer control register */
#define S3C_WTDAT	0x04	/* Watchdog timer data register */
#define S3C_WTCNT	0x08	/* Watchdog timer count register */

static uint32_t s3c_wdt_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_wdt_state_s *s = (struct s3c_wdt_state_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case S3C_WTCON:
        return s->control;
    case S3C_WTDAT:
        return s->data;
    case S3C_WTCNT:
        s3c_wdt_stop(s);
        return s->count;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_wdt_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_wdt_state_s *s = (struct s3c_wdt_state_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case S3C_WTCON:
        s3c_wdt_stop(s);
        s->control = value;
        s3c_wdt_start(s);
        break;
    case S3C_WTDAT:
        s->data = value;
        break;
    case S3C_WTCNT:
        s->count = value;
        s3c_wdt_start(s);
        break;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_wdt_readfn[] = {
    s3c_wdt_read,
    s3c_wdt_read,
    s3c_wdt_read,
};

static CPUWriteMemoryFunc *s3c_wdt_writefn[] = {
    s3c_wdt_write,
    s3c_wdt_write,
    s3c_wdt_write,
};

static void s3c_wdt_save(QEMUFile *f, void *opaque)
{
    struct s3c_wdt_state_s *s = (struct s3c_wdt_state_s *) opaque;

    s3c_wdt_stop(s);
    qemu_put_be16s(f, &s->control);
    qemu_put_be16s(f, &s->data);
    qemu_put_be16s(f, &s->count);
    qemu_put_be64s(f, &s->timestamp);
}

static int s3c_wdt_load(QEMUFile *f, void *opaque, int version_id)
{
    struct s3c_wdt_state_s *s = (struct s3c_wdt_state_s *) opaque;

    qemu_get_be16s(f, &s->control);
    qemu_get_be16s(f, &s->data);
    qemu_get_be16s(f, &s->count);
    qemu_get_be64s(f, &s->timestamp);
    s3c_wdt_start(s);

    return 0;
}

struct s3c_wdt_state_s *s3c_wdt_init(target_phys_addr_t base, qemu_irq irq)
{
    int iomemtype;
    struct s3c_wdt_state_s *s = (struct s3c_wdt_state_s *)
            qemu_mallocz(sizeof(struct s3c_wdt_state_s));

    s->base = base;
    s->irq = irq;
    s->tm = qemu_new_timer(vm_clock, s3c_wdt_timeout, s);

    s3c_wdt_reset(s);

    iomemtype = cpu_register_io_memory(0, s3c_wdt_readfn,
                    s3c_wdt_writefn, s);
    cpu_register_physical_memory(s->base, 0xffffff, iomemtype);

    register_savevm("s3c24xx_wdt", 0, 0, s3c_wdt_save, s3c_wdt_load, s);

    return s;
}

/* On-chip UARTs */
static struct {
    target_phys_addr_t base;
    int irq[3];
    int dma[1];
} s3c2410_uart[] = {
    {
        0x50000000,
        { S3C_PICS_RXD0, S3C_PICS_TXD0, S3C_PICS_ERR0 },
        { S3C_RQ_UART0 },
    },
    {
        0x50004000,
        { S3C_PICS_RXD1, S3C_PICS_TXD1, S3C_PICS_ERR1 },
        { S3C_RQ_UART1 },
    },
    {
        0x50008000,
        { S3C_PICS_RXD2, S3C_PICS_TXD2, S3C_PICS_ERR2 },
        { S3C_RQ_UART2 },
    },
    { 0, { 0, 0, 0 }, { 0 } }
};

/* General CPU reset */
void s3c2410_reset(void *opaque)
{
    struct s3c_state_s *s = (struct s3c_state_s *) opaque;
    int i;
    s3c_mc_reset(s);
    s3c_pic_reset(s->pic);
    s3c_dma_reset(s->dma);
    s3c_gpio_reset(s->io);
    s3c_lcd_reset(s->lcd);
    s3c_timers_reset(s->timers);
    s3c_mmci_reset(s->mmci);
    s3c_adc_reset(s->adc);
    s3c_i2c_reset(s->i2c);
    s3c_i2s_reset(s->i2s);
    s3c_rtc_reset(s->rtc);
    s3c_spi_reset(s->spi);
    s3c_udc_reset(s->udc);
    s3c_wdt_reset(s->wdt);
    s3c_clkpwr_reset(s);
    s3c_nand_reset(s);
    for (i = 0; s3c2410_uart[i].base; i ++)
        s3c_uart_reset(s->uart[i]);
    cpu_reset(s->env);
}

/* Initialise an S3C2410A microprocessor.  */
struct s3c_state_s *s3c2410_init(unsigned int sdram_size, DisplayState *ds)
{
    struct s3c_state_s *s;
    int iomemtype, i;
    s = (struct s3c_state_s *) qemu_mallocz(sizeof(struct s3c_state_s));

    s->env = cpu_init();
    cpu_arm_set_model(s->env, "arm920t");
    register_savevm("cpu", 0, 0, cpu_save, cpu_load, s->env);

    cpu_register_physical_memory(S3C_RAM_BASE, sdram_size,
                    qemu_ram_alloc(sdram_size) | IO_MEM_RAM);

    /* If OM pins are 00, SRAM is mapped at 0x0 instead.  */
    cpu_register_physical_memory(S3C_SRAM_BASE, S3C_SRAM_SIZE,
                    qemu_ram_alloc(S3C_SRAM_SIZE) | IO_MEM_RAM);

    s->mc_base = 0x48000000;
    s3c_mc_reset(s);
    iomemtype = cpu_register_io_memory(0, s3c_mc_readfn, s3c_mc_writefn, s);
    cpu_register_physical_memory(s->mc_base, 0xffffff, iomemtype);
    register_savevm("s3c24xx_mc", 0, 0, s3c_mc_save, s3c_mc_load, s);

    s->pic = s3c_pic_init(0x4a000000, arm_pic_init_cpu(s->env));
    s->irq = s3c_pic_get(s->pic);

    s->dma = s3c_dma_init(0x4b000000, &s->irq[S3C_PIC_DMA0]);
    s->drq = s3c_dma_get(s->dma);

    s->clkpwr_base = 0x4c000000;
    s3c_clkpwr_reset(s);
    iomemtype = cpu_register_io_memory(0, s3c_clkpwr_readfn,
                    s3c_clkpwr_writefn, s);
    cpu_register_physical_memory(s->clkpwr_base, 0xffffff, iomemtype);
    register_savevm("s3c24xx_clkpwr", 0, 0,
                    s3c_clkpwr_save, s3c_clkpwr_load, s);

    s->lcd = s3c_lcd_init(0x4d000000, ds, s->irq[S3C_PIC_LCD]);

    s->nand_base = 0x4e000000;
    s3c_nand_reset(s);
    iomemtype = cpu_register_io_memory(0, s3c_nand_readfn,
                    s3c_nand_writefn, s);
    cpu_register_physical_memory(s->nand_base, 0xffffff, iomemtype);
    register_savevm("s3c24xx_nand", 0, 0, s3c_nand_save, s3c_nand_load, s);

    for (i = 0; s3c2410_uart[i].base; i ++) {
        s->uart[i] = s3c_uart_init(s3c2410_uart[i].base,
                        &s->irq[s3c2410_uart[i].irq[0]],
                        &s->drq[s3c2410_uart[i].dma[0]]);
        if (serial_hds[i])
            s3c_uart_attach(s->uart[i], serial_hds[i]);
    }

    s->timers = s3c_timers_init(0x51000000, &s->irq[S3C_PIC_TIMER0], s->drq);

    s->udc = s3c_udc_init(0x52000000, s->irq[S3C_PIC_USBD], s->drq);

    s->wdt = s3c_wdt_init(0x53000000, s->irq[S3C_PIC_WDT]);

    s->i2c = s3c_i2c_init(0x54000000, s->irq[S3C_PIC_IIC]);

    s->i2s = s3c_i2s_init(0x55000000, s->drq);

    s->io = s3c_gpio_init(0x56000000, s->irq);

    s->rtc = s3c_rtc_init(0x57000000, s->irq[S3C_PIC_RTC]);

    s->adc = s3c_adc_init(0x58000000, s->irq[S3C_PICS_ADC],
                    s->irq[S3C_PICS_TC]);

    s->spi = s3c_spi_init(0x59000000,
                    s->irq[S3C_PIC_SPI0], s->drq[S3C_RQ_SPI0],
                    s->irq[S3C_PIC_SPI1], s->drq[S3C_RQ_SPI1], s->io);

    s->mmci = s3c_mmci_init(0x5a000000, s->irq[S3C_PIC_SDI], s->drq);

    if (usb_enabled) {
        usb_ohci_init_memio(0x49000000, 3, -1, s->irq[S3C_PIC_USBH]);
    }

    qemu_register_reset(s3c2410_reset, s);

    s3c_nand_setwp(s, 1);

    /* Power on reset */
    s3c_gpio_setpwrstat(s->io, 1);
    return s;
}
