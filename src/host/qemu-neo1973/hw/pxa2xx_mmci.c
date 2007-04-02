/*
 * Intel XScale PXA255/270 MultiMediaCard/SD/SDIO Controller emulation.
 *
 * Copyright (c) 2006 Openedhand Ltd.
 * Written by Andrzej Zaborowski <balrog@zabor.org>
 *
 * This code is licensed under the GPLv2.
 */

#include "vl.h"
#include "sd.h"

struct pxa2xx_mmci_s {
    target_phys_addr_t base;
    void *pic;
    void *dma;

    struct sd_state_s *card;

    uint32_t status;
    uint32_t clkrt;
    uint32_t spi;
    uint32_t cmdat;
    uint32_t resp_tout;
    uint32_t read_tout;
    int blklen;
    int numblk;
    uint32_t intmask;
    uint32_t intreq;
    int cmd;
    uint32_t arg;

    int active;
    int bytesleft;
    uint8_t tx_fifo[64];
    int tx_start;
    int tx_len;
    uint8_t rx_fifo[32];
    int rx_start;
    int rx_len;
    uint16_t resp_fifo[9];
    int resp_len;

    int cmdreq;
    int ac_width;
};

#define MMC_STRPCL	0x00	/* MMC Clock Start/Stop Register */
#define MMC_STAT	0x04	/* MMC Status Register */
#define MMC_CLKRT	0x08	/* MMC Clock Rate Register */
#define MMC_SPI		0x0c	/* MMC SPI Mode Register */
#define MMC_CMDAT	0x10	/* MMC Command/Data Register */
#define MMC_RESTO	0x14	/* MMC Response Time-Out Register */
#define MMC_RDTO	0x18	/* MMC Read Time-Out Register */
#define MMC_BLKLEN	0x1c	/* MMC Block Length Register */
#define MMC_NUMBLK	0x20	/* MMC Number of Blocks Register */
#define MMC_PRTBUF	0x24	/* MMC Buffer Partly Full Register */
#define MMC_I_MASK	0x28	/* MMC Interrupt Mask Register */
#define MMC_I_REG	0x2c	/* MMC Interrupt Request Register */
#define MMC_CMD		0x30	/* MMC Command Register */
#define MMC_ARGH	0x34	/* MMC Argument High Register */
#define MMC_ARGL	0x38	/* MMC Argument Low Register */
#define MMC_RES		0x3c	/* MMC Response FIFO */
#define MMC_RXFIFO	0x40	/* MMC Receive FIFO */
#define MMC_TXFIFO	0x44	/* MMC Transmit FIFO */
#define MMC_RDWAIT	0x48	/* MMC RD_WAIT Register */
#define MMC_BLKS_REM	0x4c	/* MMC Blocks Remaining Register */

/* Bitfield masks */
#define STRPCL_STOP_CLK	(1 << 0)
#define STRPCL_STRT_CLK	(1 << 1)
#define STAT_TOUT_RES	(1 << 1)
#define STAT_CLK_EN	(1 << 8)
#define STAT_DATA_DONE	(1 << 11)
#define STAT_PRG_DONE	(1 << 12)
#define STAT_END_CMDRES	(1 << 13)
#define SPI_SPI_MODE	(1 << 0)
#define CMDAT_RES_TYPE	(3 << 0)
#define CMDAT_DATA_EN	(1 << 2)
#define CMDAT_WR_RD	(1 << 3)
#define CMDAT_DMA_EN	(1 << 7)
#define CMDAT_STOP_TRAN	(1 << 10)
#define INT_DATA_DONE	(1 << 0)
#define INT_PRG_DONE	(1 << 1)
#define INT_END_CMD	(1 << 2)
#define INT_STOP_CMD	(1 << 3)
#define INT_CLK_OFF	(1 << 4)
#define INT_RXFIFO_REQ	(1 << 5)
#define INT_TXFIFO_REQ	(1 << 6)
#define INT_TINT	(1 << 7)
#define INT_DAT_ERR	(1 << 8)
#define INT_RES_ERR	(1 << 9)
#define INT_RD_STALLED	(1 << 10)
#define INT_SDIO_INT	(1 << 11)
#define INT_SDIO_SACK	(1 << 12)
#define PRTBUF_PRT_BUF	(1 << 0)

