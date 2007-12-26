/*
 * Samsung S3C24xx series MMC/SD/SDIO Host Controller interface.
 *
 * Copyright (c) 2007 OpenMoko, Inc.
 * Author: Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This code is licenced under the GNU GPL v2.
 */

#include "s3c.h"
#include "sd.h"
#include "hw.h"

struct s3c_mmci_state_s {
    target_phys_addr_t base;
    qemu_irq irq;
    qemu_irq *dma;

    struct sd_card_s *card;

    int blklen;
    int blknum;
    int blklen_cnt;
    int blknum_cnt;
    uint8_t fifo[64];
    int fifolen;
    int fifostart;
    int data;

    uint32_t control;
    uint32_t arg;
    uint32_t ccontrol;
    uint32_t cstatus;
    uint32_t dcontrol;
    uint32_t dstatus;
    uint32_t resp[4];
    uint16_t dtimer;
    uint32_t mask;
    uint8_t prescaler;

    uint16_t model;
    const target_phys_addr_t *map;
};

void s3c_mmci_reset(struct s3c_mmci_state_s *s)
{
    if (!s)
        return;

    s->blklen = 0;
    s->blknum = 0;
    s->blklen_cnt = 0;
    s->blknum_cnt = 0;
    s->fifolen = 0;
    s->fifostart = 0;
    s->control = 0;
    s->arg = 0;
    s->ccontrol = 0;
    s->cstatus = 0;
    s->dcontrol = 0;
    s->dstatus = 0;
    s->resp[0] = 0;
    s->resp[1] = 0;
    s->resp[2] = 0;
    s->resp[3] = 0;
    s->dtimer = 0x2000;
    s->mask = 0;
    s->prescaler = 0;
}

static void s3c_mmci_fifo_run(struct s3c_mmci_state_s *s)
{
    int len, dmalevel = 0;
    if (!s->data)
        goto dmaupdate;

    len = s->fifolen;
    if (((s->dcontrol >> 12) & 3) == 2) {			/* DatMode */
        s->dstatus &= ~3;
        s->dstatus |= 1 << 0;					/* RxDatOn */
        while (s->fifolen < 64 && s->blklen_cnt) {
            s->fifo[(s->fifostart + s->fifolen ++) & 63] =
                    sd_read_data(s->card);
            if (!(-- s->blklen_cnt))
                if (-- s->blknum_cnt)
                    s->blklen_cnt = s->blklen;
        }
        if ((s->mask & (1 << 0)) &&				/* RFHalf */
                        s->fifolen > 31 && len < 32)
            qemu_irq_raise(s->irq);
        if ((s->mask & (1 << 1)) &&				/* RFFull */
                        s->fifolen > 63 && len < 64)
            qemu_irq_raise(s->irq);
        if ((s->mask & (1 << 2)) && !s->blklen_cnt)		/* RFLast */
            qemu_irq_raise(s->irq);
        dmalevel = !!s->fifolen;
    } else if (((s->dcontrol >> 12) & 3) == 3) {		/* DatMode */
        s->dstatus &= ~3;
        s->dstatus |= 1 << 0;					/* TxDatOn */
        while (s->fifolen && s->blklen_cnt) {
            sd_write_data(s->card, s->fifo[s->fifostart ++]);
            s->fifostart &= 63;
            s->fifolen --;
            if (!(-- s->blklen_cnt))
                if (-- s->blknum_cnt)
                    s->blklen_cnt = s->blklen;
        }
        if ((s->mask & (1 << 3)) && !s->fifolen && len)		/* TFEmpty */
            qemu_irq_raise(s->irq);
        if ((s->mask & (1 << 4)) &&				/* TFHalf */
                        s->fifolen < 33 && len > 32)
            qemu_irq_raise(s->irq);
        dmalevel = (s->fifolen < 64) && (s->blklen_cnt > 0);
    } else
        return;

    if (!s->blklen_cnt) {
        s->data = 0;
        s->dstatus &= ~3;
        s->dstatus |= 1 << 4;					/* DatFin */
        if (s->mask & (1 << 7))					/* DatFin */
            qemu_irq_raise(s->irq);
    }
dmaupdate:
    if (s->dcontrol & (1 << 15)) {				/* EnDMA */
        qemu_set_irq(s->dma[S3C_RQ_SDI0], dmalevel);
        qemu_set_irq(s->dma[S3C_RQ_SDI1], dmalevel);
        qemu_set_irq(s->dma[S3C_RQ_SDI2], dmalevel);
    }
}

