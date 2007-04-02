/*
 * Intel XScale PXA255/270 OS Timers.
 *
 * Copyright (c) 2006 Openedhand Ltd.
 * Copyright (c) 2006 Thorsten Zitterell
 *
 * This code is licenced under the GPL.
 */

#include "vl.h"

#define OSMR0	0x00
#define OSMR1	0x04
#define OSMR2	0x08
#define OSMR3	0x0c
#define OSMR4	0x80
#define OSCR	0x10	/* OS Timer Count */
#define OSCR4	0x40
#define OMCR4	0xc0
#define OSSR	0x14	/* Timer status register */
#define OWER	0x18
#define OIER	0x1c	/* Interrupt enable register  3-0 to E3-E0 */

#define PXA25X_FREQ	3686400	/* 3.6864 MHz */
#define PXA27X_FREQ	3250000	/* 3.25 MHz */

typedef struct {
    uint32_t value;
    int level;
    int irq;
    void *pic;
    QEMUTimer *qtimer;
    int num;
    void *info;
} pxa2xx_timer;

typedef struct {
    uint32_t base;
    int32_t clock;
    int32_t oldclock;
    uint64_t lastload;
    uint32_t freq;
    pxa2xx_timer timer[4];
    uint32_t events;
    uint32_t irq_enabled;
    uint32_t reset3;
    CPUState *cpustate;
    int64_t qemu_ticks;
} pxa2xx_timer_info;

static void pxa2xx_timer_update(void *opaque, uint64_t now_qemu)
{
    pxa2xx_timer_info *s = (pxa2xx_timer_info *) opaque;
    int i;
    uint32_t now_vm;
    uint64_t new_qemu;

    now_vm = s->clock +
            muldiv64(now_qemu - s->lastload, s->freq, ticks_per_sec);

    for (i = 0; i < 4; i ++) {
        new_qemu = now_qemu + muldiv64((uint32_t) (s->timer[i].value - now_vm),
                        ticks_per_sec, s->freq);
        qemu_mod_timer(s->timer[i].qtimer, new_qemu);
    }
}

static uint32_t pxa2xx_timer_read(void *opaque, target_phys_addr_t offset)
{
    pxa2xx_timer_info *s = (pxa2xx_timer_info *) opaque;

    offset -= s->base;

    switch (offset) {
    case OSMR0:
    case OSMR1:
    case OSMR2:
    case OSMR3:
	return s->timer[offset >> 2].value;
    case OSCR:
        return s->clock + muldiv64(qemu_get_clock(vm_clock) -
                        s->lastload, s->freq, ticks_per_sec);
    case OIER:
        return s->irq_enabled;
    case OWER:
        return s->reset3;
    default:
        cpu_abort(cpu_single_env,
                        "pxa2xx_timer_read: Bad offset %x\n", offset);
    }

    return 0;
}

static void pxa2xx_timer_write(void *opaque, target_phys_addr_t offset,
                uint32_t value)
{
    int i;
    pxa2xx_timer_info *s = (pxa2xx_timer_info *) opaque;

    offset -= s->base;

    switch (offset) {
    case OSMR0:
    case OSMR1:
    case OSMR2:
    case OSMR3:
        s->timer[offset >> 2].value = value;
        pxa2xx_timer_update(s, qemu_get_clock(vm_clock));
        break;
    case OSCR:
        s->oldclock = s->clock;
        s->lastload = qemu_get_clock(vm_clock);
        s->clock = value;
        pxa2xx_timer_update(s, s->lastload);
        break;
    case OIER:
        s->irq_enabled = value;
        break;
    case OSSR:	/* Status register */
        s->events &= ~value;
        for (i = 0; i < 4; i++) {
            if (s->timer[i].level && (value & (1 << i))) {
                s->timer[i].level = 0;
                pic_set_irq_new(s->timer[i].pic, s->timer[i].irq, 0);
            }
        }
        break;
    case OWER:	/* XXX: Reset on OSMR3 match? */
        s->reset3 = value;
        break;
    default:
        cpu_abort(cpu_single_env,
                        "pxa2xx_timer_write: Bad offset %x\n", offset);
    }
}

static CPUReadMemoryFunc *pxa2xx_timer_readfn[] = {
    pxa2xx_timer_read,
    pxa2xx_timer_read,
    pxa2xx_timer_read,
};

static CPUWriteMemoryFunc *pxa2xx_timer_writefn[] = {
    pxa2xx_timer_write,
    pxa2xx_timer_write,
    pxa2xx_timer_write,
};

static void pxa2xx_timer_tick(void *opaque)
{
    pxa2xx_timer *t = (pxa2xx_timer *) opaque;
    pxa2xx_timer_info *i = (pxa2xx_timer_info *) t->info;

    if (i->irq_enabled & (1 << t->num)) {
        t->level = 1;
        pic_set_irq_new(t->pic, t->irq, 1);
    }

    if (t->num == 3)
        if (i->reset3 & 1) {
            i->reset3 = 0;
            cpu_reset(i->cpustate);
        }
}

static pxa2xx_timer_info *pxa2xx_timer_init(target_phys_addr_t base,
                void *pic, int irq, CPUState *cpustate)
{
    int i;
    int iomemtype;
    pxa2xx_timer_info *s;

    s = (pxa2xx_timer_info *) qemu_mallocz(sizeof(pxa2xx_timer_info));
    s->base = base;
    s->irq_enabled = 0;
    s->oldclock = 0;
    s->clock = 0;
    s->lastload = qemu_get_clock(vm_clock);
    s->reset3 = 0;
    s->cpustate = cpustate;

    for (i = 0; i < 4; i ++) {
        s->timer[i].value = 0;
        s->timer[i].irq = irq + i;
        s->timer[i].pic = pic;
        s->timer[i].info = s;
        s->timer[i].num = i;
        s->timer[i].level = 0;
        s->timer[i].qtimer = qemu_new_timer(vm_clock,
                        pxa2xx_timer_tick, &s->timer[i]);
    }

    iomemtype = cpu_register_io_memory(0, pxa2xx_timer_readfn,
                    pxa2xx_timer_writefn, s);
    cpu_register_physical_memory(base, 0x00000fff, iomemtype);
    return s;
}

void pxa25x_timer_init(target_phys_addr_t base,
                void *pic, int irq, CPUState *cpustate)
{
    pxa2xx_timer_info *s = pxa2xx_timer_init(base, pic, irq, cpustate);
    s->freq = PXA25X_FREQ;
}

void pxa27x_timer_init(target_phys_addr_t base,
                void *pic, int irq, CPUState *cpustate)
{
    pxa2xx_timer_info *s = pxa2xx_timer_init(base, pic, irq, cpustate);
    s->freq = PXA27X_FREQ;
}
