/*
 * QEMU Bluetooth HCI logic.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */
#include "vl.h"

void bt_submit_lmp(struct bt_device_s *bt, int length, uint8_t *data)
{
    int resp, resplen, error, op, tr;
    uint8_t respdata[17];
    if (length < 1)
        return;

    tr = *data & 1;
    op = *(data ++) >> 1;
    resp = LMP_ACCEPTED;
    resplen = 2;
    respdata[1] = op;
    error = 0;
    length --;

    if (op >= 0x7c) {	/* Extended opcode */
        op |= *(data ++) << 8;
        resp = LMP_ACCEPTED_EXT;
        resplen = 4;
        respdata[0] = op >> 8;
        respdata[1] = op & 0xff;
        length --;
    }

    switch (op) {
    case LMP_ACCEPTED:
        /* data[0]	Op code
         */
        if (length < 1) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        resp = 0;
        break;

    case LMP_ACCEPTED_EXT:
        /* data[0]	Escape op code
         * data[1]	Extended op code
         */
        if (length < 2) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        resp = 0;
        break;

    case LMP_NOT_ACCEPTED:
        /* data[0]	Op code
         * data[1]	Error code
         */
        if (length < 2) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        resp = 0;
        break;

    case LMP_NOT_ACCEPTED_EXT:
        /* data[0]	Op code
         * data[1]	Extended op code
         * data[2]	Error code
         */
        if (length < 3) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        resp = 0;
        break;

    case LMP_HOST_CONNECTION_REQ:
        break;

    case LMP_SETUP_COMPLETE:
        resp = LMP_SETUP_COMPLETE;
        resplen = 1;
        bt->setup = 1;
        break;

    case LMP_DETACH:
        /* data[0]	Error code
         */
        if (length < 1) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        bt->setup = 0;
        resp = 0;
        break;

    case LMP_SUPERVISION_TIMEOUT:
        /* data[0,1]	Supervision timeout
         */
        if (length < 2) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        resp = 0;
        break;

    case LMP_QUALITY_OF_SERVICE:
        resp = 0;
        /* Fall through */
    case LMP_QOS_REQ:
        /* data[0,1]	Poll interval
         * data[2]	N(BC)
         */
        if (length < 3) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        break;

    case LMP_MAX_SLOT:
        resp = 0;
        /* Fall through */
    case LMP_MAX_SLOT_REQ:
        /* data[0]	Max slots
         */
        if (length < 1) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        break;

    case LMP_AU_RAND:
    case LMP_IN_RAND:
    case LMP_COMB_KEY:
        /* data[0-15]	Random number
         */
        if (length < 16) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        if (op == LMP_AU_RAND) {
            if (bt->key_present) {
                resp = LMP_SRES;
                resplen = 5;
                /* XXX: [Part H] Section 6.1 on page 801 */
            } else {
                error = HCI_PIN_OR_KEY_MISSING;
                goto not_accepted;
            }
        } else if (op == LMP_IN_RAND) {
            error = HCI_PAIRING_NOT_ALLOWED;
            goto not_accepted;
        } else {
            /* XXX: [Part H] Section 3.2 on page 779 */
            resp = LMP_UNIT_KEY;
            resplen = 17;
            memcpy(respdata + 1, bt->key, 16);

            error = HCI_UNIT_LINK_KEY_USED;
            goto not_accepted;
        }
        break;

    case LMP_UNIT_KEY:
        /* data[0-15]	Key
         */
        if (length < 16) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        memcpy(bt->key, data, 16);
        bt->key_present = 1;
        break;

    case LMP_SRES:
        /* data[0-3]	Authentication response
         */
        if (length < 4) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        break;

    case LMP_CLKOFFSET_REQ:
        resp = LMP_CLKOFFSET_RES;
        resplen = 3;
        respdata[1] = 0x33;
        respdata[2] = 0x33;
        break;

    case LMP_CLKOFFSET_RES:
        /* data[0,1]	Clock offset
         * (Slave to master only)
         */
        if (length < 2) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        break;

    case LMP_VERSION_REQ:
    case LMP_VERSION_RES:
        /* data[0]	VersNr
         * data[1,2]	CompId
         * data[3,4]	SubVersNr
         */
        if (length < 5) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        if (op == LMP_VERSION_REQ) {
            resp = LMP_VERSION_RES;
            resplen = 6;
            respdata[1] = 0x20;
            respdata[2] = 0xff;
            respdata[3] = 0xff;
            respdata[4] = 0xff;
            respdata[5] = 0xff;
        } else
            resp = 0;
        break;

    case LMP_FEATURES_REQ:
    case LMP_FEATURES_RES:
        /* data[0-7]	Features
         */
        if (length < 8) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        if (op == LMP_FEATURES_REQ) {
            resp = LMP_FEATURES_RES;
            resplen = 9;
            respdata[1] = (bt->lmp_caps >> 0) & 0xff;
            respdata[2] = (bt->lmp_caps >> 8) & 0xff;
            respdata[3] = (bt->lmp_caps >> 16) & 0xff;
            respdata[4] = (bt->lmp_caps >> 24) & 0xff;
            respdata[5] = (bt->lmp_caps >> 32) & 0xff;
            respdata[6] = (bt->lmp_caps >> 40) & 0xff;
            respdata[7] = (bt->lmp_caps >> 48) & 0xff;
            respdata[8] = (bt->lmp_caps >> 56) & 0xff;
        } else
            resp = 0;
        break;

    case LMP_NAME_REQ:
        /* data[0]	Name offset
         */
        if (length < 1) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        resp = LMP_NAME_RES;
        resplen = 17;
        respdata[1] = data[0];
        respdata[2] = strlen(bt->lmp_name);
        memset(respdata + 3, 0x00, 14);
        if (respdata[2] > respdata[1])
            memcpy(respdata + 3, bt->lmp_name + respdata[1],
                            respdata[2] - respdata[1]);
        break;

    case LMP_NAME_RES:
        /* data[0]	Name offset
         * data[1]	Name length
         * data[2-15]	Name fragment
         */
        if (length < 16) {
            error = HCI_UNSUPPORTED_LMP_PARAMETER_VALUE;
            goto not_accepted;
        }
        resp = 0;
        break;

    default:
        error = HCI_UNKNOWN_LMP_PDU;
        /* Fall through */
    not_accepted:
        if (op >> 8) {
            resp = LMP_NOT_ACCEPTED_EXT;
            resplen = 5;
            respdata[0] = op >> 8;
            respdata[1] = op & 0xff;
            respdata[2] = error;
        } else {
            resp = LMP_NOT_ACCEPTED;
            resplen = 3;
            respdata[0] = op & 0xff;
            respdata[1] = error;
        }
    }

    if (resp == 0)
        return;

    if (resp >> 8) {
        respdata[0] = resp >> 8;
        respdata[1] = resp & 0xff;
    } else
        respdata[0] = resp & 0xff;

    respdata[0] <<= 1;
    respdata[0] |= tr;
}

