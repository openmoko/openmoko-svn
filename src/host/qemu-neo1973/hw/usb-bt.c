/*
 * QEMU Bluetooth HCI USB Transport Layer
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
#include "vl.h"

struct USBBtState {
    int altsetting;
    USBDevice dev;
    struct bt_hci_s hci;

    int config;

#define EVT_FIFO_LEN_MASK	15
    struct {
        int start, len;
        uint8_t pkt[262];
    } evt_fifo[EVT_FIFO_LEN_MASK + 1];
    int evt_start, evt_len;
};

#define USB_EVT_EP	1
#define USB_ACL_EP	2
#define USB_SCO_EP	3

static const uint8_t qemu_bt_dev_descriptor[] = {
    0x12,	/*  u8 bLength; */
    0x01,	/*  u8 bDescriptorType; Device */
    0x00, 0x02,	/*  u16 bcdUSB; v1.0 */

    0xe0,	/*  u8  bDeviceClass; Wireless */
    0x01,	/*  u8  bDeviceSubClass; Radio Frequency */
    0x01,	/*  u8  bDeviceProtocol; Bluetooth */
    0x40,	/*  u8  bMaxPacketSize0; 64 Bytes */

    0x12, 0x0a,	/*  u16 idVendor; */
    0x01, 0x00,	/*  u16 idProduct; Bluetooth Dongle (HCI mode) */
    0x58, 0x19,	/*  u16 bcdDevice */

    0x00,	/*  u8  iManufacturer; */
    0x00,	/*  u8  iProduct; */
    0x00,	/*  u8  iSerialNumber; */
    0x01,	/*  u8  bNumConfigurations; */
};

