/*
 * Samsung S3C24xx series I/O ports.
 *
 * Copyright (c) 2007 OpenMoko, Inc.
 * Author: Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This code is licenced under the GNU GPL v2.
 */
#include "vl.h"

#define S3C_IO_BANKS	8

struct s3c_gpio_state_s {	/* Modelled as an interrupt controller */
    target_phys_addr_t base;
    qemu_irq *pic;
    qemu_irq *in;

    struct {
        int n;
        uint32_t con;
        uint32_t dat;
        uint32_t up;
        uint32_t mask;
        qemu_irq handler[32];
    } bank[S3C_IO_BANKS];

    uint32_t inform[2];
    uint32_t pwrstat;
    uint32_t misccr;
    uint32_t dclkcon;
    uint32_t extint[3];
    uint32_t eintflt[2];
    uint32_t eintmask;
    uint32_t eintpend;
};

static inline void s3c_gpio_extint(struct s3c_gpio_state_s *s, int irq)
{
    if (s->eintmask & (1 << irq))
        return;
    s->eintpend |= (1 << irq) & 0x00fffff0;
    switch (irq) {
    case 0 ... 3:
        qemu_irq_raise(s->pic[S3C_PIC_EINT0 + irq]);
        break;
    case 4 ... 7:
        qemu_irq_raise(s->pic[S3C_PIC_EINT4]);
        break;
    case 8 ... 23:
        qemu_irq_raise(s->pic[S3C_PIC_EINT8]);
        break;
    }
}

static void s3c_gpio_set(void *opaque, int line, int level)
{
    struct s3c_gpio_state_s *s = (struct s3c_gpio_state_s *) opaque;
    int e, eint, bank = line >> 5;
    line &= 0x1f;
    /* Input ports */
    if (bank > 0 && ((s->bank[bank].con >> (2 * line)) & 3) == 0) {
        if (level)
            s->bank[bank].dat |= 1 << line;
        else
            s->bank[bank].dat &= ~(1 << line);
        return;
    }
    /* External interrupts */
    if (((s->bank[bank].con >> (2 * line)) & 3) == 2) {
        switch (bank) {
        case 5:	/* GPF */
            eint = line;
            e = 0;
            break;
        case 6:	/* GPG */
            eint = line + 8;
            e = (line > 15) ? 2 : 1;
            break;
        default:
            return;
        }
        if (level) {
            if (!((s->bank[bank].dat >> line) & 1))
                switch ((s->extint[e] >> (line * 3)) & 7) {
                case 1:
                case 4 ... 7:
                    s3c_gpio_extint(s, eint);
                    break;
                }
            s->bank[bank].dat |= 1 << line;
        } else {
            if ((s->bank[bank].dat >> line) & 1)
                switch ((s->extint[e] >> (line * 3)) & 7) {
                case 1:
                case 4 ... 5:
                    break;
                default:
                    s3c_gpio_extint(s, eint);
                }
            s->bank[bank].dat &= ~(1 << line);
        }
        return;
    }
}

qemu_irq *s3c_gpio_in_get(struct s3c_gpio_state_s *s)
{
    return s->in;
}

void s3c_gpio_out_set(struct s3c_gpio_state_s *s, int line, qemu_irq handler)
{
    int bank = line >> 5;
    line &= 0x1f;
    if (bank >= S3C_IO_BANKS || line >= s->bank[bank].n)
        cpu_abort(cpu_single_env, "%s: No I/O port %i\n", __FUNCTION__, line);
    s->bank[bank].handler[line] = handler;
}

void s3c_gpio_reset(struct s3c_gpio_state_s *s)
{
    int i;
    s->inform[0] = 0;
    s->inform[1] = 0;
    s->misccr = 0x00010330 & ~(1 << 16);
    s->dclkcon = 0x00000000;
    s->extint[0] = 0;
    s->extint[1] = 0;
    s->extint[2] = 0;
    s->eintflt[0] = 0;
    s->eintflt[1] = 0;
    s->eintmask = 0x00fffff0;
    s->eintpend = 0x00000000;

    for (i = 0; i < S3C_IO_BANKS; i ++) {
        s->bank[i].mask = (1 << s->bank[i].n) - 1;
        s->bank[i].con = 0;
        s->bank[i].dat = 0;
        s->bank[i].up = 0;
    }
    s->bank[0].con = 0x07ffffff;
    s->bank[3].up = 0x0000f000;
    s->bank[6].up = 0x0000f800;
}

void s3c_gpio_setpwrstat(struct s3c_gpio_state_s *s, int stat)
{
    s->pwrstat = stat;
}

#define S3C_GPCON	0x00	/* Configuration register */
#define S3C_GPDAT	0x04	/* Data register */
#define S3C_GPUP	0x08	/* Pull-up register */
#define S3C_MISCCR	0x80	/* Miscellaneous Control register */
#define S3C_DCLKCON	0x84	/* DCLK0/1 Control register */
#define S3C_EXTINT0	0x88	/* External Interrupt Control register 0 */
#define S3C_EXTINT1	0x8c	/* External Interrupt Control register 1 */
#define S3C_EXTINT2	0x90	/* External Interrupt Control register 2 */
#define S3C_EINTFLT0	0x94	/* External Interrupt Filter register 0 */
#define S3C_EINTFLT1	0x98	/* External Interrupt Filter register 1 */
#define S3C_EINTFLT2	0x9c	/* External Interrupt Filter register 2 */
#define S3C_EINTFLT3	0xa0	/* External Interrupt Filter register 3 */
#define S3C_EINTMASK	0xa4	/* External Interrupt Mask register */
#define S3C_EINTPEND	0xa8	/* External Interrupt Pending register */
#define S3C_GSTATUS0	0xac	/* External Pin Status register */
#define S3C_GSTATUS1	0xb0	/* Chip ID register */
#define S3C_GSTATUS2	0xb4	/* Reset Status register */
#define S3C_GSTATUS3	0xb8	/* Inform register */
#define S3C_GSTATUS4	0xbc	/* Inform register */