static void s3c_mmci_cmd_submit(struct s3c_mmci_state_s *s)
{
    int rsplen, i;
    struct sd_request_s request;
    uint8_t response[16];

    request.cmd = s->ccontrol & 0x3f;
    request.arg = s->arg;
    request.crc = 0;	/* FIXME */

    rsplen = sd_do_command(s->card, &request, response);
    s->cstatus = (s->cstatus & ~0x11ff) | request.cmd;		/* RspIndex */
    s->cstatus |= 1 << 11;					/* CmdSent */
    if (s->mask & (1 << 16))					/* CmdSent */
        qemu_irq_raise(s->irq);

    memset(s->resp, 0, sizeof(s->resp));
    if (!(s->ccontrol & (1 << 9)))				/* WaitRsp */
        goto complete;	/* No response */

    if (s->ccontrol & (1 << 10)) {				/* LongRsp */
        /* R2 */
        if (rsplen < 16)
            goto timeout;
    } else {
        /* R1, R3, R4, R5 or R6 */
        if (rsplen < 4)
            goto timeout;
    }

    for (i = 0; i < rsplen; i ++)
        s->resp[i >> 2] |= response[i] << ((~i & 3) << 3);

    s->cstatus |= 1 << 9;					/* RspFin */
    if (s->mask & (1 << 14))					/* RspEnd */
        qemu_irq_raise(s->irq);

complete:
    s->blklen_cnt = s->blklen;
    s->blknum_cnt = s->blknum;

    if (((s->dcontrol >> 12) & 3) == 1) {			/* DatMode */
        if (s->dcontrol & (1 << 18))				/* BACMD */
            s->data = 1;
    } else if (((s->dcontrol >> 12) & 3) == 2) {		/* DatMode */
        if (s->dcontrol & (1 << 19))				/* RACMD */
            s->data = 1;
    } else if (((s->dcontrol >> 12) & 3) == 3)			/* DatMode */
        if (s->dcontrol & (1 << 20))				/* RACMD */
            s->data = 1;
    return;

timeout:
    s->cstatus |= 1 << 10;					/* CmdTout */
    if (s->mask & (1 << 15))					/* CmdTout */
        qemu_irq_raise(s->irq);
}

#define S3C_SDICON	0x00	/* SDI Control register */
#define S3C_SDIPRE	0x04	/* SDI Baud Rate Prescaler register */
#define S3C_SDICARG	0x08	/* SDI Command Argument register */
#define S3C_SDICCON	0x0c	/* SDI Command Control register */
#define S3C_SDICSTA	0x10	/* SDI Command Status register */
#define S3C_SDIRSP0	0x14	/* SDI Response register 0 */
#define S3C_SDIRSP1	0x18	/* SDI Response register 1 */
#define S3C_SDIRSP2	0x1c	/* SDI Response register 2 */
#define S3C_SDIRSP3	0x20	/* SDI Response register 3 */
#define S3C_SDIDTIMER	0x24	/* SDI Data / Busy Timer register */
#define S3C_SDIBSIZE	0x28	/* SDI Block Size register */
#define S3C_SDIDCON	0x2c	/* SDI Data Control register */
#define S3C_SDICNT	0x30	/* SDI Data Remain Counter register */
#define S3C_SDIDSTA	0x34	/* SDI Data Status register */
#define S3C_SDIFSTA	0x38	/* SDI FIFO Status register */
#define S3C_SDIDAT	0x3c	/* SDI Data register */
#define S3C_SDIMSK	0x40	/* SDI Interrupt Mask register */

#define S3C_SDIMAX	0x40