void bt_submit_raw_acl(struct bt_piconet_s *net, int length, uint8_t *data)
{
    struct bt_device_s *slave;
    if (length < 1)
        return;

    slave = net->slave;

    switch (data[0] & 3) {
    case LLID_ACLC:
        bt_submit_lmp(slave, length - 1, data + 1);
        break;
    case LLID_ACLU_START:
#if 0
        bt_sumbit_l2cap(slave, length - 1, data + 1, (data[0] >> 2) & 1);
        breka;
#endif
    default:
    case LLID_ACLU_CONT:
        break;
    }
}

/* XXX: handle endiannes */
#define HNDL(raw)	(raw)

static const uint8_t bt_event_reserved_mask[8] = {
    0xff, 0x9f, 0xfb, 0xff, 0x07, 0x18, 0x00, 0x00,
};

static inline uint8_t *bt_hci_event_start(struct bt_hci_s *hci,
                int evt, int len)
{
    uint8_t *packet, mask;
    int mask_byte;
    if (len > 255)
        cpu_abort(cpu_single_env, "HCI event params too long (%ib)\n", len);

    mask_byte = (evt - 1) >> 3;
    mask = 1 << ((evt - 1) & 3);
    if (mask & bt_event_reserved_mask[mask_byte] & ~hci->event_mask[mask_byte])
        return 0;

    packet = hci->evt_packet(hci->opaque);
    packet[0] = evt;
    packet[1] = len;
    return &packet[2];
}

static inline void bt_hci_event(struct bt_hci_s *hci, int evt,
                void *params, int len)
{
    uint8_t *packet = bt_hci_event_start(hci, evt, len);
    if (!packet)
        return;

    if (len)
        memcpy(packet, params, len);

    hci->evt_submit(hci->opaque, len + 2);
}

static inline void bt_hci_event_status(struct bt_hci_s *hci, int status)
{
    evt_cmd_status params = {
        .status	= status,
        .ncmd	= 5,
        .opcode	= hci->last_cmd,
    };

    bt_hci_event(hci, EVT_CMD_STATUS, &params, EVT_CMD_STATUS_SIZE);
}

static inline void bt_hci_event_complete(struct bt_hci_s *hci,
                void *ret, int len)
{
    uint8_t *packet = bt_hci_event_start(hci, EVT_CMD_COMPLETE,
                    len + EVT_CMD_COMPLETE_SIZE);
    evt_cmd_complete *params = (evt_cmd_complete *) packet;
    if (!packet)
        return;

    params->ncmd	= 5;
    params->opcode	= hci->last_cmd;
    if (len)
        memcpy(&packet[EVT_CMD_COMPLETE_SIZE], ret, len);

    hci->evt_submit(hci->opaque, len + EVT_CMD_COMPLETE_SIZE + 2);
}

static void bt_hci_inquiry_done(void *opaque)
{
    struct bt_hci_s *hci = (struct bt_hci_s *) opaque;
    bt_hci_event(hci, EVT_INQUIRY_COMPLETE, 0, 0);
}

static void bt_hci_inquiry_result(struct bt_hci_s *hci,
                struct bt_device_s *slave)
{
    inquiry_info params;

    if (slave->acl_mode != acl_active)
        return;

    hci->lm.responses_left --;
    hci->lm.responses ++;

    params.num_responses	= 1;
    bacpy(&params.bdaddr, &slave->bd_addr);
    params.pscan_rep_mode	= 0x00;	/* R0 */
    params.pscan_period_mode	= 0x00;	/* P0 - deprecated */
    params.pscan_mode		= 0x00;	/* Standard scan - deprecated */
    params.dev_class[0]		= slave->class[0];
    params.dev_class[1]		= slave->class[1];
    params.dev_class[2]		= slave->class[2];
    params.clock_offset		= slave->clkoff;	/* XXX: Endianness */
    bt_hci_event(hci, EVT_INQUIRY_RESULT, &params, INQUIRY_INFO_SIZE);

    if (hci->lm.periodic) {
        qemu_mod_timer(hci->lm.inquiry_next, qemu_get_clock(vm_clock) +
                        muldiv64(hci->lm.inquiry_period << 7,
                                ticks_per_sec, 1000));
    }
}

