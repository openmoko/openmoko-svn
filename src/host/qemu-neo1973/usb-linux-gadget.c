/*
 * Linux host USB slave redirector
 *
 * Copyright (C) 2007 OpenMoko, Inc.
 * Written by Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include "config-host.h"
#if defined(CONFIG_GADGETFS)
# include <stdlib.h>
# include <linux/version.h>
# if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 21)
#  include <linux/usb_ch9.h>
#  include <linux/usb_gadgetfs.h>
# elif LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 23)
#  include <linux/usb/ch9.h>
#  include <linux/usb_gadgetfs.h>
# else
#  include <linux/usb/ch9.h>
#  include <linux/usb/gadgetfs.h>
# endif
# include <poll.h>
# include <signal.h>

/* Must be after usb_ch9.h */
# include "qemu-common.h"
# include "qemu-char.h"
# include "sysemu.h"
# include "hw/usb.h"

# define USBGADGETFS_PATH "/dev/gadget"

struct gadget_state_s {
    USBPort port;
    uint8_t config_num;
    int connected;
    int speed;
    int hosthighspeed;
    int highspeed;
    int addr;
    int ep0fd;
    const char *ep0path;
    struct ep_s {
        int fd;
        char *path;
        int num;
        struct gadget_state_s *state;

        int busy;
        uint8_t buffer[4096];
        USBPacket packet;
    } ep[16];

    /* Device descriptor */
    uint8_t dev_desc[128];
    int desc_len;

    /* Configuration descriptor */
    struct usb_config_descriptor config_desc;
    uint8_t config_tail_buffer[256];

    uint8_t buffer[4096];
    int async_count;
    USBPacket packet[16];
};

static void gadget_stall(struct gadget_state_s *hci, USBPacket *packet)
{
    int ret, fd;
    if (packet->devep)
        fd = ((struct ep_s *) packet->complete_opaque)->fd;
    else
        fd = hci->ep0fd;

    if (packet->pid == USB_TOKEN_IN || (packet->pid == USB_TOKEN_SETUP &&
            (((struct usb_ctrlrequest *) packet->data)->bRequestType &
             USB_DIR_IN)))
        ret = read(fd, &ret, 0);
    else
        ret = write(fd, &ret, 0);
    if (ret != -1)
        fprintf(stderr, "%s: can't stall ep%i\n",
                        __FUNCTION__, packet->devep);
    else if (errno != EL2HLT && errno != EBADMSG)
        fprintf(stderr, "%s: can't stall ep%i: %i\n",
                        __FUNCTION__, packet->devep, errno);
}

static void gadget_detach(struct gadget_state_s *hci)
{
    char *devname;
    if (hci->port.dev) {
        /* XXX We should rather only detach the device
         * (usb_attach(&hci->port, NULL)) instead of destroying it,
         * but then the port remains in used_usb_ports -> segfault */
        asprintf(&devname, "%i.%i", 0, hci->port.dev->addr);
        do_usb_del(devname);
        free(devname);
    }
}

static void gadget_run(USBPacket *prev_packet, void *opaque)
{
    struct gadget_state_s *hci = (struct gadget_state_s *) opaque;
    USBDevice *dev = hci->port.dev;
    USBPacket *packet;
    int ret;

next:
    if (hci->async_count) {
        packet = &hci->packet[-- hci->async_count];
        ret = dev->handle_packet(dev, packet);

        if (ret >= 0) {
            packet->len = ret;
            packet->complete_cb(packet, packet->complete_opaque);
        } else if (ret == USB_RET_STALL) {
            hci->async_count = 0;
            gadget_stall(hci, packet);
        } else if (ret != USB_RET_ASYNC) {
            fprintf(stderr, "%s: packet unhandled: %i\n", __FUNCTION__, ret);
            if (ret == USB_RET_NAK)
                goto next;
            hci->async_count = 0;
            gadget_detach(hci);
        }
    }
}

static void gadget_ep_run(struct ep_s *ep)
{
    USBDevice *dev = ep->state->port.dev;
    int ret;

    ret = dev->handle_packet(dev, &ep->packet);

    if (ret >= 0) {
        ep->packet.len = ret;
        ep->packet.complete_cb(&ep->packet, ep->packet.complete_opaque);
    } else if (ret == USB_RET_STALL) {
        gadget_stall(ep->state, &ep->packet);
    } else if (ret == USB_RET_ASYNC) {
        ep->busy = 1;
    } else {
        fprintf(stderr, "%s: EP%i packet unhandled: %i\n", __FUNCTION__,
                        ep->num, ret);
        if (ret != USB_RET_NAK)
            gadget_detach(ep->state);
    }
}