/* Route internal interrupt lines to the global IC and DMA */
static void pxa2xx_mmci_int_update(struct pxa2xx_mmci_s *s)
{
    uint32_t mask = s->intmask;
    if (s->cmdat & CMDAT_DMA_EN) {
        mask |= INT_RXFIFO_REQ | INT_TXFIFO_REQ;

        pic_set_irq_new(s->dma, PXA2XX_RX_RQ_MMCI,
                        !!(s->intreq & INT_RXFIFO_REQ));
        pic_set_irq_new(s->dma, PXA2XX_TX_RQ_MMCI,
                        !!(s->intreq & INT_TXFIFO_REQ));
    }

    pic_set_irq_new(s->pic, PXA2XX_PIC_MMC, !!(s->intreq & ~mask));
}

static void pxa2xx_mmci_fifo_update(struct pxa2xx_mmci_s *s)
{
    if (!s->active)
        return;

    if (s->cmdat & CMDAT_WR_RD) {
        while (s->bytesleft && s->tx_len) {
            sd_write_datline(s->card, s->tx_fifo[s->tx_start ++]);
            s->tx_start &= 0x1f;
            s->tx_len --;
            s->bytesleft --;
        }
        if (s->bytesleft)
            s->intreq |= INT_TXFIFO_REQ;
    } else
        while (s->bytesleft && s->rx_len < 32) {
            s->rx_fifo[(s->rx_start + (s->rx_len ++)) & 0x1f] =
                sd_read_datline(s->card);
            s->bytesleft --;
            s->intreq |= INT_RXFIFO_REQ;
        }

    if (!s->bytesleft) {
        s->active = 0;
        s->intreq |= INT_DATA_DONE;
        s->status |= STAT_DATA_DONE;

        if (s->cmdat & CMDAT_WR_RD) {
            s->intreq |= INT_PRG_DONE;
            s->status |= STAT_PRG_DONE;
        }
    }

    pxa2xx_mmci_int_update(s);
}

static void pxa2xx_mmci_wakequeues(struct pxa2xx_mmci_s *s)
{
    int rsplen;
    struct sd_request_s request;
    union sd_response_u response;

    s->active = 1;
    s->rx_len = 0;
    s->tx_len = 0;
    s->cmdreq = 0;

    request.cmd = s->cmd;
    request.arg = s->arg;
    request.crc = 0;	/* FIXME */

    response = sd_write_cmdline(s->card, request, &rsplen);
    s->intreq |= INT_END_CMD;

    memset(s->resp_fifo, 0, sizeof(s->resp_fifo));
    switch (s->cmdat & CMDAT_RES_TYPE) {
#define PXAMMCI_RESP(wd, value)	\
        s->resp_fifo[(wd) + 0] |= (value) >> 8;	\
        s->resp_fifo[(wd) + 1] |= (value) << 8;
    case 0:	/* No response */
        goto complete;

    case 1:	/* R1, R4, R5 or R6 */
        if (rsplen < 48)
            goto timeout;

        if (request.cmd == 3) {	/* R6 */
            PXAMMCI_RESP(0, response.r6.arg);
            PXAMMCI_RESP(1, response.r6.status);
        } else {
            PXAMMCI_RESP(0, response.r1.status >> 16);
            PXAMMCI_RESP(1, response.r1.status & 0x0000ffff);
        }
        goto complete;

    case 2:	/* R2 */
        if (rsplen < 128)
            goto timeout;

        PXAMMCI_RESP(0, bswap16(response.r2.reg[0]));
        PXAMMCI_RESP(1, bswap16(response.r2.reg[1]));
        PXAMMCI_RESP(2, bswap16(response.r2.reg[2]));
        PXAMMCI_RESP(3, bswap16(response.r2.reg[3]));
        PXAMMCI_RESP(4, bswap16(response.r2.reg[4]));
        PXAMMCI_RESP(5, bswap16(response.r2.reg[5]));
        PXAMMCI_RESP(6, bswap16(response.r2.reg[6]));
        PXAMMCI_RESP(7, bswap16(response.r2.reg[7]));
        goto complete;

    case 3:	/* R3 */
        if (rsplen < 32)
            goto timeout;

        PXAMMCI_RESP(0, response.r3.ocr_reg >> 16);
        PXAMMCI_RESP(1, response.r3.ocr_reg & 0x0000ffff);
        goto complete;

    complete:
        s->status |= STAT_END_CMDRES;

        if (!(s->cmdat & CMDAT_DATA_EN))
            s->active = 0;
        else
            s->bytesleft = s->numblk * s->blklen;

        s->resp_len = 0;
        break;

    timeout:
        s->active = 0;
        s->status |= STAT_TOUT_RES;
        break;
    }

    pxa2xx_mmci_fifo_update(s);
}