static void bt_hci_inquiry_start(struct bt_hci_s *hci, int length)
{
    struct bt_device_s *slave;

    hci->lm.inquiry_length = length;
    if (hci->lm.responses_left == 0)
        hci->lm.responses_left --;
    for (slave = hci->net->slave; slave; slave = slave->next)
        bt_hci_inquiry_result(hci, slave);

    if (hci->lm.responses_left)
        qemu_mod_timer(hci->lm.inquiry_done, qemu_get_clock(vm_clock) +
                        muldiv64(hci->lm.inquiry_length << 7,
                                ticks_per_sec, 1000));
    else
        bt_hci_inquiry_done(hci);
}

static void bt_hci_inquiry_next(void *opaque)
{
    struct bt_hci_s *hci = (struct bt_hci_s *) opaque;

    hci->lm.responses_left += hci->lm.responses;
    hci->lm.responses = 0;
    bt_hci_inquiry_start(hci,  hci->lm.inquiry_length);
}

static inline int bt_hci_handle_bad(struct bt_hci_s *hci, uint16_t handle)
{
    return handle < HCI_HANDLE_OFFSET || handle >= HCI_HANDLE_OFFSET + 16 ||
            !hci->lm.handle[handle & ~HCI_HANDLE_OFFSET];
}

static int bt_hci_connect(struct bt_hci_s *hci, bdaddr_t *bdaddr)
{
    struct bt_device_s *slave;
    uint16_t handle;
    evt_conn_complete params;

    for (slave = hci->net->slave; slave; slave = slave->next)
        if (slave->acl_mode == acl_active && !bacmp(&slave->bd_addr, bdaddr))
            break;
    if (!slave)
        return -ENODEV;

    slave->setup = 1;

    /* Make a connection handle */
    do {
        while (hci->lm.handle[++ hci->lm.last_handle])
            hci->lm.last_handle &= 15;
        handle = hci->lm.last_handle | HCI_HANDLE_OFFSET;
    } while (handle == hci->asb_handle || handle == hci->psb_handle);

    hci->lm.handle[hci->lm.last_handle] = slave;

    bt_hci_event_status(hci, HCI_SUCCESS);

    /* XXX: Send CONNCOMPLETE to slave */

    params.status	= HCI_SUCCESS;
    params.handle	= HNDL(handle);
    bacpy(&params.bdaddr, &slave->bd_addr);
    params.link_type	= ACL_LINK;
    params.encr_mode	= 0x00;		/* Encryption not required */
    bt_hci_event(hci, EVT_CONN_COMPLETE, &params, EVT_CONN_COMPLETE_SIZE);
    return 0;
}

static int bt_hci_disconnect(struct bt_hci_s *hci, uint16_t handle, int reason)
{
    struct bt_device_s *slave;
    evt_disconn_complete params;

    if (bt_hci_handle_bad(hci, handle))
        return -ENODEV;

    slave = hci->lm.handle[handle & ~HCI_HANDLE_OFFSET];

    bt_hci_event_status(hci, HCI_SUCCESS);

    /* XXX: send Disconnection Complete to remote dev. */
    slave->setup = 0;
    hci->lm.handle[handle & ~HCI_HANDLE_OFFSET] = 0;

    params.status	= HCI_SUCCESS;
    params.handle	= HNDL(handle);
    params.reason	= HCI_CONNECTION_TERMINATED;
    bt_hci_event(hci, EVT_DISCONN_COMPLETE,
                    &params, EVT_DISCONN_COMPLETE_SIZE);
    return 0;
}

static int bt_hci_name_req(struct bt_hci_s *hci, bdaddr_t *bdaddr)
{
    struct bt_device_s *slave;
    evt_remote_name_req_complete params;

    for (slave = hci->net->slave; slave; slave = slave->next)
        if (slave->acl_mode == acl_active && !bacmp(&slave->bd_addr, bdaddr))
            break;
    if (!slave)
        return -ENODEV;

    bt_hci_event_status(hci, HCI_SUCCESS);

    params.status       = HCI_SUCCESS;
    bacpy(&params.bdaddr, &slave->bd_addr);
    snprintf(params.name, sizeof(params.name), "%s", slave->lmp_name);
    bt_hci_event(hci, EVT_REMOTE_NAME_REQ_COMPLETE,
                    &params, EVT_REMOTE_NAME_REQ_COMPLETE_SIZE);
    return 0;
}

static int bt_hci_features_req(struct bt_hci_s *hci, uint16_t handle)
{
    struct bt_device_s *slave;
    evt_read_remote_features_complete params;

    if (bt_hci_handle_bad(hci, handle))
        return -ENODEV;

    slave = hci->lm.handle[handle & ~HCI_HANDLE_OFFSET];

    bt_hci_event_status(hci, HCI_SUCCESS);

    params.status	= HCI_SUCCESS;
    params.status	= HNDL(handle);
    params.features[0]	= (slave->lmp_caps >> 0) & 0xff;
    params.features[1]	= (slave->lmp_caps >> 8) & 0xff;
    params.features[2]	= (slave->lmp_caps >> 16) & 0xff;
    params.features[3]	= (slave->lmp_caps >> 24) & 0xff;
    params.features[4]	= (slave->lmp_caps >> 32) & 0xff;
    params.features[5]	= (slave->lmp_caps >> 40) & 0xff;
    params.features[6]	= (slave->lmp_caps >> 48) & 0xff;
    params.features[7]	= (slave->lmp_caps >> 56) & 0xff;
    bt_hci_event(hci, EVT_READ_REMOTE_FEATURES_COMPLETE,
                    &params, EVT_READ_REMOTE_FEATURES_COMPLETE_SIZE);
    return 0;
}