static void gadget_respond(USBPacket *packet, void *opaque)
{
    struct gadget_state_s *hci = (struct gadget_state_s *) opaque;
    int ret;

    do {
        ret = write(hci->ep0fd, hci->buffer, packet->len);
    } while (ret < packet->len && errno == EAGAIN);
    if (ret < packet->len) {
        fprintf(stderr, "%s: packet write error: %i\n", __FUNCTION__, errno);
        return;
    }

    gadget_run(packet, hci);
}

static void gadget_token_in(USBPacket *packet, void *opaque)
{
    struct ep_s *ep = (struct ep_s *) opaque;
    sigset_t new, old;
    ep->busy = 0;
    sigfillset(&new);
    /* The packets reach 1.5 kB and the loop alone, with unblocked
     * signals locks qemu up for ~10s until a 1.5 kB write succeeds
     * uninterrupted.  */
    pthread_sigmask(SIG_BLOCK, &new, &old);
    while (write(ep->fd, packet->data, packet->len) < packet->len &&
                    errno == EINTR);
    pthread_sigmask(SIG_SETMASK, &old, 0);
}

#if 0
static int gadget_ep_poll(void *opaque)
{
    struct ep_s *ep = (struct ep_s *) opaque;
    struct pollfd pfd = { ep->fd, POLLOUT, 0 };
    int ret = poll(&pfd, 1, 0);
    return (ret > 0) && (pfd.revents & POLLOUT);
}
#endif

static void gadget_ep_read(void *opaque)
{
    struct ep_s *ep = (struct ep_s *) opaque;
    struct gadget_state_s *hci = ep->state;

    if (ep->busy)
        return;

#if 0
    if (!gadget_ep_poll(opaque))
        return;
#endif

    /* write() is supposed to not block here (but it does :-/ ) */
    if (write(ep->fd, ep->buffer, 0))
        return;

    ep->packet.pid = USB_TOKEN_IN;
    ep->packet.devaddr = hci->addr;
    ep->packet.devep = ep->num;
    ep->packet.data = ep->buffer;
    ep->packet.len = sizeof(ep->buffer);
    ep->packet.complete_cb = gadget_token_in;
    ep->packet.complete_opaque = ep;

    gadget_ep_run(ep);
}

static void gadget_nop(USBPacket *prev_packet, void *opaque)
{
    ((struct ep_s *) opaque)->busy = 0;
}

static void gadget_ep_write(void *opaque)
{
    struct ep_s *ep = (struct ep_s *) opaque;
    struct gadget_state_s *hci = ep->state;
    int ret;

    if (ep->busy)
        return;

    ret = read(ep->fd, ep->buffer, sizeof(ep->buffer));
    if (ret <= 0)
        return;

    ep->packet.pid = USB_TOKEN_OUT;
    ep->packet.devaddr = hci->addr;
    ep->packet.devep = ep->num;
    ep->packet.data = ep->buffer;
    ep->packet.len = ret;
    ep->packet.complete_cb = gadget_nop;
    ep->packet.complete_opaque = ep;

    gadget_ep_run(ep);
}

static int gadget_ep_open(struct gadget_state_s *hci,
                struct usb_endpoint_descriptor *desc)
{
    struct ep_s *ep;
    int ret, i, dl;
    uint32_t format;
    uint8_t buffer[128], *dst;
    for (ep = hci->ep, i = 0; ep->fd != -1; ep ++, i ++);

    ep->state = hci;
    ep->num = desc->bEndpointAddress & 0xf;
    /* Only dummy_hcd and net2280 have "ep-?" configurable endpoints.
     * Maybe we should scan all available endpoints and choose ones that
     * match, but this would be painful.  */
    asprintf(&ep->path, "%s/ep-%c", USBGADGETFS_PATH, 'a' + i);
    ep->fd = open(ep->path, O_RDWR);
    if (ep->fd < 0) {
        ret = errno;
        goto fail;
    }

    dl = 0;
    dst = buffer;

    /* Write the format tag */
    format = 1;
    memcpy(dst, &format, 4);
    dst += 4;
    dl += 4;

    /* Write the endpoint descriptor */
    memcpy(dst, desc, desc->bLength);
    dst += desc->bLength;
    dl += desc->bLength;

