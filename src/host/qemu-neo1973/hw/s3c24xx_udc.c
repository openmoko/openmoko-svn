/*
 * Samsung S3C2410A USB Device Controller emulation.
 *
 * Copyright (c) 2007 OpenMoko, Inc.
 * Author: Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This code is licenced under the GNU GPL v2.
 */
#include "vl.h"

#define S3C_USB_FIFO_LEN	4096

struct s3c_udc_state_s {
    target_phys_addr_t base;
    USBDevice dev;
    void *pic;
    void *dma;

    /* Use FIFOs big enough to hold entire packets, just don't report
     * lengths greater than 16 and 64 bytes for EP0 and EP1-4 respectively.  */
    struct {
        int start, len;
        uint8_t fifo[S3C_USB_FIFO_LEN];
        USBPacket *packet;

        uint8_t csr;
        uint8_t maxpacket;
    } ep0;

#define S3C_EPS 5
    struct {
        int start, len;
        uint8_t fifo[S3C_USB_FIFO_LEN];
        USBPacket *packet;
        int packet_len;

        uint8_t in_csr[2];
        uint8_t out_csr[2];
        uint8_t maxpacket;

        uint8_t control;
        uint8_t unit_cnt;
        uint8_t fifo_cnt;
        uint32_t dma_size;
    } ep1[S3C_EPS - 1];

    uint8_t index;
    uint8_t power;
    uint8_t ep_intr;
    uint8_t ep_mask;
    uint8_t usb_intr;
    uint8_t usb_mask;
    uint16_t frame;
    uint8_t address;
};

void s3c_udc_reset(struct s3c_udc_state_s *s)
{
    int i;
    s->address = 0x00;
    s->power = 0x00;
    s->ep_intr = 0x00;
    s->ep_mask = (1 << S3C_EPS) - 1;
    s->usb_intr = 0x00;
    s->usb_mask = 0x06;
    s->frame = 0x0000;
    s->index = 0x00;
    s->ep0.len = 0;
    s->ep0.csr = 0x00;
    s->ep0.maxpacket = 0x01;
    s->ep0.packet = 0;
    for (i = 0; i < S3C_EPS - 1; i ++) {
        s->ep1[i].len = 0;
        s->ep1[i].in_csr[0] = 0;
        s->ep1[i].in_csr[1] = 0;
        s->ep1[i].out_csr[0] = 0;
        s->ep1[i].out_csr[1] = 0;
        s->ep1[i].maxpacket = 0x01;
        s->ep1[i].control = 0x00;
        s->ep1[i].unit_cnt = 0x00;
        s->ep1[i].fifo_cnt = 0x00;
        s->ep1[i].dma_size = 0;
        s->ep1[i].packet = 0;
    }
}

static void s3c_udc_interrupt(struct s3c_udc_state_s *s, int ep)
{
    if (ep >= 0)
        s->ep_intr |= 1 << ep;
    s->ep_intr &= s->ep_mask;
    s->usb_intr &= s->usb_mask;
    pic_set_irq_new(s->pic, S3C_PIC_USBD, s->ep_intr | s->usb_intr);
}

static void s3c_udc_queue_packet(uint8_t *dst, uint8_t *src,
                int fstart, int *flen, int len)
{
    int chunk;
    fstart += *flen;
    *flen += len;
    while (len) {
        fstart &= S3C_USB_FIFO_LEN - 1;
        chunk = MIN(len, S3C_USB_FIFO_LEN - fstart);
        memcpy(dst + fstart, src, chunk);
        len -= chunk;
        src += chunk;
        fstart += chunk;
    }
}

static void s3c_udc_dequeue_packet(uint8_t *dst, uint8_t *src,
                int *fstart, int *flen)
{
    int chunk;
    while (*flen) {
        chunk = MIN(*flen, S3C_USB_FIFO_LEN - *fstart);
        memcpy(dst, src + *fstart, chunk);
        *flen -= chunk;
        dst += chunk;
        *fstart += chunk;
        *fstart &= S3C_USB_FIFO_LEN - 1;
    }
}

