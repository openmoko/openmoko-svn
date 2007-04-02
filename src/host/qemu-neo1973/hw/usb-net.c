/*
 * QEMU USB Net devices
 * 
 * Copyright (c) 2006 Thomas Sailer
 * based on usb-hid.c Copyright (c) 2005 Fabrice Bellard
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "vl.h"
#include "../audio/sys-queue.h"

typedef uint32_t __le32;
#include "ndis.h"

/*#define TRAFFIC_DEBUG*/
/* Thanks to NetChip Technologies for donating this product ID.
 * It's for devices with only CDC Ethernet configurations.
 */
#define CDC_VENDOR_NUM          0x0525  /* NetChip */
#define CDC_PRODUCT_NUM         0xa4a1  /* Linux-USB Ethernet Gadget */
/* For hardware that can talk RNDIS and either of the above protocols,
 * use this ID ... the windows INF files will know it.
 */
#define RNDIS_VENDOR_NUM        0x0525  /* NetChip */
#define RNDIS_PRODUCT_NUM       0xa4a2  /* Ethernet/RNDIS Gadget */

#define STRING_MANUFACTURER             1
#define STRING_PRODUCT                  2
#define STRING_ETHADDR                  3
#define STRING_DATA                     4
#define STRING_CONTROL                  5
#define STRING_RNDIS_CONTROL            6
#define STRING_CDC                      7
#define STRING_SUBSET                   8
#define STRING_RNDIS                    9
#define STRING_SERIALNUMBER             10

#define DEV_CONFIG_VALUE        1       /* cdc or subset */
#define DEV_RNDIS_CONFIG_VALUE  2       /* rndis; optional */

#define USB_CDC_SUBCLASS_ACM                    0x02
#define USB_CDC_SUBCLASS_ETHERNET               0x06

#define USB_CDC_PROTO_NONE                      0
#define USB_CDC_ACM_PROTO_VENDOR                0xff

#define USB_CDC_HEADER_TYPE             0x00            /* header_desc */
#define USB_CDC_CALL_MANAGEMENT_TYPE    0x01            /* call_mgmt_descriptor */
#define USB_CDC_ACM_TYPE                0x02            /* acm_descriptor */
#define USB_CDC_UNION_TYPE              0x06            /* union_desc */
#define USB_CDC_ETHERNET_TYPE           0x0f            /* ether_desc */

#define USB_DT_CS_INTERFACE             0x24
#define USB_DT_CS_ENDPOINT              0x25

#define ClassInterfaceRequest \
        ((USB_DIR_IN|USB_TYPE_CLASS|USB_RECIP_INTERFACE)<<8)
#define ClassInterfaceOutRequest \
        ((USB_DIR_OUT|USB_TYPE_CLASS|USB_RECIP_INTERFACE)<<8)

#define USB_CDC_SEND_ENCAPSULATED_COMMAND       0x00
#define USB_CDC_GET_ENCAPSULATED_RESPONSE       0x01
#define USB_CDC_REQ_SET_LINE_CODING             0x20
#define USB_CDC_REQ_GET_LINE_CODING             0x21
#define USB_CDC_REQ_SET_CONTROL_LINE_STATE      0x22
#define USB_CDC_REQ_SEND_BREAK                  0x23
#define USB_CDC_SET_ETHERNET_MULTICAST_FILTERS  0x40
#define USB_CDC_SET_ETHERNET_PM_PATTERN_FILTER  0x41
#define USB_CDC_GET_ETHERNET_PM_PATTERN_FILTER  0x42
#define USB_CDC_SET_ETHERNET_PACKET_FILTER      0x43
#define USB_CDC_GET_ETHERNET_STATISTIC          0x44

#define USB_ENDPOINT_XFER_BULK          2
#define USB_ENDPOINT_XFER_INT           3

#define LOG2_STATUS_INTERVAL_MSEC       5       /* 1 << 5 == 32 msec */
#define STATUS_BYTECOUNT                16      /* 8 byte header + data */

#define ETH_FRAME_LEN                   1514    /* Max. octets in frame sans FCS */

/*
 * mostly the same descriptor as the linux gadget rndis driver
 */
static const uint8_t qemu_net_dev_descriptor[] = {
	0x12,                /*  u8 bLength; */
	USB_DT_DEVICE,       /*  u8 bDescriptorType; Device */
	0x00, 0x02,          /*  u16 bcdUSB; v2.0 */
	USB_CLASS_COMM,	     /*  u8  bDeviceClass; */
	0x00,	             /*  u8  bDeviceSubClass; */
	0x00,                /*  u8  bDeviceProtocol; [ low/full speeds only ] */
	0x40,                /*  u8  bMaxPacketSize0 */
	RNDIS_VENDOR_NUM & 0xff, RNDIS_VENDOR_NUM >> 8,   /*  u16 idVendor; */
 	RNDIS_PRODUCT_NUM & 0xff, RNDIS_PRODUCT_NUM >> 8, /*  u16 idProduct; */
	0x00, 0x00,          /*  u16 bcdDevice */
	STRING_MANUFACTURER, /*  u8  iManufacturer; */
	STRING_PRODUCT,      /*  u8  iProduct; */
	STRING_SERIALNUMBER, /*  u8  iSerialNumber; */
	0x02                 /*  u8  bNumConfigurations; */
};