static int bt_hci_version_req(struct bt_hci_s *hci, uint16_t handle)
{
    struct bt_device_s *slave;
    evt_read_remote_version_complete params;

    if (bt_hci_handle_bad(hci, handle))
        return -ENODEV;

    slave = hci->lm.handle[handle & ~HCI_HANDLE_OFFSET];

    bt_hci_event_status(hci, HCI_SUCCESS);

    params.status	= HCI_SUCCESS;
    params.handle	= HNDL(handle);
    params.lmp_ver	= 0x03;
    params.manufacturer	= 0xa000;	/* XXX: Endianness */
    params.lmp_subver	= 0xa607;	/* XXX: Endianness */
    bt_hci_event(hci, EVT_READ_REMOTE_VERSION_COMPLETE,
                    &params, EVT_READ_REMOTE_VERSION_COMPLETE_SIZE);
    return 0;
}

static int bt_hci_clkoffset_req(struct bt_hci_s *hci, uint16_t handle)
{
    struct bt_device_s *slave;
    evt_read_clock_offset_complete params;

    if (bt_hci_handle_bad(hci, handle))
        return -ENODEV;

    slave = hci->lm.handle[handle & ~HCI_HANDLE_OFFSET];

    bt_hci_event_status(hci, HCI_SUCCESS);

    params.status	= HCI_SUCCESS;
    params.handle	= HNDL(handle);
    params.clock_offset	= slave->clkoff;	/* XXX: Endianness */
    bt_hci_event(hci, EVT_READ_CLOCK_OFFSET_COMPLETE,
                    &params, EVT_READ_CLOCK_OFFSET_COMPLETE_SIZE);
    return 0;
}

static void bt_hci_event_mode(struct bt_hci_s *hci, uint16_t handle,
                int mode, int interval)
{
    evt_mode_change params = {
        .status		= HCI_SUCCESS,
        .handle		= HNDL(handle),
        .mode		= mode,
        .interval	= interval,	/* XXX: Endianness */
    };
    bt_hci_event(hci, EVT_MODE_CHANGE, &params, EVT_MODE_CHANGE_SIZE);
}

static int bt_hci_mode_change(struct bt_hci_s *hci, uint16_t handle,
                int interval, int mode)
{
    struct bt_device_s *slave;

    if (bt_hci_handle_bad(hci, handle))
        return -ENODEV;
    slave = hci->lm.handle[handle & ~HCI_HANDLE_OFFSET];
    if (slave->acl_mode != acl_active) {
        bt_hci_event_status(hci, HCI_COMMAND_DISALLOWED);
        return 0;
    }

    bt_hci_event_status(hci, HCI_SUCCESS);

    slave->acl_mode = mode;
    qemu_mod_timer(slave->acl_mode_timer, qemu_get_clock(vm_clock) +
                            muldiv64(interval * 625, ticks_per_sec, 1000000));
    bt_hci_event_mode(hci, handle, mode, interval);
    return 0;
}

static int bt_hci_mode_cancel(struct bt_hci_s *hci, uint16_t handle, int mode)
{
    struct bt_device_s *slave;

    if (bt_hci_handle_bad(hci, handle))
        return -ENODEV;
    slave = hci->lm.handle[handle & ~HCI_HANDLE_OFFSET];
    if (slave->acl_mode != mode) {
        bt_hci_event_status(hci, HCI_COMMAND_DISALLOWED);
        return 0;
    }

    bt_hci_event_status(hci, HCI_SUCCESS);

    slave->acl_mode = acl_active;
    qemu_del_timer(slave->acl_mode_timer);

    bt_hci_event_mode(hci, handle, acl_active, 0);
    return 0;
}

static void bt_hci_mode_tick(void *opaque)
{
    uint16_t handle;
    struct bt_device_s **slave = (struct bt_device_s **) opaque;

    handle = HCI_HANDLE_OFFSET | (
                    (slave - (*slave)->acl_hci->lm.handle) /
                    sizeof(struct bt_device_s *));

    (*slave)->acl_mode = acl_active;
    bt_hci_event_mode((*slave)->acl_hci, handle, acl_active, 0);
}

void bt_hci_reset(struct bt_hci_s *hci)
{
    hci->acl_len = 0;
    hci->last_cmd = 0;

    hci->event_mask[0] = 0xff;
    hci->event_mask[1] = 0xff;
    hci->event_mask[2] = 0xff;
    hci->event_mask[3] = 0xff;
    hci->event_mask[4] = 0xff;
    hci->event_mask[5] = 0x1f;
    hci->event_mask[6] = 0x00;
    hci->event_mask[7] = 0x00;
    hci->scan_enable = SCAN_DISABLED;
    if (hci->local_name)
        free((void *) hci->local_name);
    hci->local_name = 0;
    hci->local_class[0] = 0x00;
    hci->local_class[1] = 0x00;
    hci->local_class[2] = 0x00;
    hci->voice_setting = 0x0000;

    /* XXX: qemu_del_timer(sl->acl_mode_timer); for all slaves */
    qemu_del_timer(hci->lm.inquiry_done);
    qemu_del_timer(hci->lm.inquiry_next);

    bt_hci_event_status(hci, HCI_SUCCESS);
}

static void bt_hci_read_local_version_rp(struct bt_hci_s *hci)
{
    read_local_version_rp lv = {
        .status		= HCI_SUCCESS,
        .hci_ver	= 0x03,
        .hci_rev	= 0xa607,	/* XXX: Endianness */
        .lmp_ver	= 0x03,
        .manufacturer	= 0xa000,	/* XXX: Endianness */
        .lmp_subver	= 0xa607,	/* XXX: Endianness */
    };

    bt_hci_event_complete(hci, &lv, READ_LOCAL_VERSION_RP_SIZE);
}

static void bt_hci_read_local_commands_rp(struct bt_hci_s *hci)
{
    read_local_commands_rp lc = {
        .status		= HCI_SUCCESS,
        .commands	= {
            0xbf, 0x80, 0xf9, 0x03, 0xb2, 0xc0, 0x03, 0xc3,
            0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0xe8, 0x13,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        },
    };

    bt_hci_event_complete(hci, &lc, READ_LOCAL_COMMANDS_RP_SIZE);
}