static const uint8_t qemu_bt_config_descriptor[] = {
    /* one configuration */
    0x09,	/*  u8  bLength; */
    0x02,	/*  u8  bDescriptorType; Configuration */
    0xb1, 0x00,	/*  u16 wTotalLength; */
    0x02,	/*  u8  bNumInterfaces; (2) */
    0x01,	/*  u8  bConfigurationValue; */
    0x00,	/*  u8  iConfiguration; */
    0x80,	/*  u8  bmAttributes; 
			     Bit 7: must be set,
				 6: Self-powered,
				 5: Remote wakeup,
				 4..0: resvd */
    0x00,	/*  u8  MaxPower; */
  
    /* USB 1.1:
     * USB 2.0, single TT organization (mandatory):
     *	one interface, protocol 0
     *
     * USB 2.0, multiple TT organization (optional):
     *	two interfaces, protocols 1 (like single TT)
     *	and 2 (multiple TT mode) ... config is
     *	sometimes settable
     *	NOT IMPLEMENTED
     */

    /* interface one */
    0x09,	/*  u8  if_bLength; */
    0x04,	/*  u8  if_bDescriptorType; Interface */
    0x00,	/*  u8  if_bInterfaceNumber; */
    0x00,	/*  u8  if_bAlternateSetting; */
    0x03,	/*  u8  if_bNumEndpoints; */
    0xe0,	/*  u8  if_bInterfaceClass; Wireless */
    0x01,	/*  u8  if_bInterfaceSubClass; Radio Frequency */
    0x01,	/*  u8  if_bInterfaceProtocol; Bluetooth */
    0x00,	/*  u8  if_iInterface; */
 
    /* endpoint one */
    0x07,	/*  u8  ep_bLength; */
    0x05,	/*  u8  ep_bDescriptorType; Endpoint */
    USB_DIR_IN | USB_EVT_EP,	/*  u8  ep_bEndpointAddress; */
    0x03,	/*  u8  ep_bmAttributes; Interrupt */
    0x10, 0x00,	/*  u16 ep_wMaxPacketSize; */
    0x01,	/*  u8  ep_bInterval; (255ms -- usb 2.0 spec) */
 
    /* endpoint two */
    0x07,	/*  u8  ep_bLength; */
    0x05,	/*  u8  ep_bDescriptorType; Endpoint */
    USB_DIR_OUT | USB_ACL_EP,	/*  u8  ep_bEndpointAddress; */
    0x02,	/*  u8  ep_bmAttributes; Bulk */
    0x40, 0x00,	/*  u16 ep_wMaxPacketSize; */
    0x0a,	/*  u8  ep_bInterval; (255ms -- usb 2.0 spec) */
 
    /* endpoint three */
    0x07,	/*  u8  ep_bLength; */
    0x05,	/*  u8  ep_bDescriptorType; Endpoint */
    USB_DIR_IN | USB_ACL_EP,	/*  u8  ep_bEndpointAddress; */
    0x02,	/*  u8  ep_bmAttributes; Bulk */
    0x40, 0x00,	/*  u16 ep_wMaxPacketSize; */
    0x0a,	/*  u8  ep_bInterval; (255ms -- usb 2.0 spec) */

    /* interface two setting one */
    0x09,	/*  u8  if_bLength; */
    0x04,	/*  u8  if_bDescriptorType; Interface */
    0x01,	/*  u8  if_bInterfaceNumber; */
    0x00,	/*  u8  if_bAlternateSetting; */
    0x02,	/*  u8  if_bNumEndpoints; */
    0xe0,	/*  u8  if_bInterfaceClass; Wireless */
    0x01,	/*  u8  if_bInterfaceSubClass; Radio Frequency */
    0x01,	/*  u8  if_bInterfaceProtocol; Bluetooth */
    0x00,	/*  u8  if_iInterface; */
 
    /* endpoint one */
    0x07,	/*  u8  ep_bLength; */
    0x05,	/*  u8  ep_bDescriptorType; Endpoint */
    USB_DIR_OUT | USB_SCO_EP,	/*  u8  ep_bEndpointAddress; */
    0x01,	/*  u8  ep_bmAttributes; Isochronous */
    0x00, 0x00,	/*  u16 ep_wMaxPacketSize; */
    0x01,	/*  u8  ep_bInterval; (255ms -- usb 2.0 spec) */
 
    /* endpoint two */
    0x07,	/*  u8  ep_bLength; */
    0x05,	/*  u8  ep_bDescriptorType; Endpoint */
    USB_DIR_IN | USB_SCO_EP,	/*  u8  ep_bEndpointAddress; */
    0x01,	/*  u8  ep_bmAttributes; Isochronous */
    0x00, 0x00,	/*  u16 ep_wMaxPacketSize; */
    0x01,	/*  u8  ep_bInterval; (255ms -- usb 2.0 spec) */

    /* interface two setting two */
    0x09,	/*  u8  if_bLength; */
    0x04,	/*  u8  if_bDescriptorType; Interface */
    0x01,	/*  u8  if_bInterfaceNumber; */
    0x01,	/*  u8  if_bAlternateSetting; */
    0x02,	/*  u8  if_bNumEndpoints; */
    0xe0,	/*  u8  if_bInterfaceClass; Wireless */
    0x01,	/*  u8  if_bInterfaceSubClass; Radio Frequency */
    0x01,	/*  u8  if_bInterfaceProtocol; Bluetooth */
    0x00,	/*  u8  if_iInterface; */
 
    /* endpoint one */
    0x07,	/*  u8  ep_bLength; */
    0x05,	/*  u8  ep_bDescriptorType; Endpoint */
    USB_DIR_OUT | USB_SCO_EP,	/*  u8  ep_bEndpointAddress; */
    0x01,	/*  u8  ep_bmAttributes; Isochronous */
    0x09, 0x00,	/*  u16 ep_wMaxPacketSize; */
    0x01,	/*  u8  ep_bInterval; (255ms -- usb 2.0 spec) */
 
    /* endpoint two */
    0x07,	/*  u8  ep_bLength; */
    0x05,	/*  u8  ep_bDescriptorType; Endpoint */
    USB_DIR_IN | USB_SCO_EP,	/*  u8  ep_bEndpointAddress; */
    0x01,	/*  u8  ep_bmAttributes; Isochronous */
    0x09, 0x00,	/*  u16 ep_wMaxPacketSize; */
    0x01,	/*  u8  ep_bInterval; (255ms -- usb 2.0 spec) */

    /* interface two setting three */
    0x09,	/*  u8  if_bLength; */
    0x04,	/*  u8  if_bDescriptorType; Interface */
    0x01,	/*  u8  if_bInterfaceNumber; */
    0x02,	/*  u8  if_bAlternateSetting; */
    0x02,	/*  u8  if_bNumEndpoints; */
    0xe0,	/*  u8  if_bInterfaceClass; Wireless */
    0x01,	/*  u8  if_bInterfaceSubClass; Radio Frequency */
    0x01,	/*  u8  if_bInterfaceProtocol; Bluetooth */
    0x00,	/*  u8  if_iInterface; */
 
    /* endpoint one */
    0x07,	/*  u8  ep_bLength; */
    0x05,	/*  u8  ep_bDescriptorType; Endpoint */
    USB_DIR_OUT | USB_SCO_EP,	/*  u8  ep_bEndpointAddress; */
    0x01,	/*  u8  ep_bmAttributes; Isochronous */
    0x11, 0x00,	/*  u16 ep_wMaxPacketSize; */
    0x01,	/*  u8  ep_bInterval; (255ms -- usb 2.0 spec) */
 
    /* endpoint two */
    0x07,	/*  u8  ep_bLength; */
    0x05,	/*  u8  ep_bDescriptorType; Endpoint */
    USB_DIR_IN | USB_SCO_EP,	/*  u8  ep_bEndpointAddress; */
    0x01,	/*  u8  ep_bmAttributes; Isochronous */
    0x11, 0x00,	/*  u16 ep_wMaxPacketSize; */
    0x01,	/*  u8  ep_bInterval; (255ms -- usb 2.0 spec) */

    /* interface two setting four */
    0x09,	/*  u8  if_bLength; */
    0x04,	/*  u8  if_bDescriptorType; Interface */
    0x01,	/*  u8  if_bInterfaceNumber; */
    0x03,	/*  u8  if_bAlternateSetting; */
    0x02,	/*  u8  if_bNumEndpoints; */
    0xe0,	/*  u8  if_bInterfaceClass; Wireless */
    0x01,	/*  u8  if_bInterfaceSubClass; Radio Frequency */
    0x01,	/*  u8  if_bInterfaceProtocol; Bluetooth */
    0x00,	/*  u8  if_iInterface; */
 
    /* endpoint one */
    0x07,	/*  u8  ep_bLength; */
    0x05,	/*  u8  ep_bDescriptorType; Endpoint */
    USB_DIR_OUT | USB_SCO_EP,	/*  u8  ep_bEndpointAddress; */
    0x01,	/*  u8  ep_bmAttributes; Isochronous */
    0x19, 0x00,	/*  u16 ep_wMaxPacketSize; */
    0x01,	/*  u8  ep_bInterval; (255ms -- usb 2.0 spec) */
 
    /* endpoint two */
    0x07,	/*  u8  ep_bLength; */
    0x05,	/*  u8  ep_bDescriptorType; Endpoint */
    USB_DIR_IN | USB_SCO_EP,	/*  u8  ep_bEndpointAddress; */
    0x01,	/*  u8  ep_bmAttributes; Isochronous */
    0x19, 0x00,	/*  u16 ep_wMaxPacketSize; */
    0x01,	/*  u8  ep_bInterval; (255ms -- usb 2.0 spec) */

    /* interface two setting five */
    0x09,	/*  u8  if_bLength; */
    0x04,	/*  u8  if_bDescriptorType; Interface */
    0x01,	/*  u8  if_bInterfaceNumber; */
    0x04,	/*  u8  if_bAlternateSetting; */
    0x02,	/*  u8  if_bNumEndpoints; */
    0xe0,	/*  u8  if_bInterfaceClass; Wireless */
    0x01,	/*  u8  if_bInterfaceSubClass; Radio Frequency */
    0x01,	/*  u8  if_bInterfaceProtocol; Bluetooth */
    0x00,	/*  u8  if_iInterface; */
 
    /* endpoint one */
    0x07,	/*  u8  ep_bLength; */
    0x05,	/*  u8  ep_bDescriptorType; Endpoint */
    USB_DIR_OUT | USB_SCO_EP,	/*  u8  ep_bEndpointAddress; */
    0x01,	/*  u8  ep_bmAttributes; Isochronous */
    0x21, 0x00,	/*  u16 ep_wMaxPacketSize; */
    0x01,	/*  u8  ep_bInterval; (255ms -- usb 2.0 spec) */
 
    /* endpoint two */
    0x07,	/*  u8  ep_bLength; */
    0x05,	/*  u8  ep_bDescriptorType; Endpoint */
    USB_DIR_IN | USB_SCO_EP,	/*  u8  ep_bEndpointAddress; */
    0x01,	/*  u8  ep_bmAttributes; Isochronous */
    0x21, 0x00,	/*  u16 ep_wMaxPacketSize; */
    0x01,	/*  u8  ep_bInterval; (255ms -- usb 2.0 spec) */

    /* interface two setting six */
    0x09,	/*  u8  if_bLength; */
    0x04,	/*  u8  if_bDescriptorType; Interface */
    0x01,	/*  u8  if_bInterfaceNumber; */
    0x05,	/*  u8  if_bAlternateSetting; */
    0x02,	/*  u8  if_bNumEndpoints; */
    0xe0,	/*  u8  if_bInterfaceClass; Wireless */
    0x01,	/*  u8  if_bInterfaceSubClass; Radio Frequency */
    0x01,	/*  u8  if_bInterfaceProtocol; Bluetooth */
    0x00,	/*  u8  if_iInterface; */
 
    /* endpoint one */
    0x07,	/*  u8  ep_bLength; */
    0x05,	/*  u8  ep_bDescriptorType; Endpoint */
    USB_DIR_OUT | USB_SCO_EP,	/*  u8  ep_bEndpointAddress; */
    0x01,	/*  u8  ep_bmAttributes; Isochronous */
    0x31, 0x00,	/*  u16 ep_wMaxPacketSize; */
    0x01,	/*  u8  ep_bInterval; (255ms -- usb 2.0 spec) */

    /* endpoint two */
    0x07,	/*  u8  ep_bLength; */
    0x05,	/*  u8  ep_bDescriptorType; Endpoint */
    USB_DIR_IN | USB_SCO_EP,	/*  u8  ep_bEndpointAddress; */
    0x01,	/*  u8  ep_bmAttributes; Isochronous */
    0x31, 0x00,	/*  u16 ep_wMaxPacketSize; */
    0x01,	/*  u8  ep_bInterval; (255ms -- usb 2.0 spec) */
};