static const uint8_t qemu_net_rndis_config_descriptor[] = {
	/* Configuration Descriptor */
	0x09,                /*  u8  bLength */
	USB_DT_CONFIG,       /*  u8  bDescriptorType */
        0x00, 0x00,          /*  le16 wTotalLength */
        0x02,                /*  u8  bNumInterfaces */
        DEV_RNDIS_CONFIG_VALUE, /*  u8  bConfigurationValue */
        STRING_RNDIS,        /*  u8  iConfiguration */
        0xc0,                /*  u8  bmAttributes */
        0x32,                /*  u8  bMaxPower */
	/* RNDIS Control Interface */
        0x09,                /*  u8  bLength */
        USB_DT_INTERFACE,    /*  u8  bDescriptorType */
        0x00,                /*  u8  bInterfaceNumber */
        0x00,                /*  u8  bAlternateSetting */
        0x01,                /*  u8  bNumEndpoints */
        USB_CLASS_COMM,           /*  u8  bInterfaceClass */
        USB_CDC_SUBCLASS_ACM,     /*  u8  bInterfaceSubClass */
        USB_CDC_ACM_PROTO_VENDOR, /*  u8  bInterfaceProtocol */
        STRING_RNDIS_CONTROL,     /*  u8  iInterface */
	/* Header Descriptor */
        0x05,                /*  u8    bLength */
        USB_DT_CS_INTERFACE, /*  u8    bDescriptorType */
        USB_CDC_HEADER_TYPE, /*  u8    bDescriptorSubType */
        0x10, 0x01,          /*  le16  bcdCDC */
	/* Call Management Descriptor */
        0x05,                /*  u8    bLength */
        USB_DT_CS_INTERFACE, /*  u8    bDescriptorType */
        USB_CDC_CALL_MANAGEMENT_TYPE, /*  u8    bDescriptorSubType */
        0x00,                /*  u8    bmCapabilities */
        0x01,                /*  u8    bDataInterface */
	/* ACM Descriptor */
        0x04,                /*  u8    bLength */
        USB_DT_CS_INTERFACE, /*  u8    bDescriptorType */
        USB_CDC_ACM_TYPE,    /*  u8    bDescriptorSubType */
        0x00,                /*  u8    bmCapabilities */
	/* Union Descriptor */
        0x05,                /*  u8    bLength */
        USB_DT_CS_INTERFACE, /*  u8    bDescriptorType */
        USB_CDC_UNION_TYPE,  /*  u8    bDescriptorSubType */
        0x00,                /*  u8    bMasterInterface0 */
        0x01,                /*  u8    bSlaveInterface0 */
	/* Status Descriptor */
        0x07,                /*  u8  bLength */
        USB_DT_ENDPOINT,     /*  u8  bDescriptorType */
        USB_DIR_IN | 1,      /*  u8  bEndpointAddress */
        USB_ENDPOINT_XFER_INT, /*  u8  bmAttributes */
        STATUS_BYTECOUNT & 0xff, STATUS_BYTECOUNT >> 8, /*  le16 wMaxPacketSize */
        1 << LOG2_STATUS_INTERVAL_MSEC, /*  u8  bInterval */
	/* RNDIS Data Interface */
        0x09,                /*  u8  bLength */
        USB_DT_INTERFACE,    /*  u8  bDescriptorType */
        0x01,                /*  u8  bInterfaceNumber */
        0x00,                /*  u8  bAlternateSetting */
        0x02,                /*  u8  bNumEndpoints */
        USB_CLASS_CDC_DATA,  /*  u8  bInterfaceClass */
        0x00,                /*  u8  bInterfaceSubClass */
        0x00,                /*  u8  bInterfaceProtocol */
        STRING_DATA,         /*  u8  iInterface */
	/* Source Endpoint */
        0x07,                /*  u8  bLength */
        USB_DT_ENDPOINT,     /*  u8  bDescriptorType */
        USB_DIR_IN | 2,      /*  u8  bEndpointAddress */
        USB_ENDPOINT_XFER_BULK, /*  u8  bmAttributes */
        0x40, 0x00,          /*  le16 wMaxPacketSize */
        0x00,                /*  u8  bInterval */
	/* Sink Endpoint */
        0x07,                /*  u8  bLength */
        USB_DT_ENDPOINT,     /*  u8  bDescriptorType */
        USB_DIR_OUT | 2,     /*  u8  bEndpointAddress */
        USB_ENDPOINT_XFER_BULK, /*  u8  bmAttributes */
        0x40, 0x00,          /*  le16 wMaxPacketSize */
        0x00                 /*  u8  bInterval */
};

static const uint8_t qemu_net_cdc_config_descriptor[] = {
	/* Configuration Descriptor */
	0x09,                /*  u8  bLength */
	USB_DT_CONFIG,       /*  u8  bDescriptorType */
        0x00, 0x00,          /*  le16 wTotalLength */
        0x02,                /*  u8  bNumInterfaces */
        DEV_RNDIS_CONFIG_VALUE, /*  u8  bConfigurationValue */
        STRING_RNDIS,        /*  u8  iConfiguration */
        0xc0,                /*  u8  bmAttributes */
        0x32,                /*  u8  bMaxPower */
	/* CDC Control Interface */
        0x09,                /*  u8  bLength */
        USB_DT_INTERFACE,    /*  u8  bDescriptorType */
        0x00,                /*  u8  bInterfaceNumber */
        0x00,                /*  u8  bAlternateSetting */
        0x01,                /*  u8  bNumEndpoints */
        USB_CLASS_COMM,            /*  u8  bInterfaceClass */
        USB_CDC_SUBCLASS_ETHERNET, /*  u8  bInterfaceSubClass */
        USB_CDC_PROTO_NONE,        /*  u8  bInterfaceProtocol */
        STRING_CONTROL,            /*  u8  iInterface */
	/* Header Descriptor */
        0x05,                /*  u8    bLength */
        USB_DT_CS_INTERFACE, /*  u8    bDescriptorType */
        USB_CDC_HEADER_TYPE, /*  u8    bDescriptorSubType */
        0x10, 0x01,          /*  le16  bcdCDC */
	/* Union Descriptor */
        0x05,                /*  u8    bLength */
        USB_DT_CS_INTERFACE, /*  u8    bDescriptorType */
        USB_CDC_UNION_TYPE,  /*  u8    bDescriptorSubType */
        0x00,                /*  u8    bMasterInterface0 */
        0x01,                /*  u8    bSlaveInterface0 */
	/* Ethernet Descriptor */
        0x0d,                /*  u8    bLength */
        USB_DT_CS_INTERFACE, /*  u8    bDescriptorType */
        USB_CDC_ETHERNET_TYPE,  /*  u8    bDescriptorSubType */
        STRING_ETHADDR,         /*  u8    iMACAddress */
        0x00, 0x00, 0x00, 0x00, /*  le32  bmEthernetStatistics */
        ETH_FRAME_LEN & 0xff, ETH_FRAME_LEN >> 8, /*  le16  wMaxSegmentSize */
        0x00, 0x00,          /*  le16  wNumberMCFilters */
        0x00,                /*  u8    bNumberPowerFilters */
	/* Status Descriptor */
        0x07,                /*  u8  bLength */
        USB_DT_ENDPOINT,     /*  u8  bDescriptorType */
        USB_DIR_IN | 1,      /*  u8  bEndpointAddress */
        USB_ENDPOINT_XFER_INT, /*  u8  bmAttributes */
        STATUS_BYTECOUNT & 0xff, STATUS_BYTECOUNT >> 8, /*  le16 wMaxPacketSize */
        1 << LOG2_STATUS_INTERVAL_MSEC, /*  u8  bInterval */
	/* CDC Data (nop) Interface */
        0x09,                /*  u8  bLength */
        USB_DT_INTERFACE,    /*  u8  bDescriptorType */
        0x01,                /*  u8  bInterfaceNumber */
        0x00,                /*  u8  bAlternateSetting */
        0x00,                /*  u8  bNumEndpoints */
        USB_CLASS_CDC_DATA,  /*  u8  bInterfaceClass */
        0x00,                /*  u8  bInterfaceSubClass */
        0x00,                /*  u8  bInterfaceProtocol */
        0x00,                /*  u8  iInterface */
	/* CDC Data Interface */
        0x09,                /*  u8  bLength */
        USB_DT_INTERFACE,    /*  u8  bDescriptorType */
        0x01,                /*  u8  bInterfaceNumber */
        0x01,                /*  u8  bAlternateSetting */
        0x02,                /*  u8  bNumEndpoints */
        USB_CLASS_CDC_DATA,  /*  u8  bInterfaceClass */
        0x00,                /*  u8  bInterfaceSubClass */
        0x00,                /*  u8  bInterfaceProtocol */
        STRING_DATA,         /*  u8  iInterface */
	/* Source Endpoint */
        0x07,                /*  u8  bLength */
        USB_DT_ENDPOINT,     /*  u8  bDescriptorType */
        USB_DIR_IN | 2,      /*  u8  bEndpointAddress */
        USB_ENDPOINT_XFER_BULK, /*  u8  bmAttributes */
        0x40, 0x00,          /*  le16 wMaxPacketSize */
        0x00,                /*  u8  bInterval */
	/* Sink Endpoint */
        0x07,                /*  u8  bLength */
        USB_DT_ENDPOINT,     /*  u8  bDescriptorType */
        USB_DIR_OUT | 2,     /*  u8  bEndpointAddress */
        USB_ENDPOINT_XFER_BULK, /*  u8  bmAttributes */
        0x40, 0x00,          /*  le16 wMaxPacketSize */
        0x00                 /*  u8  bInterval */
};