static void s3c_udc_ep0_rdy(struct s3c_udc_state_s *s, uint8_t value)
{
    USBPacket *packet = s->ep0.packet;
    if (packet) {
        if (s->ep0.len < packet->len &&
                        !(value & (1 << 3))) {		/* DATA_END */
            s->ep0.csr &= ~(1 << 1);			/* IN_PKT_RDY */
            s3c_udc_interrupt(s, 0);
            return;
        }

        if (value & (1 << 1)) {
            packet->len = s->ep0.len;
            s3c_udc_dequeue_packet(packet->data, s->ep0.fifo,
                            &s->ep0.start, &s->ep0.len);
        } else
            packet->len = 0;

        /* Signal completion of IN token */
        s->ep0.csr &= ~(1 << 3);
        s->ep0.packet = 0;
        packet->complete_cb(packet, packet->complete_opaque);
    }
}

static void s3c_udc_ep1_rdy(struct s3c_udc_state_s *s, int ep, uint8_t value)
{
    USBPacket *packet = s->ep1[ep].packet;
    if (packet) {
        /* HACK: there's no simple (timing independent) way to know when
         * a packet ends.  We will terminate packets only when the OS
         * writes less than 64 bytes (the FIFO size), thus the OS has to
         * make a final zero length write if the packet is 64 multiple
         * sized.  */
        if (!((s->ep1[ep].len - s->ep1[ep].packet_len) & 63) &&
                        (s->ep1[ep].len > s->ep1[ep].packet_len ||
                         s->ep1[ep].len == 0)) {
            s->ep1[ep].packet_len = s->ep1[ep].len;
            s->ep1[ep].in_csr[0] &= ~(1 << 0);		/* IN_PKT_RDY */
            s->ep1[ep].in_csr[0] &= ~(1 << 3);		/* FIFO_FLUSH */
            s3c_udc_interrupt(s, ep + 1);
            return;
        }

        /* TODO: check that packet->len >= s->ep1[ep].len */
        packet->len = s->ep1[ep].len;
        s3c_udc_dequeue_packet(packet->data, s->ep1[ep].fifo,
                        &s->ep1[ep].start, &s->ep1[ep].len);

        /* Signal completion of IN token */
        s->ep1[ep].packet = 0;
        s->ep1[ep].packet_len = 0;
        packet->complete_cb(packet, packet->complete_opaque);
    }
}

#define S3C_FUNC_ADDR	0x140	/* Function address register */
#define S3C_PWR		0x144	/* Power management register */
#define S3C_EP_INT	0x148	/* Endpoint interrupt register */
#define S3C_USB_INT	0x158	/* USB interrupt register */
#define S3C_EP_INT_EN	0x15c	/* Endpoint interrupt enable register */
#define S3C_USB_INT_EN	0x16c	/* USB interrupt enable register */
#define S3C_FRAME_NUM1	0x170	/* Frame number 1 register */
#define S3C_FRAME_NUM2	0x174	/* Frame number 2 register */
#define S3C_INDEX	0x178	/* Index register */
#define S3C_MAXP	0x180	/* MAX packet register */
#define S3C_IN_CSR1	0x184	/* EP In control status register 1 */
#define S3C_IN_CSR2	0x188	/* EP In control status register 2 */
#define S3C_OUT_CSR1	0x190	/* EP Out control status register 1 */
#define S3C_OUT_CSR2	0x194	/* EP Out control status register 2 */
#define S3C_FIFO_CNT1	0x198	/* EP Out write count register 1 */
#define S3C_FIFO_CNT2	0x19c	/* EP Out write count register 2 */
#define S3C_EP0_FIFO	0x1c0	/* Endpoint0 FIFO register */
#define S3C_EP1_FIFO	0x1c4	/* Endpoint1 FIFO register */
#define S3C_EP2_FIFO	0x1c8	/* Endpoint2 FIFO register */
#define S3C_EP3_FIFO	0x1cc	/* Endpoint3 FIFO register */
#define S3C_EP4_FIFO	0x1d0	/* Endpoint4 FIFO register */