static uint32_t pxa2xx_mmci_read(void *opaque, target_phys_addr_t offset)
{
    struct pxa2xx_mmci_s *s = (struct pxa2xx_mmci_s *) opaque;
    uint32_t ret;
    offset -= s->base;

    switch (offset) {
    case MMC_STRPCL:
        return 0;
    case MMC_STAT:
        return s->status;
    case MMC_CLKRT:
        return s->clkrt;
    case MMC_SPI:
        return s->spi;
    case MMC_CMDAT:
        return s->cmdat;
    case MMC_RESTO:
        return s->resp_tout;
    case MMC_RDTO:
        return s->read_tout;
    case MMC_BLKLEN:
        return s->blklen;
    case MMC_NUMBLK:
        return s->numblk;
    case MMC_PRTBUF:
        return 0;
    case MMC_I_MASK:
        return s->intmask;
    case MMC_I_REG:
        return s->intreq;
    case MMC_CMD:
        return s->cmd | 0x40;
    case MMC_ARGH:
        return s->arg >> 16;
    case MMC_ARGL:
        return s->arg & 0xffff;
    case MMC_RES:
        if (s->resp_len < 9)
            return s->resp_fifo[s->resp_len ++];
        return 0;
    case MMC_RXFIFO:
        ret = 0;
        while (s->ac_width -- && s->rx_len) {
            ret |= s->rx_fifo[s->rx_start ++] << (s->ac_width << 3);
            s->rx_start &= 0x1f;
            s->rx_len --;
        }
        s->intreq &= ~INT_RXFIFO_REQ;
        pxa2xx_mmci_fifo_update(s);
        return ret;
    case MMC_RDWAIT:
        return 0;
    case MMC_BLKS_REM:
        return s->numblk;
    default:
        cpu_abort(cpu_single_env, "%s: Bad offset %x\n", __FUNCTION__, offset);
    }

    return 0;
}

static void pxa2xx_mmci_write(void *opaque,
                target_phys_addr_t offset, uint32_t value)
{
    struct pxa2xx_mmci_s *s = (struct pxa2xx_mmci_s *) opaque;
    offset -= s->base;

    switch (offset) {
    case MMC_STRPCL:
        if (value & STRPCL_STRT_CLK) {
            s->status |= STAT_CLK_EN;
            s->intreq &= ~INT_CLK_OFF;

            if (s->cmdreq && !(s->cmdat & CMDAT_STOP_TRAN)) {
                s->status &= STAT_CLK_EN;
                pxa2xx_mmci_wakequeues(s);
            }
        }

        if (value & STRPCL_STOP_CLK) {
            s->status &= ~STAT_CLK_EN;
            s->intreq |= INT_CLK_OFF;
            s->active = 0;
        }

        pxa2xx_mmci_int_update(s);
        break;

    case MMC_CLKRT:
        s->clkrt = value & 7;
        break;

    case MMC_SPI:
        s->spi = value & 0xf;
        if (value & SPI_SPI_MODE)
            printf("%s: attempted to use card in SPI mode\n", __FUNCTION__);
        break;

    case MMC_CMDAT:
        s->cmdat = value & 0x3dff;
        s->active = 0;
        s->cmdreq = 1;
        if (!(value & CMDAT_STOP_TRAN)) {
            s->status &= STAT_CLK_EN;

            if (s->status & STAT_CLK_EN)
                pxa2xx_mmci_wakequeues(s);
        }

        pxa2xx_mmci_int_update(s);
        break;

    case MMC_RESTO:
        s->resp_tout = value & 0x7f;
        break;

    case MMC_RDTO:
        s->read_tout = value & 0xffff;
        break;

    case MMC_BLKLEN:
        s->blklen = value & 0xfff;
        break;

    case MMC_NUMBLK:
        s->numblk = value & 0xffff;
        break;

    case MMC_PRTBUF:
        if (value & PRTBUF_PRT_BUF) {
            s->tx_start ^= 32;
            s->tx_len = 0;
        }
        pxa2xx_mmci_fifo_update(s);
        break;

    case MMC_I_MASK:
        s->intmask = value & 0x1fff;
        pxa2xx_mmci_int_update(s);
        break;

    case MMC_CMD:
        s->cmd = value & 0x3f;
        break;

    case MMC_ARGH:
        s->arg &= 0x0000ffff;
        s->arg |= value << 16;
        break;

    case MMC_ARGL:
        s->arg &= 0xffff0000;
        s->arg |= value & 0x0000ffff;
        break;

    case MMC_TXFIFO:
        while (s->ac_width -- && s->tx_len < 0x20)
            s->tx_fifo[(s->tx_start + (s->tx_len ++)) & 0x1f] =
                    (value >> (s->ac_width << 3)) & 0xff;
        s->intreq &= ~INT_TXFIFO_REQ;
        pxa2xx_mmci_fifo_update(s);
        break;

    case MMC_RDWAIT:
    case MMC_BLKS_REM:
        break;

    default:
        cpu_abort(cpu_single_env, "%s: Bad offset %x\n", __FUNCTION__, offset);
    }
}