/*
 * RNDIS Status
 */

#define RNDIS_MAXIMUM_FRAME_SIZE        1518
#define RNDIS_MAX_TOTAL_SIZE            1558

/* Remote NDIS Versions */
#define RNDIS_MAJOR_VERSION             1
#define RNDIS_MINOR_VERSION             0

/* Status Values */
#define RNDIS_STATUS_SUCCESS            0x00000000U     /* Success           */
#define RNDIS_STATUS_FAILURE            0xC0000001U     /* Unspecified error */
#define RNDIS_STATUS_INVALID_DATA       0xC0010015U     /* Invalid data      */
#define RNDIS_STATUS_NOT_SUPPORTED      0xC00000BBU     /* Unsupported request */
#define RNDIS_STATUS_MEDIA_CONNECT      0x4001000BU     /* Device connected  */
#define RNDIS_STATUS_MEDIA_DISCONNECT   0x4001000CU     /* Device disconnected */

/* Message Set for Connectionless (802.3) Devices */
#define REMOTE_NDIS_PACKET_MSG          0x00000001U
#define REMOTE_NDIS_INITIALIZE_MSG      0x00000002U     /* Initialize device */
#define REMOTE_NDIS_HALT_MSG            0x00000003U
#define REMOTE_NDIS_QUERY_MSG           0x00000004U
#define REMOTE_NDIS_SET_MSG             0x00000005U
#define REMOTE_NDIS_RESET_MSG           0x00000006U
#define REMOTE_NDIS_INDICATE_STATUS_MSG 0x00000007U
#define REMOTE_NDIS_KEEPALIVE_MSG       0x00000008U

/* Message completion */
#define REMOTE_NDIS_INITIALIZE_CMPLT    0x80000002U
#define REMOTE_NDIS_QUERY_CMPLT         0x80000004U
#define REMOTE_NDIS_SET_CMPLT           0x80000005U
#define REMOTE_NDIS_RESET_CMPLT         0x80000006U
#define REMOTE_NDIS_KEEPALIVE_CMPLT     0x80000008U

/* Device Flags */
#define RNDIS_DF_CONNECTIONLESS         0x00000001U
#define RNDIS_DF_CONNECTION_ORIENTED    0x00000002U

#define RNDIS_MEDIUM_802_3              0x00000000U

/* from drivers/net/sk98lin/h/skgepnmi.h */
#define OID_PNP_CAPABILITIES                    0xFD010100
#define OID_PNP_SET_POWER                       0xFD010101
#define OID_PNP_QUERY_POWER                     0xFD010102
#define OID_PNP_ADD_WAKE_UP_PATTERN             0xFD010103
#define OID_PNP_REMOVE_WAKE_UP_PATTERN          0xFD010104
#define OID_PNP_ENABLE_WAKE_UP                  0xFD010106

typedef struct rndis_init_msg_type
{
        __le32  MessageType;
        __le32  MessageLength;
        __le32  RequestID;
        __le32  MajorVersion;
        __le32  MinorVersion;
        __le32  MaxTransferSize;
} rndis_init_msg_type;

typedef struct rndis_init_cmplt_type
{
        __le32  MessageType;
        __le32  MessageLength;
        __le32  RequestID;
        __le32  Status;
        __le32  MajorVersion;
        __le32  MinorVersion;
        __le32  DeviceFlags;
        __le32  Medium;
        __le32  MaxPacketsPerTransfer;
        __le32  MaxTransferSize;
        __le32  PacketAlignmentFactor;
        __le32  AFListOffset;
        __le32  AFListSize;
} rndis_init_cmplt_type;

typedef struct rndis_halt_msg_type
{
        __le32  MessageType;
        __le32  MessageLength;
        __le32  RequestID;
} rndis_halt_msg_type;

typedef struct rndis_query_msg_type
{
        __le32  MessageType;
        __le32  MessageLength;
        __le32  RequestID;
        __le32  OID;
        __le32  InformationBufferLength;
        __le32  InformationBufferOffset;
        __le32  DeviceVcHandle;
} rndis_query_msg_type;

typedef struct rndis_query_cmplt_type
{
        __le32  MessageType;
        __le32  MessageLength;
        __le32  RequestID;
        __le32  Status;
        __le32  InformationBufferLength;
        __le32  InformationBufferOffset;
} rndis_query_cmplt_type;

typedef struct rndis_set_msg_type
{
        __le32  MessageType;
        __le32  MessageLength;
        __le32  RequestID;
        __le32  OID;
        __le32  InformationBufferLength;
        __le32  InformationBufferOffset;
        __le32  DeviceVcHandle;
} rndis_set_msg_type;

typedef struct rndis_set_cmplt_type
{
        __le32  MessageType;
        __le32  MessageLength;
        __le32  RequestID;
        __le32  Status;
} rndis_set_cmplt_type;