static void bt_hci_read_local_features_rp(struct bt_hci_s *hci)
{
    read_local_features_rp lf = {
        .status		= HCI_SUCCESS,
        .features	= {
            0x5f, 0x35, 0x85, 0x7e, 0x9b, 0x19, 0x00, 0x80,
        },
    };

    bt_hci_event_complete(hci, &lf, READ_LOCAL_FEATURES_RP_SIZE);
}

static void bt_hci_read_local_ext_features_rp(struct bt_hci_s *hci, int page)
{
    read_local_ext_features_rp lef = {
        .status		= HCI_SUCCESS,
        .page_num	= page,
        .max_page_num	= 0x00,
        .features	= {
            0x5f, 0x35, 0x85, 0x7e, 0x9b, 0x19, 0x00, 0x80,
        },
    };
    if (page)
        memset(lef.features, 0, sizeof(lef.features));

    bt_hci_event_complete(hci, &lef, READ_LOCAL_EXT_FEATURES_RP_SIZE);
}

static void bt_hci_read_buffer_size_rp(struct bt_hci_s *hci)
{
    read_buffer_size_rp bs = {
        .status		= HCI_SUCCESS,
        .acl_mtu	= 0x0180,	/* XXX: Endianness */
        .sco_mtu	= 0x40,
        .acl_max_pkt	= 0x0008,	/* XXX: Endianness */
        .sco_max_pkt	= 0x0008,	/* XXX: Endianness */
    };

    bt_hci_event_complete(hci, &bs, READ_BUFFER_SIZE_RP_SIZE);
}

static void bt_hci_read_country_code_rp(struct bt_hci_s *hci)
{
    /* This event seems to be undocumented, this code is a guess */
    struct {
        uint8_t status;
        uint8_t code;
    } cc = { 0x00, 0x00 };

    bt_hci_event_complete(hci, &cc, 2);
}

static void bt_hci_read_bd_addr_rp(struct bt_hci_s *hci)
{
    read_bd_addr_rp ba = {
        .status = HCI_SUCCESS,
        .bdaddr = {{ 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, }},
    };

    bt_hci_event_complete(hci, &ba, READ_BD_ADDR_RP_SIZE);
}

static int bt_hci_link_quality_rp(struct bt_hci_s *hci, uint16_t handle)
{
    read_link_quality_rp lq = {
        .status		= HCI_SUCCESS,
        .handle		= HNDL(handle),
        .link_quality	= 0xff,
    };

    if (bt_hci_handle_bad(hci, handle))
        lq.status = HCI_NO_CONNECTION;

    bt_hci_event_complete(hci, &lq, READ_LINK_QUALITY_RP_SIZE);
    return 0;
}

/* Generate a Command Complete event with only the Status parameter */
static inline void bt_hci_event_complete_status(struct bt_hci_s *hci,
                uint8_t status)
{
    bt_hci_event_complete(hci, &status, 1);
}

static inline void bt_hci_event_complete_conn_cancel(struct bt_hci_s *hci,
                bdaddr_t *bd_addr)
{
    int i;
    create_conn_cancel_rp params = {
        .status = HCI_NO_CONNECTION,
        .bdaddr = BAINIT(bd_addr),
    };

    for (i = 0; i < 16; i ++)
        if (hci->lm.handle[i] && !bacmp(&hci->lm.handle[i]->bd_addr, bd_addr))
            params.status = HCI_ACL_CONNECTION_EXISTS;

    bt_hci_event_complete(hci, &params, CREATE_CONN_CANCEL_RP_SIZE);
}

static inline void bt_hci_event_auth_complete(struct bt_hci_s *hci,
                uint16_t handle)
{
    evt_auth_complete params = {
        .status = HCI_SUCCESS,
        .handle = HNDL(handle),
    };

    bt_hci_event(hci, EVT_AUTH_COMPLETE, &params, EVT_AUTH_COMPLETE_SIZE);
}

static inline void bt_hci_event_encrypt_change(struct bt_hci_s *hci,
                uint16_t handle, uint8_t mode)
{
    evt_encrypt_change params = {
        .status		= HCI_SUCCESS,
        .handle		= HNDL(handle),
        .encrypt	= mode,
    };

    bt_hci_event(hci, EVT_ENCRYPT_CHANGE, &params, EVT_ENCRYPT_CHANGE_SIZE);
}

static inline void bt_hci_event_complete_name_cancel(struct bt_hci_s *hci,
                bdaddr_t *bd_addr)
{
    remote_name_req_cancel_rp params = {
        .status = HCI_INVALID_PARAMETERS,
        .bdaddr = BAINIT(bd_addr),
    };

    bt_hci_event_complete(hci, &params, REMOTE_NAME_REQ_CANCEL_RP_SIZE);
}

static inline void bt_hci_event_read_remote_ext_features(struct bt_hci_s *hci,
                uint16_t handle)
{
    evt_read_remote_ext_features_complete params = {
        .status = HCI_UNSUPPORTED_FEATURE,
        .handle = HNDL(handle),
        /* Rest uninitialised */
    };

    bt_hci_event(hci, EVT_READ_REMOTE_EXT_FEATURES_COMPLETE,
                    &params, EVT_READ_REMOTE_EXT_FEATURES_COMPLETE_SIZE);
}

static inline void bt_hci_event_complete_lmp_handle(struct bt_hci_s *hci,
                uint16_t handle)
{
    read_lmp_handle_rp params = {
        .status		= HCI_NO_CONNECTION,
        .handle		= HNDL(handle),
        .reserved	= 0,
        /* Rest uninitialised */
    };

    bt_hci_event_complete(hci, &params, READ_LMP_HANDLE_RP_SIZE);
}

