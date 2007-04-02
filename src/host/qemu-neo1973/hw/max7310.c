/*
 * MAX7310 8-port GPIO expansion chip.
 *
 * Copyright (c) 2006 Openedhand Ltd.
 * Written by Andrzej Zaborowski <balrog@zabor.org>
 *
 * This file is licensed under GNU GPL.
 */

#include "vl.h"

struct max7310_s {
    uint8_t level;
    uint8_t direction;
    uint8_t polarity;
    uint8_t status;
    uint8_t command;
    int i2c_dir;
    struct i2c_slave_s i2c;
    struct {
        gpio_handler_t fn;
        void *opaque;
    } handler[8];
};

void max7310_reset(struct i2c_slave_s *i2c)
{
    struct max7310_s *s = (struct max7310_s *) i2c->opaque;
    s->level &= s->direction;
    s->direction = 0xff;
    s->polarity = 0xf0;
    s->status = 0x01;
    s->command = 0x00;
}

static void max7310_start(void *opaque, int dir)
{
    struct max7310_s *s = (struct max7310_s *) opaque;
    s->i2c_dir = dir;
}

static int max7310_read(void *opaque, uint8_t *data, int len)
{
    struct max7310_s *s = (struct max7310_s *) opaque;

    switch (s->command) {
    case 0x00:	/* Input port */
        memset(data, s->level ^ s->polarity, len);
        break;

    case 0x01:	/* Output port */
        memset(data, s->level & ~s->direction, len);
        break;

    case 0x02:	/* Polarity inversion */
        memset(data, s->polarity, len);
        break;

    case 0x03:	/* Configuration */
        memset(data, s->direction, len);
        break;

    case 0x04:	/* Timeout */
        memset(data, s->status, len);
        break;

    case 0xff:	/* Reserved */
        memset(data, 0xff, len);
        break;

    default:
#ifdef VERBOSE
        printf("%s: unknown register %02x\n", __FUNCTION__, s->command);
#endif
        return 1;
    }
    return 0;
}

static int max7310_write(void *opaque, uint8_t *data, int len)
{
    struct max7310_s *s = (struct max7310_s *) opaque;
    uint8_t diff;
    int line;

    if (len >= 1)
        s->command = data[0];

    if (len < 2) {
#ifdef VERBOSE
        printf("%s: message too short (%i bytes)\n", __FUNCTION__, len);
#endif
        return 0;
    }

    switch (s->command) {
    case 0x01:	/* Output port */
        for (diff = (data[1] ^ s->level) & ~s->direction; diff;
                        diff &= ~(1 << line)) {
            line = ffs(diff) - 1;
            if (s->handler[line].fn)
                s->handler[line].fn(line, (data[1] >> line) & 1,
                                s->handler[line].opaque);
        }
        s->level = (s->level & s->direction) | (data[1] & ~s->direction);
        break;

    case 0x02:	/* Polarity inversion */
        s->polarity = data[1];
        break;

    case 0x03:	/* Configuration */
        s->level &= ~(s->direction ^ data[1]);
        s->direction = data[1];
        break;

    case 0x04:	/* Timeout */
        s->status = data[1];
        break;

    default:
#ifdef VERBOSE
        printf("%s: unknown register %02x\n", __FUNCTION__, s->command);
#endif
        return 1;
    }

    return 0;
}

static int max7310_tx(void *opaque, uint8_t *data, int len)
{
    struct max7310_s *s = (struct max7310_s *) opaque;
    if (len) {
        if (s->i2c_dir)
            return max7310_write(opaque, data, len);
        else
            return max7310_read(opaque, data, len);
    }

    return 0;
}

struct i2c_slave_s *max7310_init(void)
{
    struct max7310_s *s = qemu_mallocz(sizeof(struct max7310_s));
    s->i2c.opaque = s;
    s->i2c.tx = max7310_tx;
    s->i2c.start = max7310_start;

    max7310_reset(&s->i2c);
    return &s->i2c;
}

void max7310_gpio_set(struct i2c_slave_s *i2c, int line, int level)
{
    struct max7310_s *s = (struct max7310_s *) i2c->opaque;
    if (line >= sizeof(s->handler) / sizeof(*s->handler) || line  < 0)
        cpu_abort(cpu_single_env, "bad GPIO line");

    if (level)
        s->level |= s->direction & (1 << line);
    else
        s->level &= ~(s->direction & (1 << line));
}

void max7310_gpio_handler_set(struct i2c_slave_s *i2c, int line,
                gpio_handler_t handler, void *opaque)
{
    struct max7310_s *s = (struct max7310_s *) i2c->opaque;
    if (line >= sizeof(s->handler) / sizeof(*s->handler) || line  < 0)
        cpu_abort(cpu_single_env, "bad GPIO line");

    s->handler[line].fn = handler;
    s->handler[line].opaque = opaque;
}