typedef struct rndis_reset_msg_type
{
        __le32  MessageType;
        __le32  MessageLength;
        __le32  Reserved;
} rndis_reset_msg_type;

typedef struct rndis_reset_cmplt_type
{
        __le32  MessageType;
        __le32  MessageLength;
        __le32  Status;
        __le32  AddressingReset;
} rndis_reset_cmplt_type;

typedef struct rndis_indicate_status_msg_type
{
        __le32  MessageType;
        __le32  MessageLength;
        __le32  Status;
        __le32  StatusBufferLength;
        __le32  StatusBufferOffset;
} rndis_indicate_status_msg_type;

typedef struct rndis_keepalive_msg_type
{
        __le32  MessageType;
        __le32  MessageLength;
        __le32  RequestID;
} rndis_keepalive_msg_type;

typedef struct rndis_keepalive_cmplt_type
{
        __le32  MessageType;
        __le32  MessageLength;
        __le32  RequestID;
        __le32  Status;
} rndis_keepalive_cmplt_type;

struct rndis_packet_msg_type
{
        __le32  MessageType;
        __le32  MessageLength;
        __le32  DataOffset;
        __le32  DataLength;
        __le32  OOBDataOffset;
        __le32  OOBDataLength;
        __le32  NumOOBDataElements;
        __le32  PerPacketInfoOffset;
        __le32  PerPacketInfoLength;
        __le32  VcHandle;
        __le32  Reserved;
};

struct rndis_config_parameter
{
        __le32  ParameterNameOffset;
        __le32  ParameterNameLength;
        __le32  ParameterType;
        __le32  ParameterValueOffset;
        __le32  ParameterValueLength;
};

/* implementation specific */
enum rndis_state
{
        RNDIS_UNINITIALIZED,
        RNDIS_INITIALIZED,
        RNDIS_DATA_INITIALIZED,
};

static const uint32_t oid_supported_list[] =
{
        /* the general stuff */
        OID_GEN_SUPPORTED_LIST,
        OID_GEN_HARDWARE_STATUS,
        OID_GEN_MEDIA_SUPPORTED,
        OID_GEN_MEDIA_IN_USE,
        OID_GEN_MAXIMUM_FRAME_SIZE,
        OID_GEN_LINK_SPEED,
        OID_GEN_TRANSMIT_BLOCK_SIZE,
        OID_GEN_RECEIVE_BLOCK_SIZE,
        OID_GEN_VENDOR_ID,
        OID_GEN_VENDOR_DESCRIPTION,
        OID_GEN_VENDOR_DRIVER_VERSION,
        OID_GEN_CURRENT_PACKET_FILTER,
        OID_GEN_MAXIMUM_TOTAL_SIZE,
        OID_GEN_MEDIA_CONNECT_STATUS,
        OID_GEN_PHYSICAL_MEDIUM,
        /* the statistical stuff */
        OID_GEN_XMIT_OK,
        OID_GEN_RCV_OK,
        OID_GEN_XMIT_ERROR,
        OID_GEN_RCV_ERROR,
        OID_GEN_RCV_NO_BUFFER,
        /* mandatory 802.3 */
        /* the general stuff */
        OID_802_3_PERMANENT_ADDRESS,
        OID_802_3_CURRENT_ADDRESS,
        OID_802_3_MULTICAST_LIST,
        OID_802_3_MAC_OPTIONS,
        OID_802_3_MAXIMUM_LIST_SIZE,

        /* the statistical stuff */
        OID_802_3_RCV_ERROR_ALIGNMENT,
        OID_802_3_XMIT_ONE_COLLISION,
        OID_802_3_XMIT_MORE_COLLISIONS
};

struct rndis_response {
	TAILQ_ENTRY(rndis_response) entries;
	uint32_t length;
	uint8_t buf[0];
};


typedef struct USBNetState {
	USBDevice dev;

	unsigned int rndis;
	enum rndis_state rndis_state;
        uint32_t medium;
        uint32_t speed;
        uint32_t media_state;
	uint16_t filter;
       	uint32_t vendorid;
	uint8_t mac[6];

	unsigned int out_ptr;
	uint8_t out_buf[2048];

	USBPacket *inpkt;
	unsigned int in_ptr, in_len;
	uint8_t in_buf[2048];	

	VLANClientState *vc;
	TAILQ_HEAD(rndis_resp_head, rndis_response) rndis_resp;
} USBNetState;


