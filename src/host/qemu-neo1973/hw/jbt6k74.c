/*
 * Toshiba JBT6K74-AS(PI) chip - ASIC part of the Toppoly TD028TTEC1 LCM.
 *
 * Copyright (c) 2007 OpenMoko, Inc.
 * Author: Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This file is licensed under GNU GPL v2.
 */

#include "hw.h"

struct jbt6k74_s {
    uint8_t command;
    uint32_t parameter;
    uint32_t response;
    int dc;
    int bit;
    int bytes;
    int respbit;
    uint16_t regs[0x100];
};

#define REG_C		0x00
#define REG_R		0x00
#define REG_W		0x80
#define REG_MASK	0x7f
static const int jbt_reg_width[0x100] = {
    [0 ... 0xff] = -1,
    [0x00] = 0 | REG_C,
    [0x01] = 0 | REG_C,
    [0x04] = 3 | REG_R,
    [0x06] = 1 | REG_R,
    [0x07] = 1 | REG_R,
    [0x08] = 1 | REG_R,
    [0x09] = 4 | REG_R,
    [0x0a] = 1 | REG_R,
    [0x0b] = 1 | REG_R,
    [0x0c] = 1 | REG_R,
    [0x0d] = 1 | REG_R,
    [0x0e] = 1 | REG_R,
    [0x10] = 0 | REG_C,
    [0x11] = 0 | REG_C,
    [0x20] = 0 | REG_C,
    [0x21] = 0 | REG_C,
    [0x28] = 0 | REG_C,
    [0x29] = 0 | REG_C,
    [0x36] = 1 | REG_W,
    [0x3a] = 1 | REG_W,
    [0x3b] = 1 | REG_W,
    [0xb0] = 1 | REG_W,
    [0xb1] = 1 | REG_W,
    [0xb2] = 1 | REG_W,
    [0xb3] = 1 | REG_W,
    [0xb4] = 1 | REG_W,
    [0xb5] = 1 | REG_W,
    [0xb6] = 1 | REG_W,
    [0xb7] = 1 | REG_W,
    [0xb8] = 2 | REG_W,
    [0xb9] = 1 | REG_W,
    [0xba] = 1 | REG_W,
    [0xbb] = 1 | REG_W,
    [0xbc] = 1 | REG_W,
    [0xbd] = 1 | REG_W,
    [0xbe] = 1 | REG_W,
    [0xbf] = 1 | REG_W,
    [0xc0] = 1 | REG_W,
    [0xc1] = 1 | REG_W,
    [0xc2] = 1 | REG_W,
    [0xc3] = 2 | REG_W,
    [0xc4] = 2 | REG_W,
    [0xc5] = 2 | REG_W,
    [0xc6] = 2 | REG_W,
    [0xc7] = 2 | REG_W,
    [0xc8] = 1 | REG_W,
    [0xc9] = 1 | REG_W,
    [0xca] = 1 | REG_W,
    [0xcf] = 1 | REG_W,
    [0xd0] = 2 | REG_W,
    [0xd1] = 1 | REG_W,
    [0xd2] = 2 | REG_W,
    [0xd3] = 2 | REG_W,
    [0xd4] = 2 | REG_W,
    [0xd5] = 1 | REG_W,
    [0xd6] = 1 | REG_W,
    [0xd7] = 2 | REG_W,
    [0xd8] = 1 | REG_W,
    [0xd9] = 2 | REG_W,
    [0xda] = 2 | REG_R,
    [0xde] = 2 | REG_W,
    [0xdf] = 2 | REG_W,
    [0xe0] = 1 | REG_W,
    [0xe1] = 1 | REG_W,
    [0xe2] = 1 | REG_W,
    [0xe3] = 1 | REG_W,
    [0xe4] = 2 | REG_W,
    [0xe5] = 2 | REG_W,
    [0xe6] = 1 | REG_W,
    [0xe7] = 2 | REG_W,
    [0xe8] = 1 | REG_W,
    [0xe9] = 1 | REG_W,
    [0xea] = 2 | REG_W,
    [0xeb] = 2 | REG_R,
    [0xec] = 2 | REG_W,
    [0xed] = 2 | REG_W,
};

static const uint16_t jbt_reg_default[0x100] = {
    [0 ... 0xff] = 0,
    [0x0c] = 0x0060, [0x3a] = 0x0060, [0xb0] = 0x0016, [0xb1] = 0x005a,
    [0xb2] = 0x0033, [0xb3] = 0x0011, [0xb4] = 0x0004, [0xb5] = 0x0020,
    [0xb6] = 0x0040, [0xb7] = 0x0002, [0xb8] = 0xfff5, [0xb9] = 0x0024,
    [0xba] = 0x0001, [0xbd] = 0x0002, [0xbf] = 0x0011, [0xc0] = 0x0011,
    [0xc1] = 0x0011, [0xc2] = 0x0011, [0xc3] = 0x2040, [0xc4] = 0x3060,
    [0xc5] = 0x1020, [0xc6] = 0x60c0, [0xc7] = 0x3343, [0xc8] = 0x0044,
    [0xc9] = 0x0033, [0xcf] = 0x0002, [0xd0] = 0x0804, [0xd1] = 0x0001,
    [0xd2] = 0x001e, [0xd3] = 0x1428, [0xd4] = 0x2864, [0xd5] = 0x0028,
    [0xd6] = 0x0002, [0xd7] = 0x0804, [0xd8] = 0x0001, [0xd9] = 0x0008,
    [0xda] = 0x7410, [0xde] = 0x050a, [0xdf] = 0x0a19, [0xe0] = 0x000a,
    [0xe3] = 0x0032, [0xe4] = 0x0003, [0xe5] = 0x0204, [0xe6] = 0x0003,
    [0xe7] = 0x040a, [0xe8] = 0x0004, [0xe9] = 0x0010, [0xea] = 0x1010,
    [0xeb] = 0x2040, [0xec] = 0x01f0, [0xed] = 0x00ff,
};