static void usb_bt_handle_reset(USBDevice *dev)
{
    struct USBBtState *s = (struct USBBtState *) dev->opaque;
    s->altsetting = 0;
}

static int usb_bt_handle_control(USBDevice *dev, int request, int value,
                int index, int length, uint8_t *data)
{
    struct USBBtState *s = (struct USBBtState *) dev->opaque;
    int ret = 0;

    switch (request) {
    case DeviceRequest | USB_REQ_GET_STATUS:
    case InterfaceRequest | USB_REQ_GET_STATUS:
    case EndpointRequest | USB_REQ_GET_STATUS:
        data[0] = (1 << USB_DEVICE_SELF_POWERED) |
            (dev->remote_wakeup << USB_DEVICE_REMOTE_WAKEUP);
        data[1] = 0x00;
        ret = 2;
        break;
    case DeviceOutRequest | USB_REQ_CLEAR_FEATURE:
    case InterfaceOutRequest | USB_REQ_CLEAR_FEATURE:
    case EndpointOutRequest | USB_REQ_CLEAR_FEATURE:
        if (value == USB_DEVICE_REMOTE_WAKEUP) {
            dev->remote_wakeup = 0;
        } else {
            goto fail;
        }
        ret = 0;
        break;
    case DeviceOutRequest | USB_REQ_SET_FEATURE:
    case InterfaceOutRequest | USB_REQ_SET_FEATURE:
    case EndpointOutRequest | USB_REQ_SET_FEATURE:
        if (value == USB_DEVICE_REMOTE_WAKEUP) {
            dev->remote_wakeup = 1;
        } else {
            goto fail;
        }
        ret = 0;
        break;
    case DeviceOutRequest | USB_REQ_SET_ADDRESS:
        dev->addr = value;
        ret = 0;
        break;
    case DeviceRequest | USB_REQ_GET_DESCRIPTOR:
        switch (value >> 8) {
        case USB_DT_DEVICE:
            ret = sizeof(qemu_bt_dev_descriptor);
            memcpy(data, qemu_bt_dev_descriptor, ret);
            break;
        case USB_DT_CONFIG:
            ret = sizeof(qemu_bt_config_descriptor);
            memcpy(data, qemu_bt_config_descriptor, ret);
            break;
        case USB_DT_STRING:
            switch(value & 0xff) {
            case 0:
                /* language ids */
                data[0] = 4;
                data[1] = 3;
                data[2] = 0x09;
                data[3] = 0x04;
                ret = 4;
                break;
            default:
                goto fail;
            }
            break;
        default:
            goto fail;
        }
        break;
    case DeviceRequest | USB_REQ_GET_CONFIGURATION:
        data[0] = qemu_bt_config_descriptor[0x5];
        ret = 1;
        s->config = 0;
        break;
    case DeviceOutRequest | USB_REQ_SET_CONFIGURATION:
        ret = 0;
        if (value != qemu_bt_config_descriptor[0x5] && value != 0) {
            printf("%s: Wrong SET_CONFIGURATION request (%i)\n",
                            __FUNCTION__, value);
            goto fail;
        }
        s->config = 1;
        s->evt_len = 0;
        bt_hci_reset(&s->hci);
        break;
    case InterfaceRequest | USB_REQ_GET_INTERFACE:
        if (value != 0 || (index & ~1) || length != 1)
            goto fail;
        if (index == 1)
            data[0] = s->altsetting;
        else
            data[0] = 0;
        ret = 1;
        break;
    case InterfaceOutRequest | USB_REQ_SET_INTERFACE:
        if ((index & ~1) || length != 0 ||
                        (index == 1 && (value < 0 || value > 4)) ||
                        (index == 0 && value != 0)) {
            printf("%s: Wrong SET_INTERFACE request (%i, %i)\n",
                            __FUNCTION__, index, value);
            goto fail;
        }
        s->altsetting = value;
        ret = 0;
        break;
    case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_DEVICE) << 8):
        if (s->config)
            bt_submit_hci(&s->hci, length, data);
        break;
    default:
    fail:
        ret = USB_RET_STALL;
        break;
    }
    return ret;
}