static inline void bt_hci_event_complete_role_discovery(struct bt_hci_s *hci,
                int status, uint16_t handle)
{
    role_discovery_rp params = {
        .status		= status,
        .handle		= HNDL(handle),
        .role		= 0x00,	/* Master */
    };

    bt_hci_event_complete(hci, &params, ROLE_DISCOVERY_RP_SIZE);
}

static inline void bt_hci_event_complete_flush(struct bt_hci_s *hci,
                int status, uint16_t handle)
{
    flush_rp params = {
        .status		= status,
        .handle		= HNDL(handle),
    };

    bt_hci_event_complete(hci, &params, FLUSH_RP_SIZE);
}

static inline void bt_hci_event_complete_read_local_name(struct bt_hci_s *hci)
{
    read_local_name_rp params;
    params.status = HCI_SUCCESS;
    memset(params.name, 0, sizeof(params.name));
    if (hci->local_name)
        strncpy(params.name, hci->local_name, sizeof(params.name));

    bt_hci_event_complete(hci, &params, READ_LOCAL_NAME_RP_SIZE);
}

static inline void bt_hci_event_complete_read_scan_enable(struct bt_hci_s *hci)
{
    read_scan_enable_rp params = {
        .status = HCI_SUCCESS,
        .enable = hci->scan_enable,
    };

    bt_hci_event_complete(hci, &params, READ_SCAN_ENABLE_RP_SIZE);
}

static inline void bt_hci_event_complete_read_local_class(struct bt_hci_s *hci)
{
    read_class_of_dev_rp params;
    params.status = HCI_SUCCESS;
    memcpy(params.dev_class, hci->local_class, sizeof(params.dev_class));

    bt_hci_event_complete(hci, &params, READ_CLASS_OF_DEV_RP_SIZE);
}

static inline void bt_hci_event_complete_voice_setting(struct bt_hci_s *hci)
{
    read_voice_setting_rp params = {
        .status		= HCI_SUCCESS,
        .voice_setting	= hci->voice_setting,	/* XXX: Endianness */
    };

    bt_hci_event_complete(hci, &params, READ_VOICE_SETTING_RP_SIZE);
}