static void jbt6k74_reset(struct jbt6k74_s *s)
{
    s->command = 0;
    s->parameter = 0;
    s->bytes = 0;
    s->bit = 0;
    s->respbit = 0;
    memcpy(s->regs, jbt_reg_default, sizeof(s->regs));
}

static uint32_t jbt6k74_command(struct jbt6k74_s *s,
                uint8_t reg, uint16_t value)
{
    s->respbit = (jbt_reg_width[reg] & REG_MASK) << 3;
    switch (reg) {
    case 0x00:	/* No-op */
        break;
    case 0x01:	/* Reset */
        jbt6k74_reset(s);
        break;
    case 0x10:	/* Sleep-in */
    case 0x11:	/* Sleep-out */
    case 0x20:	/* Inversion off */
    case 0x21:	/* Inversion on */
        break;
    case 0x28:	/* Display off */
        printf("%s: Display off.\n", __FUNCTION__);
        break;
    case 0x29:	/* Display on */
        printf("%s: Display on.\n", __FUNCTION__);
        break;
    case 0x04:	/* Identification */
        return 0x748010;
    case 0x09:	/* Status */
        return 0x00600000;
    case 0xeb:	/* VCS and VCOM */
        return (s->regs[0xb5] << 8) | s->regs[0xb6];
    default:
        if (jbt_reg_width[reg] & REG_W) {
            s->regs[reg] = value;
            s->respbit = 0;
        } else {
            return s->regs[reg];
        }
    }
    return 0;
}

uint8_t jbt6k74_btxrx(void *opaque, uint8_t value)
{
    struct jbt6k74_s *s = (struct jbt6k74_s *) opaque;
    uint8_t ret = 0;
    if (s->respbit)
        ret = (s->response >> -- s->respbit) & 1;

    if (s->bit ++ == 0) {
        s->dc = value;
        return ret;
    }

    if (!s->dc) {
        s->command <<= 1;
        s->command |= value;
    } else {
        s->parameter <<= 1;
        s->parameter |= value;
    }

    if (s->bit == 9) {
        s->bit = 0;
        if (s->dc)
            s->bytes ++;
        if (s->bytes >= (jbt_reg_width[s->command] & REG_MASK) ||
                        jbt_reg_width[s->command] == -1) {
            if (jbt_reg_width[s->command] == -1)
                printf("%s: Bad command %02x\n", __FUNCTION__, s->command);
            else
                s->response = jbt6k74_command(s, s->command, s->parameter);
            s->parameter = 0;
            s->command = 0;
            s->bytes = 0;
        }
    }
    return ret;
}

uint8_t jbt6k74_txrx(void *opaque, uint8_t value)
{
    uint8_t ret = 0;
    int i;
    for (i = 0; i < 8; i ++)
        ret |= jbt6k74_btxrx(opaque, (value >> (7 - i)) & 1) << (7 - i);
    return ret;
}

static void jbt6k74_save(QEMUFile *f, void *opaque)
{
    struct jbt6k74_s *s = (struct jbt6k74_s *) opaque;
    int i;

    qemu_put_8s(f, &s->command);
    qemu_put_be32s(f, &s->parameter);
    qemu_put_be32s(f, &s->response);
    qemu_put_be32(f, s->dc);
    qemu_put_be32(f, s->bit);
    qemu_put_be32(f, s->bytes);
    qemu_put_be32(f, s->respbit);
    for (i = 0; i < 0x100; i ++)
        qemu_put_be16s(f, &s->regs[i]);
}

static int jbt6k74_load(QEMUFile *f, void *opaque, int version_id)
{
    struct jbt6k74_s *s = (struct jbt6k74_s *) opaque;
    int i;

    qemu_get_8s(f, &s->command);
    qemu_get_be32s(f, &s->parameter);
    qemu_get_be32s(f, &s->response);
    s->dc = qemu_get_be32(f);
    s->bit = qemu_get_be32(f);
    s->bytes = qemu_get_be32(f);
    s->respbit = qemu_get_be32(f);
    for (i = 0; i < 0x100; i ++)
        qemu_get_be16s(f, &s->regs[i]);

    return 0;
}

static int jbt6k74_iid = 0;

void *jbt6k74_init()
{
    struct jbt6k74_s *s = qemu_mallocz(sizeof(struct jbt6k74_s));
    jbt6k74_reset(s);
    register_savevm("jbt6k74", jbt6k74_iid ++, 0,
                    jbt6k74_save, jbt6k74_load, s);

    return s;
}