static int usb_bt_event_dequeue(struct USBBtState *s, USBPacket *p)
{
    int pkt, ret;
    if (!s->evt_len)
        return 0;

    pkt = s->evt_start;
    ret = MIN(p->len, s->evt_fifo[pkt].len);

    if (!ret) {
        s->evt_len --;
        s->evt_start ++;
        s->evt_start &= EVT_FIFO_LEN_MASK;
    }

    memcpy(p->data, s->evt_fifo[pkt].pkt + s->evt_fifo[pkt].start, ret);
    s->evt_fifo[pkt].start += ret;
    s->evt_fifo[pkt].len -= ret;

    return ret;
}

static int usb_bt_handle_data(USBDevice *dev, USBPacket *p)
{
    struct USBBtState *s = (struct USBBtState *) dev->opaque;
    int ret = 0;

    if (!s->config)
        goto fail;

    switch (p->pid) {
    case USB_TOKEN_IN:
        switch (p->devep & 0xf) {
        case USB_EVT_EP:
            ret = usb_bt_event_dequeue(s, p);
            break;

        default:
            goto fail;
        }
        break;

    case USB_TOKEN_OUT:
        switch (p->devep & 0xf) {
        case USB_ACL_EP:
            bt_submit_acl(&s->hci, p->len, p->data);
            break;
        case USB_SCO_EP:
            bt_submit_sco(&s->hci, p->len, p->data);
            break;

        default:
            goto fail;
        }
        break;

    default:
    fail:
        ret = USB_RET_STALL;
        break;
    }
    return ret;
}