static uint32_t s3c_gpio_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_gpio_state_s *s = (struct s3c_gpio_state_s *) opaque;
    int bank = 0;
    addr -= s->base;
    if ((addr >> 4) < S3C_IO_BANKS) {
        bank = addr >> 4;
        addr &= 0xf;
    }

    switch (addr) {
    case S3C_GSTATUS0:
        return 0x0;
    case S3C_GSTATUS1:
        return 0x32410002;
    case S3C_GSTATUS2:
        return s->pwrstat;
    case S3C_GSTATUS3:
        return s->inform[0];
    case S3C_GSTATUS4:
        return s->inform[1];
    case S3C_MISCCR:
        return s->misccr;
    case S3C_DCLKCON:
        return s->dclkcon;
    case S3C_EXTINT0:
        return s->extint[0];
    case S3C_EXTINT1:
        return s->extint[1];
    case S3C_EXTINT2:
        return s->extint[2];
    case S3C_EINTFLT2:
        return s->eintflt[0];
    case S3C_EINTFLT3:
        return s->eintflt[1];
    case S3C_EINTMASK:
        return s->eintmask;
    case S3C_EINTPEND:
        return s->eintpend;
    /* Per bank registers */
    case S3C_GPCON:
        return s->bank[bank].con;
    case S3C_GPDAT:
        return s->bank[bank].dat;
    case S3C_GPUP:
        return s->bank[bank].up;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_gpio_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_gpio_state_s *s = (struct s3c_gpio_state_s *) opaque;
    uint32_t diff;
    int ln, bank = 0;
    addr -= s->base;
    if ((addr >> 4) < S3C_IO_BANKS) {
        bank = addr >> 4;
        addr &= 0xf;
    }

    switch (addr) {
    case S3C_GSTATUS2:
        s->pwrstat &= 7 & ~value;
        break;
    case S3C_GSTATUS3:
        s->inform[0] = value;
        break;
    case S3C_GSTATUS4:
        s->inform[1] = value;
        break;
    case S3C_MISCCR:
        if (value & (1 << 16))				/* nRSTCON */
            printf("%s: software reset.\n", __FUNCTION__);
        if ((value ^ s->misccr) & (1 << 3))		/* USBPAD */
            printf("%s: now in USB %s mode.\n", __FUNCTION__,
                            (value >> 3) & 1 ? "host" : "slave");
        s->misccr = value & 0x000f377b;
        break;
    case S3C_DCLKCON:
        s->dclkcon = value & 0x0ff30ff3;
        break;
    case S3C_EXTINT0:
        s->extint[0] = value;
        break;
    case S3C_EXTINT1:
        s->extint[1] = value;
        break;
    case S3C_EXTINT2:
        s->extint[2] = value;
        break;
    case S3C_EINTFLT2:
        s->eintflt[0] = value;
        break;
    case S3C_EINTFLT3:
        s->eintflt[1] = value;
        break;
    case S3C_EINTMASK:
        s->eintmask = value & 0x00fffff0;
        break;
    case S3C_EINTPEND:
        s->eintpend &= ~value;
        break;
    /* Per bank registers */
    case S3C_GPCON:
        s->bank[bank].con = value;
        break;
    case S3C_GPDAT:
        diff = (s->bank[bank].dat ^ value) & s->bank[bank].mask;
        s->bank[bank].dat = value;
        while ((ln = ffs(diff))) {
            ln --;
            if (s->bank[bank].handler[ln]) {
                if (bank && ((s->bank[bank].con >> (2 * ln)) & 3) == 1)
                    qemu_set_irq(s->bank[bank].handler[ln], (value >> ln) & 1);
                else if (!bank && ((s->bank[bank].con >> ln) & 1) == 0)
                    qemu_set_irq(s->bank[bank].handler[ln], (value >> ln) & 1);
            }
            diff &= ~(1 << ln);
        }
        break;
    case S3C_GPUP:
        s->bank[bank].up = value;
        break;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_gpio_readfn[] = {
    s3c_gpio_read,
    s3c_gpio_read,
    s3c_gpio_read,
};

static CPUWriteMemoryFunc *s3c_gpio_writefn[] = {
    s3c_gpio_write,
    s3c_gpio_write,
    s3c_gpio_write,
};

struct s3c_gpio_state_s *s3c_gpio_init(target_phys_addr_t base, qemu_irq *pic)
{
    int iomemtype;
    struct s3c_gpio_state_s *s = (struct s3c_gpio_state_s *)
            qemu_mallocz(sizeof(struct s3c_gpio_state_s));

    s->base = base;
    s->pic = pic;
    s->in = qemu_allocate_irqs(s3c_gpio_set, s, S3C_GP_MAX);

    s->bank[0].n = 23;
    s->bank[1].n = 11;
    s->bank[2].n = 16;
    s->bank[3].n = 16;
    s->bank[4].n = 16;
    s->bank[5].n = 8;
    s->bank[6].n = 16;
    s->bank[7].n = 11;

    s3c_gpio_reset(s);

    iomemtype = cpu_register_io_memory(0, s3c_gpio_readfn,
                    s3c_gpio_writefn, s);
    cpu_register_physical_memory(s->base, 0xffffff, iomemtype);

    return s;
}