    if (hci->hosthighspeed) {
        /* Send the same information again as high-speed descriptors */
        memcpy(dst, desc, desc->bLength);
        dst += desc->bLength;
        dl += desc->bLength;
    }

    ret = write(ep->fd, buffer, dl);
    if (ret < dl) {
        if (ret < 0)
            ret = errno;
        else
            ret = 1;
        close(ep->fd);
        goto fail;
    }

    ep->busy = 0;

    if (desc->bEndpointAddress & USB_DIR_IN)
        qemu_set_fd_handler(ep->fd, NULL, gadget_ep_read, ep);
    else
        qemu_set_fd_handler(ep->fd, gadget_ep_write, NULL, ep);

    return 0;

fail:
    free(ep->path);
    ep->fd = -1;
    return -ret;
}

static void gadget_ep_close(struct gadget_state_s *hci, int addr)
{
    struct ep_s *ep;
    int i;
    for (ep = hci->ep, i = 0; ep->num != addr || ep->fd == -1; ep ++, i ++);

    qemu_set_fd_handler2(ep->fd, NULL, NULL, NULL, NULL);
    close(ep->fd);
    ep->fd = -1;
    free(ep->path);
}

static void gadget_ep_done(struct gadget_state_s *hci)
{
    int i;
    for (i = 0; i < 16; i ++)
        if (hci->ep[i].fd != -1)
            gadget_ep_close(hci, hci->ep[i].num);
}

static void gadget_ep_setup(struct gadget_state_s *hci)
{
    int ret, sl = 0;
    struct usb_descriptor_header *src =
            (struct usb_descriptor_header *) &hci->config_desc;

    /* Clean-up */
    gadget_ep_done(hci);

    while (sl < hci->config_desc.wTotalLength) {
        if (src->bDescriptorType == USB_DT_ENDPOINT) {
            ret = gadget_ep_open(hci, (struct usb_endpoint_descriptor *) src);
            if (ret < 0) {
                if (ret != -ESHUTDOWN)
                    goto fail;
                fprintf(stderr, "%s: EPs not configured due to disconnect - "
                                "will retry\n", __FUNCTION__);
                return;
            }
        }
        sl += src->bLength;
        src = (typeof(src)) ((uint8_t *) src + src->bLength);
    }
    return;

fail:
    gadget_detach(hci);
    fprintf(stderr, "%s: endpoint configuration failed: %i\n",
                    __FUNCTION__, ret);
}

static void gadget_desc_parse(USBPacket *packet, void *opaque)
{
    struct gadget_state_s *hci = (struct gadget_state_s *) opaque;
    uint8_t *p;
    hci->desc_len = packet->len;

    /* HACK: report always only a single Configuration */
    ((struct usb_device_descriptor *) hci->dev_desc)->bNumConfigurations = 1;

    if (hci->hosthighspeed) {
        /* We need to fake a USB 2.0 device if we're already faking
         * a high-speed device (due to gadgetfs limitations).  */
        p = (uint8_t *) &((struct usb_device_descriptor *)
                        hci->dev_desc)->bcdUSB;
        p[0] = 0x20;
        p[1] = 0x00;
    }

    gadget_run(packet, hci);
}