static uint32_t pxa2xx_mmci_readb(void *opaque, target_phys_addr_t offset)
{
    struct pxa2xx_mmci_s *s = (struct pxa2xx_mmci_s *) opaque;
    s->ac_width = 1;
    return pxa2xx_mmci_read(opaque, offset);
}

static uint32_t pxa2xx_mmci_readh(void *opaque, target_phys_addr_t offset)
{
    struct pxa2xx_mmci_s *s = (struct pxa2xx_mmci_s *) opaque;
    s->ac_width = 2;
    return pxa2xx_mmci_read(opaque, offset);
}

static uint32_t pxa2xx_mmci_readw(void *opaque, target_phys_addr_t offset)
{
    struct pxa2xx_mmci_s *s = (struct pxa2xx_mmci_s *) opaque;
    s->ac_width = 4;
    return pxa2xx_mmci_read(opaque, offset);
}

static CPUReadMemoryFunc *pxa2xx_mmci_readfn[] = {
    pxa2xx_mmci_readb,
    pxa2xx_mmci_readh,
    pxa2xx_mmci_readw
};

static void pxa2xx_mmci_writeb(void *opaque,
                target_phys_addr_t offset, uint32_t value)
{
    struct pxa2xx_mmci_s *s = (struct pxa2xx_mmci_s *) opaque;
    s->ac_width = 1;
    pxa2xx_mmci_write(opaque, offset, value);
}

static void pxa2xx_mmci_writeh(void *opaque,
                target_phys_addr_t offset, uint32_t value)
{
    struct pxa2xx_mmci_s *s = (struct pxa2xx_mmci_s *) opaque;
    s->ac_width = 2;
    pxa2xx_mmci_write(opaque, offset, value);
}

static void pxa2xx_mmci_writew(void *opaque,
                target_phys_addr_t offset, uint32_t value)
{
    struct pxa2xx_mmci_s *s = (struct pxa2xx_mmci_s *) opaque;
    s->ac_width = 4;
    pxa2xx_mmci_write(opaque, offset, value);
}

static CPUWriteMemoryFunc *pxa2xx_mmci_writefn[] = {
    pxa2xx_mmci_writeb,
    pxa2xx_mmci_writeh,
    pxa2xx_mmci_writew
};

struct pxa2xx_mmci_s *pxa2xx_mmci_init(target_phys_addr_t base,
                void *pic, void *dma)
{
    int iomemtype;
    struct pxa2xx_mmci_s *s;

    s = (struct pxa2xx_mmci_s *) qemu_mallocz(sizeof(struct pxa2xx_mmci_s));
    s->base = base;
    s->pic = pic;
    s->dma = dma;

    iomemtype = cpu_register_io_memory(0, pxa2xx_mmci_readfn,
                    pxa2xx_mmci_writefn, s);
    cpu_register_physical_memory(base, 0x000fffff, iomemtype);

    /* Instantiate the actual storage */
    s->card = sd_init();

    return s;
}

void pxa2xx_mmci_handlers(struct pxa2xx_mmci_s *s, void *opaque,
                void (*readonly_cb)(void *, int),
                void (*coverswitch_cb)(void *, int))
{
    sd_set_cb(s->card, opaque, readonly_cb, coverswitch_cb);
}