static int ndis_query(USBNetState *s, uint32_t oid, uint8_t *inbuf, unsigned int inlen, uint8_t *outbuf)
{
	switch (oid) {
        /* general oids (table 4-1) */
        /* mandatory */
        case OID_GEN_SUPPORTED_LIST:
	{
                unsigned int i, count = sizeof(oid_supported_list) / sizeof(uint32_t);
                for (i = 0; i < count; i++)
                        ((__le32 *)outbuf)[i] = cpu_to_le32(oid_supported_list[i]);
                return sizeof(oid_supported_list);
	}

        /* mandatory */
	case OID_GEN_HARDWARE_STATUS:
		*((__le32 *)outbuf) = cpu_to_le32(0);
		return sizeof(__le32);
		
        /* mandatory */
        case OID_GEN_MEDIA_SUPPORTED:
		*((__le32 *)outbuf) = cpu_to_le32(s->medium);
		return sizeof(__le32);

        /* mandatory */
        case OID_GEN_MEDIA_IN_USE:
		*((__le32 *)outbuf) = cpu_to_le32(s->medium);
		return sizeof(__le32);

        /* mandatory */
        case OID_GEN_MAXIMUM_FRAME_SIZE:
		*((__le32 *)outbuf) = cpu_to_le32(ETH_FRAME_LEN);
		return sizeof(__le32);
		
        /* mandatory */
        case OID_GEN_LINK_SPEED:
		*((__le32 *)outbuf) = cpu_to_le32(s->speed);
		return sizeof(__le32);

        /* mandatory */
        case OID_GEN_TRANSMIT_BLOCK_SIZE:
		*((__le32 *)outbuf) = cpu_to_le32(ETH_FRAME_LEN);
		return sizeof(__le32);
		
        /* mandatory */
        case OID_GEN_RECEIVE_BLOCK_SIZE:
		*((__le32 *)outbuf) = cpu_to_le32(ETH_FRAME_LEN);
		return sizeof(__le32);

        /* mandatory */
        case OID_GEN_VENDOR_ID:
		*((__le32 *)outbuf) = cpu_to_le32(0x1234);
		return sizeof(__le32);

        /* mandatory */
        case OID_GEN_VENDOR_DESCRIPTION:
		strcpy(outbuf, "QEMU USB RNDIS Net");
		return strlen(outbuf) + 1;

       case OID_GEN_VENDOR_DRIVER_VERSION:
		*((__le32 *)outbuf) = cpu_to_le32(1);
		return sizeof(__le32);

        /* mandatory */
        case OID_GEN_CURRENT_PACKET_FILTER:
		*((__le32 *)outbuf) = cpu_to_le32(s->filter);
		return sizeof(__le32);

        /* mandatory */
        case OID_GEN_MAXIMUM_TOTAL_SIZE:
		*((__le32 *)outbuf) = cpu_to_le32(RNDIS_MAX_TOTAL_SIZE);
		return sizeof(__le32);

        /* mandatory */
        case OID_GEN_MEDIA_CONNECT_STATUS:
		*((__le32 *)outbuf) = cpu_to_le32(s->media_state);
		return sizeof(__le32);

        case OID_GEN_PHYSICAL_MEDIUM:
		*((__le32 *)outbuf) = cpu_to_le32(0);
		return sizeof(__le32);

        case OID_GEN_MAC_OPTIONS:
		*((__le32 *)outbuf) = cpu_to_le32(NDIS_MAC_OPTION_RECEIVE_SERIALIZED | NDIS_MAC_OPTION_FULL_DUPLEX);
		return sizeof(__le32);

        /* statistics OIDs (table 4-2) */
        /* mandatory */
        case OID_GEN_XMIT_OK:
		*((__le32 *)outbuf) = cpu_to_le32(0);
		return sizeof(__le32);

        /* mandatory */
        case OID_GEN_RCV_OK:
		*((__le32 *)outbuf) = cpu_to_le32(0);
		return sizeof(__le32);

        /* mandatory */
        case OID_GEN_XMIT_ERROR:
		*((__le32 *)outbuf) = cpu_to_le32(0);
		return sizeof(__le32);

        /* mandatory */
        case OID_GEN_RCV_ERROR:
		*((__le32 *)outbuf) = cpu_to_le32(0);
		return sizeof(__le32);

        /* mandatory */
        case OID_GEN_RCV_NO_BUFFER:
		*((__le32 *)outbuf) = cpu_to_le32(0);
		return sizeof(__le32);

        /* ieee802.3 OIDs (table 4-3) */
        /* mandatory */
        case OID_802_3_PERMANENT_ADDRESS:
		memcpy(outbuf, s->mac, 6);
		return 6;

        /* mandatory */
        case OID_802_3_CURRENT_ADDRESS:
		memcpy(outbuf, s->mac, 6);
		return 6;

        /* mandatory */
        case OID_802_3_MULTICAST_LIST:
		*((__le32 *)outbuf) = cpu_to_le32(0xE0000000);
		return sizeof(__le32);

        /* mandatory */
        case OID_802_3_MAXIMUM_LIST_SIZE:
		*((__le32 *)outbuf) = cpu_to_le32(1);
		return sizeof(__le32);

        case OID_802_3_MAC_OPTIONS:
		return 0;

        /* ieee802.3 statistics OIDs (table 4-4) */
        /* mandatory */
        case OID_802_3_RCV_ERROR_ALIGNMENT:
		*((__le32 *)outbuf) = cpu_to_le32(0);
		return sizeof(__le32);

        /* mandatory */
        case OID_802_3_XMIT_ONE_COLLISION:
		*((__le32 *)outbuf) = cpu_to_le32(0);
		return sizeof(__le32);

        /* mandatory */
        case OID_802_3_XMIT_MORE_COLLISIONS:
		*((__le32 *)outbuf) = cpu_to_le32(0);
		return sizeof(__le32);

	default:
		fprintf(stderr, "usbnet: unknown OID 0x%08x\n", oid);
		return 0;
	}
	return -1;
}

static int ndis_set(USBNetState *s, uint32_t oid, uint8_t *inbuf, unsigned int inlen)
{
	switch (oid) {
        case OID_GEN_CURRENT_PACKET_FILTER:
		s->filter = le32_to_cpup((__le32 *)inbuf);
		if (s->filter) {
			s->rndis_state = RNDIS_DATA_INITIALIZED;
		} else {
			s->rndis_state = RNDIS_INITIALIZED;
		}
		return 0;

        case OID_802_3_MULTICAST_LIST:
		return 0;

	}
	return -1;
}

static int rndis_get_response(USBNetState *s, uint8_t *buf)
{
	int ret = 0;
	struct rndis_response *r = s->rndis_resp.tqh_first;
	if (!r)
		return ret;
	TAILQ_REMOVE(&s->rndis_resp, r, entries);
	ret = r->length;
	memcpy(buf, r->buf, r->length);
	qemu_free(r);
	return ret;
}

static void *rndis_queue_response(USBNetState *s, unsigned int length)
{
	struct rndis_response *r = qemu_mallocz(sizeof(struct rndis_response) + length);
	if (!r)
		return NULL;
	TAILQ_INSERT_TAIL(&s->rndis_resp, r, entries);
	r->length = length;
	return &r->buf[0];
}

static void rndis_clear_responsequeue(USBNetState *s)
{
	struct rndis_response *r;
	
	while ((r = s->rndis_resp.tqh_first)) {
		TAILQ_REMOVE(&s->rndis_resp, r, entries);
		qemu_free(r);
	}
}

static int rndis_init_response(USBNetState *s, rndis_init_msg_type *buf)
{
	rndis_init_cmplt_type *resp = rndis_queue_response(s, sizeof(rndis_init_cmplt_type));
	if (!resp)
		return USB_RET_STALL;
	resp->MessageType = cpu_to_le32(REMOTE_NDIS_INITIALIZE_CMPLT);
        resp->MessageLength = cpu_to_le32(sizeof(rndis_init_cmplt_type));
        resp->RequestID = buf->RequestID; /* Still LE in msg buffer */
        resp->Status = cpu_to_le32(RNDIS_STATUS_SUCCESS);
        resp->MajorVersion = cpu_to_le32(RNDIS_MAJOR_VERSION);
        resp->MinorVersion = cpu_to_le32(RNDIS_MINOR_VERSION);
        resp->DeviceFlags = cpu_to_le32(RNDIS_DF_CONNECTIONLESS);
        resp->Medium = cpu_to_le32(RNDIS_MEDIUM_802_3);
        resp->MaxPacketsPerTransfer = cpu_to_le32(1);
        resp->MaxTransferSize = cpu_to_le32(ETH_FRAME_LEN + sizeof(struct rndis_packet_msg_type) + 22);
        resp->PacketAlignmentFactor = cpu_to_le32(0);
        resp->AFListOffset = cpu_to_le32(0);
        resp->AFListSize = cpu_to_le32(0);
        return 0;
}