static void gadget_ep_parse(USBPacket *packet, void *opaque)
{
    struct gadget_state_s *hci = (struct gadget_state_s *) opaque;
    uint8_t buffer[4096];
    int dl = 0;
    int sl = 0;
    struct usb_config_descriptor *cfg;
    struct usb_descriptor_header *src =
            (struct usb_descriptor_header *) packet->data;
    uint8_t *dst = buffer;
    uint32_t format;
    int ret = 1;

    /* Write the format tag */
    format = 0;
    memcpy(dst, &format, 4);
    dst += 4;
    dl += 4;

    /* Write the config & endpoint descriptors */
    if (src->bDescriptorType != USB_DT_CONFIG)
        goto fail;
    cfg = (struct usb_config_descriptor *) src;
    if (packet->len < cfg->wTotalLength)
        goto fail;
    if (dl + cfg->wTotalLength > sizeof(buffer))
        goto fail;
    cfg->bMaxPower = 0x00;
    cfg->bmAttributes = 0xc0; /* dummy_hcd is picky about power */

    if (hci->hosthighspeed) {
        /* For high-speed devices we need to set the interval for
         * Interrupt Enpoints as high as possible so that the host doesn't
         * expect really fast responses from the emulator and thus
         * time-out on some packets.  */
        while (sl < cfg->wTotalLength) {
            if (src->bDescriptorType == USB_DT_ENDPOINT) {
                if (((struct usb_endpoint_descriptor *) src)->bmAttributes ==
                                USB_ENDPOINT_XFER_INT)
                    ((struct usb_endpoint_descriptor *) src)->bInterval = 0x0e;
            }
            sl += src->bLength;
            src = (typeof(src)) ((uint8_t *) src + src->bLength);
        }
    }

    memcpy(dst, cfg, cfg->wTotalLength);
    memcpy(&hci->config_desc, cfg, cfg->wTotalLength);
    dl += cfg->wTotalLength;
    dst += cfg->wTotalLength;

    if (hci->hosthighspeed) {
        /* Send the same information again as high-speed descriptors */
        if (dl + cfg->wTotalLength > sizeof(buffer))
            goto fail;
        memcpy(dst, cfg, cfg->wTotalLength);
        dl += cfg->wTotalLength;
        dst += cfg->wTotalLength;
    }

    /* Write the device descriptor */
    if (dl + hci->desc_len > sizeof(buffer))
        goto fail;
    memcpy(dst, hci->dev_desc, hci->desc_len);
    dl += hci->desc_len;
    dst += hci->desc_len;

    ret = write(hci->ep0fd, buffer, dl);
    if (ret < dl) {
        ret = -errno;
        goto fail;
    }

    return;

fail:
    usb_attach(&hci->port, NULL); /* XXX or call gadget_detach(hci); */
    fprintf(stderr, "%s: failed to configure gadgetfs: %i\n",
                    __FUNCTION__, ret);
}

/* GadgetFS apparentlye xpects the device to be in Address State and
 * not necesarily configured, at the point when device descriptor is
 * written to ep0 fd.  Go into that state, enumerate endpoints and
 * report endpoint and device descriptors.  */
static void gadget_ep_configure(struct gadget_state_s *hci)
{
    struct usb_ctrlrequest *req;

    hci->addr = 5; /* XXX How should the value be decided */

    hci->async_count = 6;

    /* Ask for the device descriptor */
    hci->packet[5].pid = USB_TOKEN_SETUP;
    hci->packet[5].devaddr = 0;
    hci->packet[5].devep = 0;
    hci->packet[5].data = hci->buffer;
    hci->packet[5].len = 8;
    hci->packet[5].complete_cb = gadget_run;
    hci->packet[5].complete_opaque = hci;

    req = (struct usb_ctrlrequest *) hci->buffer;
    req->bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req->bRequest = USB_REQ_GET_DESCRIPTOR;
    req->wValue = (USB_DT_DEVICE << 8) | 0;
    req->wIndex = 0x0000;
    req->wLength = sizeof(hci->dev_desc);

    /* Read the response */
    hci->packet[4].pid = USB_TOKEN_IN;
    hci->packet[4].devaddr = 0;
    hci->packet[4].devep = 0;
    hci->packet[4].data = hci->dev_desc;
    hci->packet[4].len = sizeof(hci->dev_desc);
    hci->packet[4].complete_cb = gadget_desc_parse;
    hci->packet[4].complete_opaque = hci;

    /* Set address */
    hci->packet[3].pid = USB_TOKEN_SETUP;
    hci->packet[3].devaddr = 0;
    hci->packet[3].devep = 0;
    hci->packet[3].data = hci->buffer + 8;
    hci->packet[3].len = 8;
    hci->packet[3].complete_cb = gadget_run;
    hci->packet[3].complete_opaque = hci;

    req = (struct usb_ctrlrequest *) (hci->buffer + 8);
    req->bRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req->bRequest = USB_REQ_SET_ADDRESS;
    req->wValue = hci->addr;
    req->wIndex = 0x0000;
    req->wLength = 0x0000;

    /* Dummy read */
    hci->packet[2].pid = USB_TOKEN_IN;
    hci->packet[2].devaddr = 0;
    hci->packet[2].devep = 0;
    hci->packet[2].data = hci->buffer;
    hci->packet[2].len = 0;
    hci->packet[2].complete_cb = gadget_run;
    hci->packet[2].complete_opaque = hci;

    /* Ask for configuration #0 descriptor (which contains endpoints info) */
    hci->packet[1].pid = USB_TOKEN_SETUP;
    hci->packet[1].devaddr = hci->addr;
    hci->packet[1].devep = 0;
    hci->packet[1].data = hci->buffer + 16;
    hci->packet[1].len = 8;
    hci->packet[1].complete_cb = gadget_run;
    hci->packet[1].complete_opaque = hci;

    req = (struct usb_ctrlrequest *) (hci->buffer + 16);
    req->bRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    req->bRequest = USB_REQ_GET_DESCRIPTOR;
    req->wValue = (USB_DT_CONFIG << 8) | hci->config_num;
    req->wIndex = 0x0000;
    req->wLength = sizeof(hci->buffer);

    /* Read and parse the response */
    hci->packet[0].pid = USB_TOKEN_IN;
    hci->packet[0].devaddr = hci->addr;
    hci->packet[0].devep = 0;
    hci->packet[0].data = hci->buffer;
    hci->packet[0].len = sizeof(hci->buffer);
    hci->packet[0].complete_cb = gadget_ep_parse;
    hci->packet[0].complete_opaque = hci;

    usb_send_msg(hci->port.dev, USB_MSG_RESET);

    gadget_run(0, hci);
}