static uint32_t s3c_mmci_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_mmci_state_s *s = (struct s3c_mmci_state_s *) opaque;
    uint32_t ret;
    addr -= s->base;
    if (addr > S3C_SDIMAX)
        goto bad_reg;
    addr = s->map[addr];

    switch (addr) {
    case S3C_SDICON:
        return s->control;
    case S3C_SDIPRE:
        return s->prescaler;
    case S3C_SDICARG:
        return s->arg;
    case S3C_SDICCON:
        return s->ccontrol;
    case S3C_SDICSTA:
        return s->cstatus;
    case S3C_SDIRSP0:
        return s->resp[0];
    case S3C_SDIRSP1:
        return s->resp[1];
    case S3C_SDIRSP2:
        return s->resp[2];
    case S3C_SDIRSP3:
        return s->resp[3];
    case S3C_SDIDTIMER:
        return s->dtimer;
    case S3C_SDIBSIZE:
        return s->blklen;
    case S3C_SDIDCON:
        return s->dcontrol;
    case S3C_SDICNT:
        return s->blklen_cnt | (s->blknum_cnt << 12);
    case S3C_SDIDSTA:
        return s->dstatus;
    case S3C_SDIFSTA:
        /* XXX on S3C2440 these bits have to cleared explicitely.  */
        if (((s->dcontrol >> 12) & 3) == 2)			/* DatMode */
            return s->fifolen |					/* FFCNT */
                ((s->fifolen > 31) ? (1 << 7) : 0) |		/* RFHalf */
                ((s->fifolen > 63) ? (1 << 8) : 0) |		/* RFFull */
                (s->blklen_cnt ? 0 : (1 << 9)) |		/* RFLast */
                (3 << 10) |					/* TFHalf */
                (s->fifolen ? (1 << 12) : 0);			/* RFDET */
        else if (((s->dcontrol >> 12) & 3) == 3)		/* DatMode */
            return s->fifolen |					/* FFCNT */
                (s->fifolen ? 0 : (1 << 10)) |			/* TFEmpty */
                ((s->fifolen < 33) ? (1 << 11) : 0) |		/* TFHalf */
                ((s->fifolen < 64) ? (1 << 13) : 0);		/* TFDET */
        else
            return s->fifolen;					/* FFCNT */
    case S3C_SDIDAT:
        /* TODO: 8/16-bit access */
        ret = 0;
        if (s->fifolen >= 4) {
            ret |= s->fifo[s->fifostart ++] << 0;
            s->fifostart &= 63;
            ret |= s->fifo[s->fifostart ++] << 8;
            s->fifostart &= 63;
            ret |= s->fifo[s->fifostart ++] << 16;
            s->fifostart &= 63;
            ret |= s->fifo[s->fifostart ++] << 24;
            s->fifostart &= 63;
            s->fifolen -= 4;
            s3c_mmci_fifo_run(s);
        } else
            printf("%s: FIFO underrun\n", __FUNCTION__);

        return ret;
    case S3C_SDIMSK:
        return s->mask;
    default:
    bad_reg:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_mmci_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_mmci_state_s *s = (struct s3c_mmci_state_s *) opaque;
    addr -= s->base;
    if (addr > S3C_SDIMAX)
        goto bad_reg;
    addr = s->map[addr];

    switch (addr) {
    case S3C_SDICON:
        s->control = value & 0x1d;
        if (value & (1 << 1))					/* FRST */
            s->fifolen = 0;
        break;
    case S3C_SDIPRE:
        s->prescaler = value & 0xff;
        break;
    case S3C_SDICARG:
        s->arg = value;
        break;
    case S3C_SDICCON:
        s->ccontrol = value & 0x1fff;
        if (value & (1 << 8))					/* CMST */
            s3c_mmci_cmd_submit(s);
        s3c_mmci_fifo_run(s);
        break;
    case S3C_SDICSTA:
        s->cstatus &= ~(value & 0x1e00);
        break;
    case S3C_SDIDTIMER:
        s->dtimer = value & 0xffff;
        break;
    case S3C_SDIBSIZE:
        s->blklen = value & 0xfff;
        break;
    case S3C_SDIDCON:
        s->dcontrol = value;
        s->blknum = value & 0xfff;
        if (value & (1 << 14))					/* STOP */
            s->data = 0;
        if (((s->dcontrol >> 12) & 3) == 1) {			/* DatMode */
            if (!(s->dcontrol & (1 << 18)))			/* BACMD */
                s->data = 1;
	} else if (((s->dcontrol >> 12) & 3) == 2) {		/* DatMode */
            if (!(s->dcontrol & (1 << 19)))			/* RACMD */
                s->data = 1;
	} else if (((s->dcontrol >> 12) & 3) == 3)		/* DatMode */
            if (!(s->dcontrol & (1 << 20)))			/* RACMD */
                s->data = 1;
        s3c_mmci_fifo_run(s);
        break;
    case S3C_SDIDSTA:
        s->dstatus &= ~(value & 0x3f8);
        break;
    case S3C_SDIDAT:
        /* TODO: 8/16-bit access */
        s->fifo[(s->fifostart + s->fifolen ++) & 63] = (value >> 0) & 0xff;
        s->fifo[(s->fifostart + s->fifolen ++) & 63] = (value >> 8) & 0xff;
        s->fifo[(s->fifostart + s->fifolen ++) & 63] = (value >> 16) & 0xff;
        s->fifo[(s->fifostart + s->fifolen ++) & 63] = (value >> 24) & 0xff;
        s3c_mmci_fifo_run(s);
        break;
    case S3C_SDIMSK:
        s->mask = value & 0x3ffff;
        break;
    default:
    bad_reg:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_mmci_readfn[] = {
    s3c_mmci_read,
    s3c_mmci_read,
    s3c_mmci_read,
};

static CPUWriteMemoryFunc *s3c_mmci_writefn[] = {
    s3c_mmci_write,
    s3c_mmci_write,
    s3c_mmci_write,
};

static void s3c_mmci_cardirq(void *opaque, int line, int level)
{
    struct s3c_mmci_state_s *s = (struct s3c_mmci_state_s *) opaque;

    if (~s->control & (1 << 3))					/* RcvIOInt */
        return;
    if (!level)
        return;

    s->dstatus |= 1 << 9;					/* IOIntDet */
    if (s->mask & (1 << 12))					/* IOIntDet */
        qemu_irq_raise(s->irq);
}

static void s3c_mmci_save(QEMUFile *f, void *opaque)
{
    struct s3c_mmci_state_s *s = (struct s3c_mmci_state_s *) opaque;
    qemu_put_be32(f, s->blklen);
    qemu_put_be32(f, s->blknum);
    qemu_put_be32(f, s->blklen_cnt);
    qemu_put_be32(f, s->blknum_cnt);
    qemu_put_buffer(f, s->fifo, sizeof(s->fifo));
    qemu_put_be32(f, s->fifolen);
    qemu_put_be32(f, s->fifostart);
    qemu_put_be32(f, s->data);

    qemu_put_be32s(f, &s->control);
    qemu_put_be32s(f, &s->arg);
    qemu_put_be32s(f, &s->ccontrol);
    qemu_put_be32s(f, &s->cstatus);
    qemu_put_be32s(f, &s->dcontrol);
    qemu_put_be32s(f, &s->dstatus);
    qemu_put_be32s(f, &s->resp[0]);
    qemu_put_be32s(f, &s->resp[1]);
    qemu_put_be32s(f, &s->resp[2]);
    qemu_put_be32s(f, &s->resp[3]);
    qemu_put_be16s(f, &s->dtimer);
    qemu_put_be32s(f, &s->mask);
    qemu_put_8s(f, &s->prescaler);
}

static const target_phys_addr_t s3c2410_regmap[S3C_SDIMAX + 1] = {
    [0 ... S3C_SDIMAX] = -1,
    [0x00] = S3C_SDICON,
    [0x04] = S3C_SDIPRE,
    [0x08] = S3C_SDICARG,
    [0x0c] = S3C_SDICCON,
    [0x10] = S3C_SDICSTA,
    [0x14] = S3C_SDIRSP0,
    [0x18] = S3C_SDIRSP1,
    [0x1c] = S3C_SDIRSP2,
    [0x20] = S3C_SDIRSP3,
    [0x24] = S3C_SDIDTIMER,
    [0x28] = S3C_SDIBSIZE,
    [0x2c] = S3C_SDIDCON,
    [0x30] = S3C_SDICNT,
    [0x34] = S3C_SDIDSTA,
    [0x38] = S3C_SDIFSTA,
    [0x3c] = S3C_SDIDAT,
    [0x40] = S3C_SDIMSK,
};

static const target_phys_addr_t s3c2440_regmap[S3C_SDIMAX + 1] = {
    [0 ... S3C_SDIMAX] = -1,
    [0x00] = S3C_SDICON,
    [0x04] = S3C_SDIPRE,
    [0x08] = S3C_SDICARG,
    [0x0c] = S3C_SDICCON,
    [0x10] = S3C_SDICSTA,
    [0x14] = S3C_SDIRSP0,
    [0x18] = S3C_SDIRSP1,
    [0x1c] = S3C_SDIRSP2,
    [0x20] = S3C_SDIRSP3,
    [0x24] = S3C_SDIDTIMER,
    [0x28] = S3C_SDIBSIZE,
    [0x2c] = S3C_SDIDCON,
    [0x30] = S3C_SDICNT,
    [0x34] = S3C_SDIDSTA,
    [0x38] = S3C_SDIFSTA,
    [0x3c] = S3C_SDIMSK,
    [0x40] = S3C_SDIDAT,
};

static int s3c_mmci_load(QEMUFile *f, void *opaque, int version_id)
{
    struct s3c_mmci_state_s *s = (struct s3c_mmci_state_s *) opaque;
    s->blklen = qemu_get_be32(f);
    s->blknum = qemu_get_be32(f);
    s->blklen_cnt = qemu_get_be32(f);
    s->blknum_cnt = qemu_get_be32(f);
    qemu_get_buffer(f, s->fifo, sizeof(s->fifo));
    s->fifolen = qemu_get_be32(f);
    s->fifostart = qemu_get_be32(f);
    s->data = qemu_get_be32(f);

    qemu_get_be32s(f, &s->control);
    qemu_get_be32s(f, &s->arg);
    qemu_get_be32s(f, &s->ccontrol);
    qemu_get_be32s(f, &s->cstatus);
    qemu_get_be32s(f, &s->dcontrol);
    qemu_get_be32s(f, &s->dstatus);
    qemu_get_be32s(f, &s->resp[0]);
    qemu_get_be32s(f, &s->resp[1]);
    qemu_get_be32s(f, &s->resp[2]);
    qemu_get_be32s(f, &s->resp[3]);
    qemu_get_be16s(f, &s->dtimer);
    qemu_get_be32s(f, &s->mask);
    qemu_get_8s(f, &s->prescaler);

    return 0;
}

struct s3c_mmci_state_s *s3c_mmci_init(target_phys_addr_t base, uint16_t model,
                struct sd_card_s *mmc, qemu_irq irq, qemu_irq *dma)
{
    int iomemtype;
    struct s3c_mmci_state_s *s;

    if (!mmc)
        return 0;

    s = (struct s3c_mmci_state_s *)
            qemu_mallocz(sizeof(struct s3c_mmci_state_s));
    s->base = base;
    s->irq = irq;
    s->dma = dma;
    s->card = mmc;
    s->model = model;
    switch (model) {
    case 0x2410:
        s->map = s3c2410_regmap;
        break;
    case 0x2440:
        s->map = s3c2440_regmap;
        break;
    default:
        fprintf(stderr, "%s: unknown MMC/SD/SDIO HC model %04x\n",
                        __FUNCTION__, model);
        exit(-1);
    }

    mmc->irq = qemu_allocate_irqs(s3c_mmci_cardirq, s, 1)[0];

    s3c_mmci_reset(s);

    iomemtype = cpu_register_io_memory(0, s3c_mmci_readfn,
                    s3c_mmci_writefn, s);
    cpu_register_physical_memory(s->base, 0xffffff, iomemtype);

    register_savevm("s3c24xx_mmci", 0, 0, s3c_mmci_save, s3c_mmci_load, s);

    return s;
}