static int rndis_query_response(USBNetState *s, rndis_query_msg_type *buf, unsigned int length)
{
        rndis_query_cmplt_type *resp;
	uint8_t infobuf[sizeof(oid_supported_list)]; /* oid_supported_list is the largest data reply */
	uint32_t bufoffs, buflen;
	int infobuflen;
	unsigned int resplen;
	bufoffs = le32_to_cpu(buf->InformationBufferOffset) + 8;
	buflen = le32_to_cpu(buf->InformationBufferLength);
	if (bufoffs + buflen > length)
		return USB_RET_STALL;
	infobuflen = ndis_query(s, le32_to_cpu(buf->OID), bufoffs + (uint8_t *)buf, buflen, infobuf);
	resplen = sizeof(rndis_query_cmplt_type) + ((infobuflen < 0) ? 0 : infobuflen);
	resp = rndis_queue_response(s, resplen);
	if (!resp)
		return USB_RET_STALL;
        resp->MessageType = cpu_to_le32(REMOTE_NDIS_QUERY_CMPLT);
        resp->RequestID = buf->RequestID; /* Still LE in msg buffer */
	resp->MessageLength = cpu_to_le32(resplen);
	if (infobuflen < 0) {
		/* OID not supported */
                resp->Status = cpu_to_le32(RNDIS_STATUS_NOT_SUPPORTED);
		resp->InformationBufferLength = cpu_to_le32(0);
                resp->InformationBufferOffset = cpu_to_le32(0);
		return 0;
	}
	resp->Status = cpu_to_le32(RNDIS_STATUS_SUCCESS);
	resp->InformationBufferOffset = cpu_to_le32(infobuflen ? sizeof(rndis_query_cmplt_type) - 8 : 0);
	resp->InformationBufferLength = cpu_to_le32(infobuflen);
	memcpy(resp + 1, infobuf, infobuflen);
	return 0;
}

static int rndis_set_response(USBNetState *s, rndis_set_msg_type *buf, unsigned int length)
{
        rndis_set_cmplt_type *resp = rndis_queue_response(s, sizeof(rndis_set_cmplt_type));
	uint32_t bufoffs, buflen;
	if (!resp)
		return USB_RET_STALL;
	bufoffs = le32_to_cpu(buf->InformationBufferOffset) + 8;
	buflen = le32_to_cpu(buf->InformationBufferLength);
	if (bufoffs + buflen > length)
		return USB_RET_STALL;
	int ret = ndis_set(s, le32_to_cpu(buf->OID), bufoffs + (uint8_t *)buf, buflen);
        resp->MessageType = cpu_to_le32(REMOTE_NDIS_SET_CMPLT);
        resp->RequestID = buf->RequestID; /* Still LE in msg buffer */
	resp->MessageLength = cpu_to_le32(sizeof(rndis_set_cmplt_type));
	if (ret < 0) {
		/* OID not supported */
                resp->Status = cpu_to_le32(RNDIS_STATUS_NOT_SUPPORTED);
		return 0;
	}
	resp->Status = cpu_to_le32(RNDIS_STATUS_SUCCESS);
	return 0;
}

static int rndis_reset_response(USBNetState *s, rndis_reset_msg_type *buf)
{
        rndis_reset_cmplt_type *resp = rndis_queue_response(s, sizeof(rndis_reset_cmplt_type));
	if (!resp)
		return USB_RET_STALL;
        resp->MessageType = cpu_to_le32(REMOTE_NDIS_RESET_CMPLT);
        resp->MessageLength = cpu_to_le32(sizeof(rndis_reset_cmplt_type));
        resp->Status = cpu_to_le32(RNDIS_STATUS_SUCCESS);
        /* resent information */
        resp->AddressingReset = cpu_to_le32(1);
	return 0;
}

static int rndis_keepalive_response(USBNetState *s, rndis_keepalive_msg_type *buf)
{
        rndis_keepalive_cmplt_type *resp = rndis_queue_response(s, sizeof(rndis_keepalive_cmplt_type));
	if (!resp)
		return USB_RET_STALL;
        resp->MessageType = cpu_to_le32(REMOTE_NDIS_KEEPALIVE_CMPLT);
        resp->MessageLength = cpu_to_le32(sizeof(rndis_keepalive_cmplt_type));
        resp->RequestID = buf->RequestID; /* Still LE in msg buffer */
        resp->Status = cpu_to_le32(RNDIS_STATUS_SUCCESS);
	return 0;
}

static int rndis_parse(USBNetState *s, uint8_t *data, int length)
{
        uint32_t MsgType, MsgLength;
        __le32 *tmp = (__le32 *)data;
        MsgType = le32_to_cpup(tmp++);
        MsgLength = le32_to_cpup(tmp++);
	
	switch (MsgType) {
	case REMOTE_NDIS_INITIALIZE_MSG:
		s->rndis_state = RNDIS_INITIALIZED;
		return rndis_init_response(s, (rndis_init_msg_type *)data);

        case REMOTE_NDIS_HALT_MSG:
		s->rndis_state = RNDIS_UNINITIALIZED;
                return 0;

	case REMOTE_NDIS_QUERY_MSG:
                return rndis_query_response(s, (rndis_query_msg_type *)data, length);

        case REMOTE_NDIS_SET_MSG:
                return rndis_set_response(s, (rndis_set_msg_type *)data, length);

        case REMOTE_NDIS_RESET_MSG:
		rndis_clear_responsequeue(s);
		s->out_ptr = s->in_ptr = s->in_len = 0;
                return rndis_reset_response(s, (rndis_reset_msg_type *)data);

        case REMOTE_NDIS_KEEPALIVE_MSG:
                /* For USB: host does this every 5 seconds */
                return rndis_keepalive_response(s, (rndis_keepalive_msg_type *)data);	
	}
	return USB_RET_STALL;
}

static void usb_net_handle_reset(USBDevice *dev)
{
}