static const target_phys_addr_t s3c_udc_ep_reg[S3C_EPS] = {
    0x000, 0x200, 0x218, 0x240, 0x258
};
#define S3C_EP_DMA_CON		0x00	/* DMA control register */
#define S3C_EP_DMA_UNIT		0x04	/* DMA unit counter register */
#define S3C_EP_DMA_FIFO		0x08	/* DMA FIFO counter register */
#define S3C_EP_DMA_TTC_L	0x0c	/* DMA transfer counter low-byte */
#define S3C_EP_DMA_TTC_M	0x10	/* DMA transfer counter middle-byte */
#define S3C_EP_DMA_TTC_H	0x14	/* DMA transfer counter high-byte */

static uint32_t s3c_udc_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_udc_state_s *s = (struct s3c_udc_state_s *) opaque;
    int ep = S3C_EPS;
    uint8_t ret = 0;
    addr -= s->base;
    while (addr < s3c_udc_ep_reg[-- ep]);
    addr -= s3c_udc_ep_reg[ep];

    switch (addr) {
    case S3C_FUNC_ADDR:
        return s->address;
    case S3C_PWR:
        return s->power;
    case S3C_EP_INT:
        return s->ep_intr;
    case S3C_USB_INT:
        return s->usb_intr;
    case S3C_EP_INT_EN:
        return s->ep_mask;
    case S3C_USB_INT_EN:
        return s->usb_mask & ~(1 << 1);
    case S3C_FRAME_NUM1:
        return s->frame & 0xff;
    case S3C_FRAME_NUM2:
        return s->frame >> 8;
    case S3C_INDEX:
        return s->index;

    case S3C_IN_CSR1:
        if (s->index >= S3C_EPS)
            goto bad_reg;
        if (s->index == 0)
            return s->ep0.csr;
        else
            return s->ep1[s->index - 1].in_csr[0];
    case S3C_IN_CSR2:
        if (s->index >= S3C_EPS || s->index == 0)
            goto bad_reg;
        return s->ep1[s->index - 1].in_csr[1];
    case S3C_OUT_CSR1:
        if (s->index >= S3C_EPS || s->index == 0)
            goto bad_reg;
        return s->ep1[s->index - 1].out_csr[0];
    case S3C_OUT_CSR2:
        if (s->index >= S3C_EPS || s->index == 0)
            goto bad_reg;
        return s->ep1[s->index - 1].out_csr[1];
    case S3C_EP0_FIFO:
        if (unlikely(!s->ep0.len))
            printf("%s: Endpoint0 underrun\n", __FUNCTION__);
        else {
            ret = s->ep0.fifo[s->ep0.start ++];
            s->ep0.len --;
        }
        s->ep0.start &= S3C_USB_FIFO_LEN - 1;
        return ret;
    case S3C_EP4_FIFO: ep ++;
    case S3C_EP3_FIFO: ep ++;
    case S3C_EP2_FIFO: ep ++;
    case S3C_EP1_FIFO:
        if (unlikely(s->ep1[ep].in_csr[1] & (1 << 5)))	/* MODE_IN */
            goto bad_reg;
        if (unlikely(!s->ep1[ep].len))
            printf("%s: Endpoint%i underrun\n", __FUNCTION__, ep + 1);
        else {
            ret = s->ep1[ep].fifo[s->ep1[ep].start ++];
            s->ep1[ep].len --;
        }
        s->ep1[ep].start &= S3C_USB_FIFO_LEN - 1;
        if (s->ep1[ep].out_csr[1] & (1 << 7))		/* AUTO_CLR */
            if (s->ep1[ep].len <= 0) {
                s->ep1[ep].out_csr[0] &= ~(1 << 0);	/* OUT_PKT_READY */
            }
        return ret;
    case S3C_MAXP:
        if (s->index >= S3C_EPS)
            goto bad_reg;
        if (s->index == 0)
            return s->ep0.maxpacket;
        else
            return s->ep1[s->index - 1].maxpacket;
    case S3C_FIFO_CNT1:
        if (s->index >= S3C_EPS)
            goto bad_reg;
        if (s->index == 0)
            return s->ep0.len & 0xff;
        else
            return s->ep1[s->index - 1].len & 0xff;
    case S3C_FIFO_CNT2:
        if (s->index >= S3C_EPS)
            goto bad_reg;
        if (s->index == 0)
            return s->ep0.len >> 8;
        else
            return s->ep1[s->index - 1].len >> 8;

    case S3C_EP_DMA_CON:
        if (!ep --)
            goto bad_reg;
        return s->ep1[ep].control;
    case S3C_EP_DMA_UNIT:
        if (!ep --)
            goto bad_reg;
        return s->ep1[ep].unit_cnt;
    case S3C_EP_DMA_FIFO:
        if (!ep --)
            goto bad_reg;
        return s->ep1[ep].fifo_cnt;
    case S3C_EP_DMA_TTC_L:
        if (!ep --)
            goto bad_reg;
        return (s->ep1[ep].dma_size >> 0) & 0xff;
    case S3C_EP_DMA_TTC_M:
        if (!ep --)
            goto bad_reg;
        return (s->ep1[ep].dma_size >> 8) & 0xff;
    case S3C_EP_DMA_TTC_H:
        if (!ep --)
            goto bad_reg;
        return (s->ep1[ep].dma_size >> 16) & 0xf;
    bad_reg:
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_udc_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_udc_state_s *s = (struct s3c_udc_state_s *) opaque;
    int ep = S3C_EPS;
    addr -= s->base;
    while (addr < s3c_udc_ep_reg[-- ep]);
    addr -= s3c_udc_ep_reg[ep];

    switch (addr) {
    case S3C_FUNC_ADDR:
        s->address = value | (1 << 7);			/* ADDR_UPDATE */
        s->dev.addr = value & 0x7f;
        break;

    case S3C_PWR:
        if (!(value & (1 << 2)))			/* MCU_RESUME */
            s->power &= ~(1 << 1);			/* SUSPEND_MODE */
        s->power = (value & 5) | (s->power & 10);
        /* Should send a SUSPEND interrupt every 3ms if SUSPEND is enabled.  */
        break;

    case S3C_EP_INT:
        s->ep_intr &= ~value;
        break;

    case S3C_USB_INT:
        s->usb_intr &= ~value;
        break;

    case S3C_EP_INT_EN:
        s->ep_mask = value & ((1 << S3C_EPS) - 1);
        break;

    case S3C_USB_INT_EN:
        s->usb_mask = (value & 0x5) | (1 << 1);
        break;

    case S3C_INDEX:
        s->index = value;
        break;

    case S3C_IN_CSR1:
        if (s->index >= S3C_EPS)
            goto bad_reg;
        if (s->index == 0) {
            s->ep0.csr = (value | (s->ep0.csr & 0xa)) & (s->ep0.csr | ~0x4);
            if (value & (1 << 7))			/* SERVICED_SETUP_EN */
                s->ep0.csr &= ~(1 << 4);		/* SETUP_END */
            if (value & (1 << 6))			/* SERVICED_OUT_PKT_ */
                s->ep0.csr &= ~(1 << 0);		/* OUT_PKT_RDY */
            if (value & (1 << 1))			/* IN_PKT_RDY */
                s3c_udc_ep0_rdy(s, value);
            else if (value & (1 << 3))			/* DATA_END */
                s3c_udc_ep0_rdy(s, value);
        } else {
            ep = s->index - 1;
            s->ep1[ep].in_csr[0] = (value | (s->ep1[ep].in_csr[0] & 1)) & 0x19;
            if (value & (1 << 3))			/* FIFO_FLUSH */
                s3c_udc_ep1_rdy(s, ep, value);
            else if (value & (1 << 0))			/* IN_PKT_RDY */
                s3c_udc_ep1_rdy(s, ep, value);
        }
        break;

    case S3C_IN_CSR2:
        if (s->index >= S3C_EPS || s->index == 0)
            goto bad_reg;
        s->ep1[s->index - 1].in_csr[1] = value & 0xf0;
        break;

    case S3C_OUT_CSR1:
        if (s->index >= S3C_EPS || s->index == 0)
            goto bad_reg;
        if (s->ep1[s->index - 1].out_csr[0] & (1 << 0)) {	/* OUT_PKT_R */
            if (value & (1 << 4))		/* FIFO_FLUSH */
                s->ep1[s->index - 1].len = 0;
            else if (!(value & (1 << 0)) &&	/* OUT_PKT_RDY */
                            s->ep1[s->index - 1].len) {
                value |= 1 << 0;		/* OUT_PKT_RDY */
                s3c_udc_interrupt(s, s->index);
            }
        }
        s->ep1[s->index - 1].out_csr[0] = value &
                (0xa0 | (s->ep1[s->index - 1].out_csr[0] & 0x41));
        break;

    case S3C_OUT_CSR2:
        if (s->index >= S3C_EPS || s->index == 0)
            goto bad_reg;
        s->ep1[s->index - 1].out_csr[1] = value & 0xe0;
        break;

    case S3C_EP0_FIFO:
        if (unlikely(s->ep0.len >= S3C_USB_FIFO_LEN))
            printf("%s: Endpoint0 overrun\n", __FUNCTION__);
        s->ep0.fifo[(s->ep0.start + s->ep0.len ++) &
                (S3C_USB_FIFO_LEN - 1)] = value;
        break;

    case S3C_EP4_FIFO: ep ++;
    case S3C_EP3_FIFO: ep ++;
    case S3C_EP2_FIFO: ep ++;
    case S3C_EP1_FIFO:
        if (unlikely(!(s->ep1[ep].in_csr[1] & (1 << 5))))	/* MODE_IN */
            goto bad_reg;
        if (unlikely(s->ep1[ep].len >= S3C_USB_FIFO_LEN))
            printf("%s: Endpoint%i overrun\n", __FUNCTION__, ep + 1);
        s->ep1[ep].fifo[(s->ep1[ep].start + s->ep1[ep].len ++) &
                (S3C_USB_FIFO_LEN - 1)] = value;
        if (s->ep1[ep].in_csr[1] & (1 << 7))		/* AUTO_SET */
            if (s->ep1[ep].len >= (s->ep1[ep].maxpacket << 3)) {
                s->ep1[ep].in_csr[0] |= 1 << 0;		/* IN_PKT_RDY */
                s3c_udc_ep1_rdy(s, ep, s->ep1[ep].in_csr[0]);
            }
        break;

    case S3C_MAXP:
        if (s->index >= S3C_EPS)
            goto bad_reg;
        if (s->index == 0)
            s->ep0.maxpacket = value & 0xf;
        else
            s->ep1[s->index - 1].maxpacket = value & 0xf;
        break;

    case S3C_EP_DMA_CON:
        if (!ep --)
            goto bad_reg;
        s->ep1[ep].control = value;
        break;

    case S3C_EP_DMA_UNIT:
        if (!ep --)
            goto bad_reg;
        s->ep1[ep].unit_cnt = value;
        break;

    case S3C_EP_DMA_FIFO:
        if (!ep --)
            goto bad_reg;
        s->ep1[ep].fifo_cnt = value;
        break;

    case S3C_EP_DMA_TTC_L:
        if (!ep --)
            goto bad_reg;
        s->ep1[ep].dma_size &= 0xfff00;
        s->ep1[ep].dma_size |= (value & 0xff) << 0;
        break;

    case S3C_EP_DMA_TTC_M:
        if (!ep --)
            goto bad_reg;
        s->ep1[ep].dma_size &= 0xf00ff;
        s->ep1[ep].dma_size |= (value & 0xff) << 8;
        break;

    case S3C_EP_DMA_TTC_H:
        if (!ep --)
            goto bad_reg;
        s->ep1[ep].dma_size &= 0x0ffff;
        s->ep1[ep].dma_size |= (value & 0xf) << 16;
        break;

    bad_reg:
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_udc_readfn[] = {
    s3c_udc_read,
    s3c_udc_read,
    s3c_udc_read,
};

static CPUWriteMemoryFunc *s3c_udc_writefn[] = {
    s3c_udc_write,
    s3c_udc_write,
    s3c_udc_write,
};

static int s3c_udc_handle_packet(USBDevice *dev, USBPacket *p)
{
    struct s3c_udc_state_s *s = (struct s3c_udc_state_s *) dev->opaque;
    int ep;
    int ret = 0;
    s->power &= ~(1 << 3);				/* USB_RESET */

    switch (p->pid) {
    case USB_MSG_ATTACH:
        dev->state = USB_STATE_ATTACHED;
        break;
    case USB_MSG_DETACH:
        dev->state = USB_STATE_NOTATTACHED;
        break;
    case USB_MSG_RESET:
        s->power |= 1 << 3;				/* USB_RESET */
#if 0
        s->usb_intr |= 1 << 2;				/* RESET */
#endif
        s3c_udc_interrupt(s, -1);
        break;
    case USB_TOKEN_SETUP:
        if (unlikely(s->ep0.packet))
            printf("%s: EP0 overrun\n", __FUNCTION__);
        if (s->ep0.csr & (1 << 5)) {			/* SEND_STALL */
            ret = USB_RET_STALL;
            s->ep0.csr |= 1 << 2;			/* SENT_STALL */
            s3c_udc_interrupt(s, 0);
            break;
        }
        s->frame ++;
        s3c_udc_queue_packet(s->ep0.fifo, p->data,
                        s->ep0.start, &s->ep0.len, p->len);
        s->ep0.csr |= 1 << 0;				/* OUT_PKT_RDY */
        s3c_udc_interrupt(s, 0);
        break;
    case USB_TOKEN_IN:
        ep = p->devep - 1;
        if (unlikely(ep >= 0 &&
                !(s->ep1[ep].in_csr[1] & (1 << 5))))	/* MODE_IN */
            goto fail;
        if (p->devep == 0) {
            if (unlikely(s->ep0.packet))
                printf("%s: EP0 overrun\n", __FUNCTION__);
            if (s->ep0.csr & (1 << 5)) {		/* SEND_STALL */
                ret = USB_RET_STALL;
                s->ep0.csr |= 1 << 2;			/* SENT_STALL */
                s3c_udc_interrupt(s, 0);
                break;
            }
            s->ep0.csr &= ~(1 << 1);			/* IN_PKT_RDY */
            s3c_udc_interrupt(s, p->devep);
            s->ep0.packet = p;
        } else {
            if (unlikely(s->ep1[ep].packet))
                printf("%s: EP%i overrun\n", __FUNCTION__, ep + 1);
            if (s->ep1[ep].in_csr[0] & (1 << 4)) {	/* SEND_STALL */
                ret = USB_RET_STALL;
                s->ep1[ep].in_csr[0] |= 1 << 5;		/* SENT_STALL */
                s3c_udc_interrupt(s, 0);
                break;
            }
            if (!(s->ep1[ep].in_csr[1] & (1 << 4)) ||	/* IN_DMA_INT_MASK */
                    !(s->ep1[ep].control & (1 << 0))) {	/* DMA_MODE_EN */
                s->ep1[ep].in_csr[0] &= ~(1 << 0);	/* IN_PKT_RDY */
                s->ep1[ep].in_csr[0] &= ~(1 << 3);	/* FIFO_FLUSH */
                s3c_udc_interrupt(s, p->devep);
            }
            /* TODO: DMA */
            s->ep1[ep].packet = p;
        }
        s->frame ++;
        ret = USB_RET_ASYNC;
        break;
    case USB_TOKEN_OUT:
        ep = p->devep - 1;
        if (unlikely(ep >= 0 &&
                s->ep1[ep].in_csr[1] & (1 << 5)))	/* MODE_IN */
            goto fail;
        s->frame ++;
        if (p->devep == 0) {
            if (unlikely(s->ep0.packet || (s->ep0.csr &
                                            (1 << 0))))	/* OUT_PKT_RDY */
                printf("%s: EP0 overrun\n", __FUNCTION__);
            if (s->ep0.csr & (1 << 5)) {		/* SEND_STALL */
                ret = USB_RET_STALL;
                s->ep0.csr |= 1 << 2;			/* SENT_STALL */
                s3c_udc_interrupt(s, 0);
                break;
            }
            s3c_udc_queue_packet(s->ep0.fifo, p->data,
                            s->ep0.start, &s->ep0.len, p->len);
            s->ep0.csr |= 1 << 0;			/* OUT_PKT_RDY */
            s3c_udc_interrupt(s, p->devep);
        } else {
            if (unlikely(s->ep1[ep].packet || (s->ep1[ep].out_csr[0] &
                                            (1 << 0))))	/* OUT_PKT_R */
                printf("%s: EP%i overrun\n", __FUNCTION__, ep + 1);
            if (s->ep1[ep].out_csr[0] & (1 << 5)) {	/* SEND_STALL */
                ret = USB_RET_STALL;
                s->ep1[ep].out_csr[0] |= 1 << 6;	/* SENT_STALL */
                s3c_udc_interrupt(s, 0);
                break;
            }
            s3c_udc_queue_packet(s->ep1[ep].fifo, p->data,
                            s->ep1[ep].start,
                            &s->ep1[ep].len, p->len);
            if (!(s->ep1[ep].out_csr[1] & (1 << 5)) ||	/* OUT_DMA_INT_MASK */
                    !(s->ep1[ep].control & (1 << 0))) {	/* DMA_MODE_EN */
                s->ep1[ep].out_csr[0] |= 1 << 0;	/* OUT_PKT_RDY */
                s3c_udc_interrupt(s, p->devep);
            }
            /* TODO: DMA */
        }
        /* Perhaps it is a good idea to return USB_RET_ASYNC here and in
         * USB_TOKEN_SETUP and trigger completion only after OUT_PKT_RDY
         * condition is serviced by the guest to avoid potential overruns
         * and so that the host can know about timeouts (seems there's no
         * way to indicate other errors asynchronously).  */
        break;
    default:
    fail:
        ret = USB_RET_STALL;
        break;
    }
    return ret;
}

static void s3c_udc_handle_destroy(USBDevice *s)
{
}

struct s3c_udc_state_s *s3c_udc_init(target_phys_addr_t base,
                void *pic, void *dma)
{
    int iomemtype;
    struct s3c_udc_state_s *s = (struct s3c_udc_state_s *)
            qemu_mallocz(sizeof(struct s3c_udc_state_s));

    s->base = base;
    s->pic = pic;
    s->dma = dma;

    s->dev.speed = USB_SPEED_FULL;
    s->dev.handle_packet = s3c_udc_handle_packet;
    s->dev.handle_destroy = s3c_udc_handle_destroy;
    s->dev.opaque = s;

    s3c_udc_reset(s);

    iomemtype = cpu_register_io_memory(0, s3c_udc_readfn,
                    s3c_udc_writefn, s);
    cpu_register_physical_memory(s->base, 0xffffff, iomemtype);

    qemu_register_usb_gadget(&s->dev);

    return s;
}