void bt_submit_hci(struct bt_hci_s *hci, int length, uint8_t *data)
{
    uint16_t cmd;
    int paramlen, i;

    if (length < 3)
        return;

    hci->last_cmd = *(uint16_t *) data;

    cmd = (data[1] << 8) | data[0];
    paramlen = data[2];
    if (cmd_opcode_ogf(cmd) == 0 || cmd_opcode_ocf(cmd) == 0)	/* NOP */
        return;

    data += 3;
    length -= 3;

    if (paramlen > length)
        return;

#define PARAM(cmd, param)	(((cmd *) data)->param)
#define PARAMHANDLE(cmd)	HNDL(((cmd *) data)->handle)
    switch (cmd) {
    case cmd_opcode_pack(OGF_LINK_CTL, OCF_INQUIRY):
        hci->lm.inquire = 1;
        hci->lm.periodic = 0;
        hci->lm.responses_left = PARAM(inquiry_cp, num_rsp);
        hci->lm.responses = 0;
        bt_hci_event_status(hci, HCI_SUCCESS);
        bt_hci_inquiry_start(hci, PARAM(inquiry_cp, length));
        break;

    case cmd_opcode_pack(OGF_LINK_CTL, OCF_INQUIRY_CANCEL):
        hci->lm.inquire = 0;
        qemu_del_timer(hci->lm.inquiry_done);
        bt_hci_event_complete_status(hci, HCI_SUCCESS);
        break;

    case cmd_opcode_pack(OGF_LINK_CTL, OCF_PERIODIC_INQUIRY):
        hci->lm.inquire = 1;
        hci->lm.periodic = 1;
        hci->lm.responses_left = PARAM(periodic_inquiry_cp, num_rsp);
        hci->lm.responses = 0;
        hci->lm.inquiry_period = PARAM(periodic_inquiry_cp, max_period);
        bt_hci_event_complete_status(hci, HCI_SUCCESS);
        bt_hci_inquiry_start(hci, PARAM(periodic_inquiry_cp, length));
        break;

    case cmd_opcode_pack(OGF_LINK_CTL, OCF_EXIT_PERIODIC_INQUIRY):
        hci->lm.inquire = 0;
        qemu_del_timer(hci->lm.inquiry_done);
        qemu_del_timer(hci->lm.inquiry_next);
        bt_hci_event_complete_status(hci, HCI_SUCCESS);
        break;

    case cmd_opcode_pack(OGF_LINK_CTL, OCF_CREATE_CONN):
        if (bt_hci_connect(hci, &PARAM(create_conn_cp, bdaddr)))
            bt_hci_event_status(hci, HCI_NO_CONNECTION);
        break;

    case cmd_opcode_pack(OGF_LINK_CTL, OCF_DISCONNECT):
        if (bt_hci_disconnect(hci, PARAMHANDLE(disconnect_cp),
                                PARAM(disconnect_cp, reason)))
            bt_hci_event_status(hci, HCI_NO_CONNECTION);
        break;

    case cmd_opcode_pack(OGF_LINK_CTL, OCF_CREATE_CONN_CANCEL):
        bt_hci_event_complete_conn_cancel(hci,
                        &PARAM(create_conn_cancel_cp, bdaddr));
        break;

    case cmd_opcode_pack(OGF_LINK_CTL, OCF_AUTH_REQUESTED):
        if (bt_hci_handle_bad(hci, PARAMHANDLE(auth_requested_cp)))
            bt_hci_event_status(hci, HCI_NO_CONNECTION);
        else {
            bt_hci_event_status(hci, HCI_SUCCESS);
            bt_hci_event_auth_complete(hci, PARAMHANDLE(auth_requested_cp));
        }
        break;

    case cmd_opcode_pack(OGF_LINK_CTL, OCF_SET_CONN_ENCRYPT):
        if (bt_hci_handle_bad(hci, PARAMHANDLE(set_conn_encrypt_cp)))
            bt_hci_event_status(hci, HCI_NO_CONNECTION);
        else {
            bt_hci_event_status(hci, HCI_SUCCESS);
            bt_hci_event_encrypt_change(hci,
                            PARAMHANDLE(set_conn_encrypt_cp),
                            PARAM(set_conn_encrypt_cp, encrypt));
        }
        break;

    case cmd_opcode_pack(OGF_LINK_CTL, OCF_REMOTE_NAME_REQ):
        if (bt_hci_name_req(hci, &PARAM(remote_name_req_cp, bdaddr)))
            bt_hci_event_status(hci, HCI_NO_CONNECTION);
        break;

    case cmd_opcode_pack(OGF_LINK_CTL, OCF_REMOTE_NAME_REQ_CANCEL):
        bt_hci_event_complete_name_cancel(hci,
                        &PARAM(remote_name_req_cancel_cp, bdaddr));
        break;

    case cmd_opcode_pack(OGF_LINK_CTL, OCF_READ_REMOTE_FEATURES):
        if (bt_hci_features_req(hci, PARAMHANDLE(read_remote_features_cp)))
            bt_hci_event_status(hci, HCI_NO_CONNECTION);
        break;

    case cmd_opcode_pack(OGF_LINK_CTL, OCF_READ_REMOTE_EXT_FEATURES):
        if (bt_hci_handle_bad(hci, PARAMHANDLE(read_remote_ext_features_cp)))
            bt_hci_event_status(hci, HCI_NO_CONNECTION);
        else {
            bt_hci_event_status(hci, HCI_SUCCESS);
            bt_hci_event_read_remote_ext_features(hci,
                            PARAMHANDLE(read_remote_ext_features_cp));
        }
        break;

    case cmd_opcode_pack(OGF_LINK_CTL, OCF_READ_REMOTE_VERSION):
        if (bt_hci_version_req(hci, PARAMHANDLE(read_remote_version_cp)))
            bt_hci_event_status(hci, HCI_NO_CONNECTION);
        break;

    case cmd_opcode_pack(OGF_LINK_CTL, OCF_READ_CLOCK_OFFSET):
        if (bt_hci_clkoffset_req(hci, PARAMHANDLE(read_clock_offset_cp)))
            bt_hci_event_status(hci, HCI_NO_CONNECTION);
        break;

    case cmd_opcode_pack(OGF_LINK_CTL, OCF_READ_LMP_HANDLE):
        bt_hci_event_complete_lmp_handle(hci, PARAMHANDLE(read_lmp_handle_cp));
        break;

    case cmd_opcode_pack(OGF_LINK_POLICY, OCF_HOLD_MODE):
        if (bt_hci_mode_change(hci, PARAMHANDLE(hold_mode_cp),
                                PARAM(hold_mode_cp, max_interval), acl_hold))
            bt_hci_event_status(hci, HCI_NO_CONNECTION);
        break;

    case cmd_opcode_pack(OGF_LINK_POLICY, OCF_PARK_MODE):
        if (bt_hci_mode_change(hci, PARAMHANDLE(park_mode_cp),
                                PARAM(park_mode_cp, max_interval), acl_parked))
            bt_hci_event_status(hci, HCI_NO_CONNECTION);
        break;

    case cmd_opcode_pack(OGF_LINK_POLICY, OCF_EXIT_PARK_MODE):
        if (bt_hci_mode_cancel(hci, PARAMHANDLE(exit_park_mode_cp),
                                acl_parked))
            bt_hci_event_status(hci, HCI_NO_CONNECTION);
        break;

    case cmd_opcode_pack(OGF_LINK_POLICY, OCF_ROLE_DISCOVERY):
        if (bt_hci_handle_bad(hci, PARAMHANDLE(role_discovery_cp)))
            bt_hci_event_complete_role_discovery(hci,
                            HCI_NO_CONNECTION, PARAMHANDLE(role_discovery_cp));
        else
            bt_hci_event_complete_role_discovery(hci,
                            HCI_SUCCESS, PARAMHANDLE(role_discovery_cp));
        break;

    case cmd_opcode_pack(OGF_HOST_CTL, OCF_SET_EVENT_MASK):
        memcpy(hci->event_mask, PARAM(set_event_mask_cp, mask), 8);
        bt_hci_event_complete_status(hci, HCI_SUCCESS);
        break;

    case cmd_opcode_pack(OGF_HOST_CTL, OCF_RESET):
        bt_hci_reset(hci);
        break;

    case cmd_opcode_pack(OGF_HOST_CTL, OCF_SET_EVENT_FLT):
        /* Filters are not implemented */
        bt_hci_event_complete_status(hci, HCI_SUCCESS);
        break;

    case cmd_opcode_pack(OGF_HOST_CTL, OCF_FLUSH):
        if (bt_hci_handle_bad(hci, PARAMHANDLE(flush_cp)))
            bt_hci_event_complete_flush(hci,
                            HCI_NO_CONNECTION, PARAMHANDLE(flush_cp));
        else {
            bt_hci_event(hci, EVT_FLUSH_OCCURRED,
                            &PARAM(flush_cp, handle),
                            EVT_FLUSH_OCCURRED_SIZE);
            bt_hci_event_complete_flush(hci,
                            HCI_SUCCESS, PARAMHANDLE(flush_cp));
        }
        break;

    case cmd_opcode_pack(OGF_HOST_CTL, OCF_CHANGE_LOCAL_NAME):
        if (hci->local_name)
            free((void *) hci->local_name);
        hci->local_name = strdup(PARAM(change_local_name_cp, name));
        bt_hci_event_complete_status(hci, HCI_SUCCESS);
        break;

    case cmd_opcode_pack(OGF_HOST_CTL, OCF_READ_LOCAL_NAME):
        bt_hci_event_complete_read_local_name(hci);
        break;

    case cmd_opcode_pack(OGF_HOST_CTL, OCF_READ_SCAN_ENABLE):
        bt_hci_event_complete_read_scan_enable(hci);
        break;

    case cmd_opcode_pack(OGF_HOST_CTL, OCF_WRITE_SCAN_ENABLE):
        hci->scan_enable = PARAM(write_scan_enable_cp, scan_enable);
        bt_hci_event_complete_status(hci, HCI_SUCCESS);
        break;

    case cmd_opcode_pack(OGF_HOST_CTL, OCF_READ_CLASS_OF_DEV):
        bt_hci_event_complete_read_local_class(hci);
        break;

    case cmd_opcode_pack(OGF_HOST_CTL, OCF_WRITE_CLASS_OF_DEV):
        memcpy(hci->local_class, PARAM(write_class_of_dev_cp, dev_class), 3);
        bt_hci_event_complete_status(hci, HCI_SUCCESS);
        break;

    case cmd_opcode_pack(OGF_HOST_CTL, OCF_READ_VOICE_SETTING):
        bt_hci_event_complete_voice_setting(hci);
        break;

    case cmd_opcode_pack(OGF_HOST_CTL, OCF_WRITE_VOICE_SETTING):
        hci->voice_setting = PARAM(write_voice_setting_cp, voice_setting);
        bt_hci_event_complete_status(hci, HCI_SUCCESS);
        break;

    case cmd_opcode_pack(OGF_HOST_CTL, OCF_HOST_NUMBER_OF_COMPLETED_PACKETS):
        for (i = 0; i < data[0]; i ++)
            if (bt_hci_handle_bad(hci,
                                    data[i * 2 + 1] | (data[i * 2 + 2] << 16)))
                bt_hci_event_complete_status(hci, HCI_INVALID_PARAMETERS);
        break;

    case cmd_opcode_pack(OGF_INFO_PARAM, OCF_READ_LOCAL_VERSION):
        bt_hci_read_local_version_rp(hci);
        break;

    case cmd_opcode_pack(OGF_INFO_PARAM, OCF_READ_LOCAL_COMMANDS):
        bt_hci_read_local_commands_rp(hci);
        break;

    case cmd_opcode_pack(OGF_INFO_PARAM, OCF_READ_LOCAL_FEATURES):
        bt_hci_read_local_features_rp(hci);
        break;

    case cmd_opcode_pack(OGF_INFO_PARAM, OCF_READ_LOCAL_EXT_FEATURES):
        bt_hci_read_local_ext_features_rp(hci,
                        PARAM(read_local_ext_features_cp, page_num));
        break;

    case cmd_opcode_pack(OGF_INFO_PARAM, OCF_READ_BUFFER_SIZE):
        bt_hci_read_buffer_size_rp(hci);
        break;

    case cmd_opcode_pack(OGF_INFO_PARAM, OCF_READ_COUNTRY_CODE):
        bt_hci_read_country_code_rp(hci);
        break;

    case cmd_opcode_pack(OGF_INFO_PARAM, OCF_READ_BD_ADDR):
        bt_hci_read_bd_addr_rp(hci);
        break;

    case cmd_opcode_pack(OGF_STATUS_PARAM, OCF_READ_LINK_QUALITY):
        bt_hci_link_quality_rp(hci, PARAMHANDLE(read_link_quality_cp));
        break;

    default:
        bt_hci_event_status(hci, HCI_UNKNOWN_COMMAND);
    }
}