static void gadget_read(void *opaque)
{
    struct gadget_state_s *s = (struct gadget_state_s *) opaque;
    struct usb_gadgetfs_event event;
    int ret, len;

    if (!s->addr)
        return;

    ret = read(s->ep0fd, &event, sizeof(event));
    if (ret < 0 && errno != EAGAIN)
        fprintf(stderr, "%s: event error: %i\n", __FUNCTION__, errno);
    if (ret < (int) sizeof(event))
        return;

    switch (event.type) {
    case GADGETFS_NOP:
    case GADGETFS_SUSPEND:
        break;

    case GADGETFS_CONNECT:
        s->connected = 1;
        s->speed = event.u.speed;
        gadget_ep_setup(s);
        break;

    case GADGETFS_SETUP:
        s->connected = 1;
        if (s->async_count) {
            fprintf(stderr, "%s: overrun\n", __FUNCTION__);
            gadget_detach(s);
            return;
        }

        s->async_count = 2;

        memcpy(s->buffer, &event.u.setup, sizeof(event.u.setup));
        s->packet[1].pid = USB_TOKEN_SETUP;
        s->packet[1].devaddr = s->addr;
        s->packet[1].devep = 0;
        s->packet[1].data = s->buffer;
        s->packet[1].len = sizeof(event.u.setup);
        s->packet[1].complete_cb = gadget_run;
        s->packet[1].complete_opaque = s;

        /* Handle the response */
        if (event.u.setup.bRequestType & USB_DIR_IN) {
            s->packet[0].pid = USB_TOKEN_IN;
            s->packet[0].devaddr = s->addr;
            s->packet[0].devep = 0;
            s->packet[0].data = s->buffer;
            s->packet[0].len = sizeof(s->buffer);
            s->packet[0].complete_cb = gadget_respond;
            s->packet[0].complete_opaque = s;
        } else {
            len = event.u.setup.wLength;
            if (len > sizeof(s->buffer))
                len = sizeof(s->buffer);
            ret = read(s->ep0fd, s->buffer, len);
            if (ret < 0) {
                fprintf(stderr, "%s: read error\n", __FUNCTION__);
                ret = 0;
            }
            s->packet[0].pid = USB_TOKEN_OUT;
            s->packet[0].devaddr = s->addr;
            s->packet[0].devep = 0;
            s->packet[0].data = s->buffer;
            s->packet[0].len = ret;
            s->packet[0].complete_cb = gadget_run;
            s->packet[0].complete_opaque = s;
        }

        gadget_run(0, s);
        break;

    case GADGETFS_DISCONNECT:
        s->connected = 0;
        s->speed = USB_SPEED_UNKNOWN;
        gadget_ep_done(s);
        break;

    default:
        fprintf(stderr, "%s: unhandled event: %i\n", __FUNCTION__, event.type);
    }
}

static int gadget_open(struct gadget_state_s *hci)
{
    hci->ep0fd = open(hci->ep0path, O_RDWR);
    if (hci->ep0fd < 0)
        return -errno;

    qemu_set_fd_handler(hci->ep0fd, gadget_read, NULL, hci);
    return 0;
}

static void gadget_close(struct gadget_state_s *hci)
{
    gadget_ep_done(hci);

    qemu_set_fd_handler(hci->ep0fd, NULL, NULL, NULL);

    close(hci->ep0fd);
}