static void usb_bt_handle_destroy(USBDevice *dev)
{
    struct USBBtState *s = (struct USBBtState *) dev->opaque;

    bt_hci_done(&s->hci);
    qemu_free(s);
}

static uint8_t *usb_bt_evt_packet(void *opaque)
{
    struct USBBtState *s = (struct USBBtState *) opaque;
    return s->evt_fifo[(s->evt_start + s->evt_len) & EVT_FIFO_LEN_MASK].pkt;
}

static void usb_bt_evt_submit(void *opaque, int len)
{
    struct USBBtState *s = (struct USBBtState *) opaque;
    s->evt_fifo[(s->evt_start + s->evt_len) & EVT_FIFO_LEN_MASK].start = 0;
    s->evt_fifo[(s->evt_start + s->evt_len ++) & EVT_FIFO_LEN_MASK].len = len;
}

static void usb_bt_acl_submit(void *opaque, uint8_t *data, int len)
{
    struct USBBtState *s = (struct USBBtState *) opaque;
}

static void usb_bt_sco_submit(void *opaque, uint8_t *data, int len)
{
    struct USBBtState *s = (struct USBBtState *) opaque;
}

USBDevice *usb_bt_init(struct bt_piconet_s *net)
{
    struct USBBtState *s;

    s = qemu_mallocz(sizeof(struct USBBtState));
    if (!s)
        return NULL;
    s->dev.opaque = s;
    s->dev.speed = USB_SPEED_HIGH;
    s->dev.handle_packet = usb_generic_handle_packet;
    pstrcpy(s->dev.devname, sizeof(s->dev.devname), "QEMU BT dongle");

    s->dev.handle_reset = usb_bt_handle_reset;
    s->dev.handle_control = usb_bt_handle_control;
    s->dev.handle_data = usb_bt_handle_data;
    s->dev.handle_destroy = usb_bt_handle_destroy;

    bt_hci_init(&s->hci);
    s->hci.evt_packet = usb_bt_evt_packet;
    s->hci.evt_submit = usb_bt_evt_submit;
    s->hci.acl_submit = usb_bt_acl_submit;
    s->hci.sco_submit = usb_bt_sco_submit;
    s->hci.opaque = s;
    s->hci.net = net;

    return &s->dev;
}