void bt_submit_acl(struct bt_hci_s *hci, int length, uint8_t *data)
{
    uint16_t handle;
    int datalen, flags;

    if (length < 4)
        return;

    handle = acl_handle((data[1] << 8) | data[0]);
    flags = acl_flags((data[1] << 8) | data[0]);
    datalen = (data[3] << 8) | data[2];
    data += 4;
    length -= 4;

    if (datalen > length);
        return;

    switch (flags & 3) {
    case ACL_CONT:
        memcpy(hci->acl_buf + hci->acl_len, data + 4, datalen);
        hci->acl_len += datalen;
        break;
    case ACL_START:
        memcpy(hci->acl_buf, data + 4, datalen);
        hci->acl_len = datalen;
        break;
    default:
        return;
    }

    if (flags & ACL_ACTIVE_BCAST)
        hci->asb_handle = handle;

    if (flags & ACL_PICO_BCAST)
        hci->psb_handle = handle;

    /* TODO */
}

void bt_submit_sco(struct bt_hci_s *hci, int length, uint8_t *data)
{
    uint16_t handle;
    int datalen;

    if (length < 3)
        return;

    handle = acl_handle((data[1] << 8) | data[0]);
    datalen = data[2];
    data += 3;
    length -= 3;

    if (datalen > length);
        return;

    /* TODO */
}

void bt_hci_init(struct bt_hci_s *hci)
{
    hci->lm.inquiry_done = qemu_new_timer(vm_clock, bt_hci_inquiry_done, hci);
    hci->lm.inquiry_next = qemu_new_timer(vm_clock, bt_hci_inquiry_next, hci);
}

void bt_hci_done(struct bt_hci_s *hci)
{
    if (hci->local_name)
        free((void *) hci->local_name);

    /* XXX: Send DISCONNECT to all slaves */
    qemu_free_timer(hci->lm.inquiry_done);
    qemu_free_timer(hci->lm.inquiry_next);
}