/* Attach or detach a device on the gadget hcd.  */
static void gadget_attach(USBPort *port, USBDevice *dev)
{
    struct gadget_state_s *s = (struct gadget_state_s *) port->opaque;
    int ret;

    if (dev) {
        if (port->dev) {
            usb_attach(port, NULL);
            /* XXX or call gadget_detach for consistency */
        }

        ret = gadget_open(s);
        if (ret < 0) {
            fprintf(stderr, "%s: warning: failed to open gadgetfs: %i\n",
                            __FUNCTION__, ret);
            return;
        }

        port->dev = dev;
        s->highspeed = (s->hosthighspeed && dev->speed == USB_SPEED_HIGH);

        /* send the attach message */
        usb_send_msg(dev, USB_MSG_ATTACH);

        gadget_ep_configure(s);
    } else {
        dev = port->dev;
        if (dev) {
            /* send the detach message */
            usb_send_msg(dev, USB_MSG_DETACH);

            gadget_close(s);
        }

        s->addr = 0;
        port->dev = NULL;
    }
}

static int gadget_autoconfig(struct gadget_state_s *s)
{
    struct stat statb;
    s->ep0fd = -1;

    s->hosthighspeed = 1;

    /* dummy_hcd, high/full speed */
    s->ep0path = USBGADGETFS_PATH "/dummy_udc";
    if (stat(s->ep0path, &statb) == 0)
        goto found;
    /* NetChip 2280 PCI device, high/full speed */
    s->ep0path = USBGADGETFS_PATH "/net2280";
    if (stat(s->ep0path, &statb) == 0)
        goto found;

    s->hosthighspeed = 0;

    /* Intel PXA 2xx processor, full speed only */
    s->ep0path = USBGADGETFS_PATH "/pxa2xx_udc";
    if (stat(s->ep0path, &statb) == 0)
        goto found;
    /* AMD au1x00 processor, full speed only */
    s->ep0path = USBGADGETFS_PATH "/au1x00_udc";
    if (stat(s->ep0path, &statb) == 0)
        goto found;
    /* Intel SA-1100 processor, full speed only */
    s->ep0path = USBGADGETFS_PATH "/sa1100";
    if (stat(s->ep0path, &statb) == 0)
        goto found;
    /* Toshiba TC86c001 PCI device, full speed only */
    s->ep0path = USBGADGETFS_PATH "/goku_udc";
    if (stat(s->ep0path, &statb) == 0)
        goto found;
    /* Renesas SH77xx processors, full speed only */
    s->ep0path = USBGADGETFS_PATH "/sh_udc";
    if (stat(s->ep0path, &statb) == 0)
        goto found;
    /* OMAP 1610 and newer devices, full speed only, fifo mode 0 or 3 */
    s->ep0path = USBGADGETFS_PATH "/omap_udc";
    if (stat(s->ep0path, &statb) == 0)
        goto found;
    /* Something based on Mentor USB Highspeed Dual-Role Controller */
    s->ep0path = USBGADGETFS_PATH "/musb_hdrc";
    if (stat(s->ep0path, &statb) == 0)
        goto found;
    /* Atmel AT91 processors, full speed only */
    s->ep0path = USBGADGETFS_PATH "/at91_udc";
    if (stat(s->ep0path, &statb) == 0)
        goto found;
    /* Sharp LH740x processors, full speed only */
    s->ep0path = USBGADGETFS_PATH "/lh740x_udc";
    if (stat(s->ep0path, &statb) == 0)
        goto found;

    return -ENOENT;

found:
    /* XXX gadgetfs seems to always believe it's high-speed */
    s->hosthighspeed = 1;
    return 0;
}

static void gadget_done(void)
{
#if 0
    if (s->ep0fd >= 0)
        close(s->ep0fd);
#endif
}

int usb_gadget_init(void)
{
    struct gadget_state_s *hci = (struct gadget_state_s *)
            qemu_mallocz(sizeof(struct gadget_state_s));
    int i, ret;
    for (i = 0; i < 16; i ++)
        hci->ep[i].fd = -1;

    ret = gadget_autoconfig(hci);
    if (ret < 0)
        return ret;
    atexit(gadget_done);

    hci->addr = 0;
    hci->config_num = 0;

    qemu_register_usb_port(&hci->port, hci, USB_INDEX_HOST, gadget_attach);

    return ret;
}

void usb_gadget_config_set(USBPort *port, int config)
{
    struct gadget_state_s *hci = (struct gadget_state_s *) port->opaque;
    hci->config_num = config;
}

#else
# include "qemu-common.h"
# include "hw/usb.h"

int usb_gadget_init(void)
{
    return -ENODEV;
}

void usb_gadget_config_set(USBPort *port, int config)
{
}

#endif