static int usb_net_handle_control(USBDevice *dev, int request, int value,
                                  int index, int length, uint8_t *data)
{
	USBNetState *s = (USBNetState *)dev;
	int ret = 0;

	switch(request) {
	case DeviceRequest | USB_REQ_GET_STATUS:
		data[0] = (1 << USB_DEVICE_SELF_POWERED) |
			(dev->remote_wakeup << USB_DEVICE_REMOTE_WAKEUP);
		data[1] = 0x00;
		ret = 2;
		break;

	case DeviceOutRequest | USB_REQ_CLEAR_FEATURE:
		if (value == USB_DEVICE_REMOTE_WAKEUP) {
			dev->remote_wakeup = 0;
		} else {
			goto fail;
		}
		ret = 0;
		break;

	case DeviceOutRequest | USB_REQ_SET_FEATURE:
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

	case ClassInterfaceOutRequest | USB_CDC_SEND_ENCAPSULATED_COMMAND:
		if (!s->rndis || value || index != 0)
			goto fail;
#if TRAFFIC_DEBUG
		{
			unsigned int i;
			fprintf(stderr, "SEND_ENCAPSULATED_COMMAND:");
			for (i = 0; i < length; i++) {
				if (!(i & 15))
					fprintf(stderr, "\n%04X:", i);
				fprintf(stderr, " %02X", data[i]);
			}
			fprintf(stderr, "\n\n");
		}
#endif
		ret = rndis_parse(s, data, length);
		break;

	case ClassInterfaceRequest | USB_CDC_GET_ENCAPSULATED_RESPONSE:
		if (!s->rndis || value || index != 0)
			goto fail;
		ret = rndis_get_response(s, data);
		if (!ret) {
			data[0] = 0;
			ret = 1;
		}
#if TRAFFIC_DEBUG
		{
			unsigned int i;
			fprintf(stderr, "GET_ENCAPSULATED_RESPONSE:");
			for (i = 0; i < ret; i++) {
				if (!(i & 15))
					fprintf(stderr, "\n%04X:", i);
				fprintf(stderr, " %02X", data[i]);
			}
			fprintf(stderr, "\n\n");
		}
#endif
		break;

	case DeviceRequest | USB_REQ_GET_DESCRIPTOR:
		switch(value >> 8) {
		case USB_DT_DEVICE:
			ret = sizeof(qemu_net_dev_descriptor);
			memcpy(data, qemu_net_dev_descriptor, ret);
			break;

		case USB_DT_CONFIG:
			switch (value & 0xff) {
			case 0:
				ret = sizeof(qemu_net_rndis_config_descriptor);
				memcpy(data, qemu_net_rndis_config_descriptor, ret);
				break;

			case 1:
				ret = sizeof(qemu_net_cdc_config_descriptor);
				memcpy(data, qemu_net_cdc_config_descriptor, ret);
				break;

			default:
				goto fail;
			}
			data[2] = ret & 0xff;
			data[3] = ret >> 8;
			break;

		case USB_DT_STRING:
			switch (value & 0xff) {
			case 0:
				/* language ids */
				data[0] = 4;
				data[1] = 3;
				data[2] = 0x09;
				data[3] = 0x04;
				ret = 4;
				break;
				
			case STRING_MANUFACTURER:
				ret = set_usb_string(data, "QEMU");
				break;

			case STRING_PRODUCT:
				ret = set_usb_string(data, "RNDIS/QEMU USB Network Device");
				break;

			case STRING_ETHADDR:
				ret = set_usb_string(data, "400102030405");
				break;

			case STRING_DATA:
				ret = set_usb_string(data, "QEMU USB Net Data Interface");
				break;

			case STRING_CONTROL:
				ret = set_usb_string(data, "QEMU USB Net Control Interface");
				break;

			case STRING_RNDIS_CONTROL:
				ret = set_usb_string(data, "QEMU USB Net RNDIS Control Interface");
				break;

			case STRING_CDC:
				ret = set_usb_string(data, "QEMU USB Net CDC");
				break;

			case STRING_SUBSET:
				ret = set_usb_string(data, "QEMU USB Net Subset");
				break;

			case STRING_RNDIS:
				ret = set_usb_string(data, "QEMU USB Net RNDIS");
				break;

			case STRING_SERIALNUMBER:
				ret = set_usb_string(data, "1");
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
		data[0] = s->rndis ? DEV_RNDIS_CONFIG_VALUE : DEV_CONFIG_VALUE;
		ret = 1;
		break;

	case DeviceOutRequest | USB_REQ_SET_CONFIGURATION:
		switch (value & 0xff) {
		case DEV_CONFIG_VALUE:
			s->rndis = 0;
			break;

		case DEV_RNDIS_CONFIG_VALUE:
			s->rndis = 1;
			break;

		default:
			goto fail;
		}
		ret = 0;
		break;

	case DeviceRequest | USB_REQ_GET_INTERFACE:
		data[0] = 0;
		ret = 1;
		break;

	case DeviceOutRequest | USB_REQ_SET_INTERFACE:
		ret = 0;
		break;

	default:
	fail:
		fprintf(stderr, "usbnet: failed control transaction: request 0x%x value 0x%x index 0x%x length 0x%x\n",
			request, value, index, length);
		ret = USB_RET_STALL;
		break;
	}
	return ret;
}

static int usb_net_handle_statusin(USBNetState *s, USBPacket *p)
{
	int ret = 8;
	if (p->len < 8)
		return USB_RET_STALL;
	((__le32 *)p->data)[0] = cpu_to_le32(1);
	((__le32 *)p->data)[1] = cpu_to_le32(0);
	if (!s->rndis_resp.tqh_first)
		ret = USB_RET_NAK;
#if DEBUG
	fprintf(stderr, "usbnet: interrupt poll len %u return %d", p->len, ret);
	{
		int i;
		fprintf(stderr, ":");
		for (i = 0; i < ret; i++) {
			if (!(i & 15))
				fprintf(stderr, "\n%04X:", i);
			fprintf(stderr, " %02X", p->data[i]);
		}
		fprintf(stderr, "\n\n");
	}
#endif
	return ret;
}

static int usb_net_handle_datain(USBNetState *s, USBPacket *p)
{
	int ret = USB_RET_NAK;
	
	if (s->in_ptr > s->in_len) {
		s->in_ptr = s->in_len = 0;
		ret = USB_RET_NAK;
		return ret;
	}
	if (!s->in_len) {
		ret = USB_RET_NAK;
		return ret;
	}
	ret = s->in_len - s->in_ptr;
	if (ret > p->len)
		ret = p->len;
	memcpy(p->data, &s->in_buf[s->in_ptr], ret);
	s->in_ptr += ret;
	if (s->in_ptr >= s->in_len && (s->rndis || (s->in_len & (64-1)) || !ret)) {
		/* no short packet necessary */
		s->in_ptr = s->in_len = 0;
	}
#if TRAFFIC_DEBUG
	fprintf(stderr, "usbnet: data in len %u return %d", p->len, ret);
	{
		int i;
		fprintf(stderr, ":");
		for (i = 0; i < ret; i++) {
			if (!(i & 15))
				fprintf(stderr, "\n%04X:", i);
			fprintf(stderr, " %02X", p->data[i]);
		}
		fprintf(stderr, "\n\n");
	}
#endif
	return ret;
}

static int usb_net_handle_dataout(USBNetState *s, USBPacket *p)
{
	int ret = p->len;
	int sz = sizeof(s->out_buf) - s->out_ptr;
	struct rndis_packet_msg_type *msg = (struct rndis_packet_msg_type *)s->out_buf;
	uint32_t len;

#if TRAFFIC_DEBUG
	fprintf(stderr, "usbnet: data out len %u\n", p->len);
	{
		int i;
		fprintf(stderr, ":");
		for (i = 0; i < p->len; i++) {
			if (!(i & 15))
				fprintf(stderr, "\n%04X:", i);
			fprintf(stderr, " %02X", p->data[i]);
		}
		fprintf(stderr, "\n\n");
	}
#endif
	if (sz > ret)
		sz = ret;
	memcpy(&s->out_buf[s->out_ptr], p->data, sz);
	s->out_ptr += sz;
	if (!s->rndis) {
		if (ret < 64) {
			qemu_send_packet(s->vc, s->out_buf, s->out_ptr);
			s->out_ptr = 0;
		}
		return ret;
	}
	len = le32_to_cpu(msg->MessageLength);
	if (s->out_ptr < 8 || s->out_ptr < len)
		return ret;
	if (le32_to_cpu(msg->MessageType) == REMOTE_NDIS_PACKET_MSG) {
		uint32_t offs = 8 + le32_to_cpu(msg->DataOffset);
		uint32_t size = le32_to_cpu(msg->DataLength);
		if (offs + size <= len)
			qemu_send_packet(s->vc, s->out_buf + offs, size);
	}
	s->out_ptr -= len;
	memmove(s->out_buf, &s->out_buf[len], s->out_ptr);
	return ret;
}

static int usb_net_handle_data(USBDevice *dev, USBPacket *p)
{
	USBNetState *s = (USBNetState *)dev;
	int ret = 0;

	switch(p->pid) {
	case USB_TOKEN_IN:
		switch (p->devep) {
		case 1:
			ret = usb_net_handle_statusin(s, p);
			break;

		case 2:
			ret = usb_net_handle_datain(s, p);
			break;

		default:
			goto fail;
		}
		break;

	case USB_TOKEN_OUT:
		switch (p->devep) {
		case 2:
			ret = usb_net_handle_dataout(s, p);
			break;

		default:
			goto fail;
		}
		break;


#if 0
	case USB_TOKEN_IN:
		if (p->devep == 1) {
			if (s->kind == USB_MOUSE)
				ret = usb_mouse_poll(s, p->data, p->len);
			else if (s->kind == USB_TABLET)
				ret = usb_tablet_poll(s, p->data, p->len);
		} else {
			goto fail;
		}
		break;
	case USB_TOKEN_OUT:
#endif
	default:
	fail:
		ret = USB_RET_STALL;
		break;
	}
	if (ret == USB_RET_STALL)
		fprintf(stderr, "usbnet: failed data transaction: pid 0x%x ep 0x%x len 0x%x\n", p->pid, p->devep, p->len);
	return ret;
}

static void usbnet_receive(void *opaque, const uint8_t *buf, int size)
{
	USBNetState *s = opaque;

	if (s->rndis) {
		struct rndis_packet_msg_type *msg = (struct rndis_packet_msg_type *)s->in_buf;
		if (!s->rndis_state == RNDIS_DATA_INITIALIZED)
			return;
		if (size + sizeof(struct rndis_packet_msg_type) > sizeof(s->in_buf))
			return;
		memset(msg, 0, sizeof(struct rndis_packet_msg_type));
		msg->MessageType = cpu_to_le32(REMOTE_NDIS_PACKET_MSG);
		msg->MessageLength = cpu_to_le32(size + sizeof(struct rndis_packet_msg_type));
		msg->DataOffset = cpu_to_le32(sizeof(struct rndis_packet_msg_type) - 8);
		msg->DataLength = cpu_to_le32(size);
		//msg->OOBDataOffset;
		//msg->OOBDataLength;
		//msg->NumOOBDataElements;
		//msg->PerPacketInfoOffset;
		//msg->PerPacketInfoLength;
		//msg->VcHandle;
		//msg->Reserved;
		memcpy(msg + 1, buf, size);
		s->in_len = size + sizeof(struct rndis_packet_msg_type);
	} else {
		if (size > sizeof(s->in_buf))
			return;
		memcpy(s->in_buf, buf, size);
		s->in_len = size;
	}
	s->in_ptr = 0;
}

static int usbnet_can_receive(void *opaque)
{
	USBNetState *s = opaque;

	if (s->rndis && !s->rndis_state == RNDIS_DATA_INITIALIZED)
		return 1;
	return !s->in_len;
}

static void usb_net_handle_destroy(USBDevice *dev)
{
	USBNetState *s = (USBNetState *)dev;
	rndis_clear_responsequeue(s);
	qemu_free(s);
}

USBDevice *usb_net_init(NICInfo *nd)
{
	USBNetState *s;

	s = qemu_mallocz(sizeof(USBNetState));
	if (!s)
		return NULL;
	s->dev.speed = USB_SPEED_FULL;
	s->dev.handle_packet = usb_generic_handle_packet;

	s->dev.handle_reset = usb_net_handle_reset;
	s->dev.handle_control = usb_net_handle_control;
	s->dev.handle_data = usb_net_handle_data;
	s->dev.handle_destroy = usb_net_handle_destroy;

	s->rndis = 1;
	s->rndis_state = RNDIS_UNINITIALIZED;
        s->medium = NDIS_MEDIUM_802_3;
        s->speed = 1000000; /* 100MBps, in 100Bps units */
        s->media_state = NDIS_MEDIA_STATE_CONNECTED;
	s->filter = 0;
        s->vendorid = 0x1234;
	memcpy(s->mac, nd->macaddr, 6);
	TAILQ_INIT(&s->rndis_resp);

	pstrcpy(s->dev.devname, sizeof(s->dev.devname), "QEMU USB Network Interface");
	s->vc = qemu_new_vlan_client(nd->vlan, usbnet_receive, usbnet_can_receive, s);
	snprintf(s->vc->info_str, sizeof(s->vc->info_str),
		 "usbnet macaddr=%02x:%02x:%02x:%02x:%02x:%02x",
		 s->mac[0], s->mac[1], s->mac[2],
		 s->mac[3], s->mac[4], s->mac[5]);
	fprintf(stderr, "usbnet: initialized mac %02x:%02x:%02x:%02x:%02x:%02x\n",
		s->mac[0], s->mac[1], s->mac[2],
		s->mac[3], s->mac[4], s->mac[5]);
	return (USBDevice *)s;
}
