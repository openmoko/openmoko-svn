/*
 * Simplified I2C(tm) bus / SMBus(tm).
 *
 * Copyright (c) 2006 Openedhand Ltd.
 * Written by Andrzej Zaborowski <balrog@zabor.org>
 *
 * This file is licensed under GNU GPL.
 */

#define I2C_MAX_MSG	4096

struct i2c_slave_s {
    uint8_t address;
    int (*tx)(void *opaque, uint8_t *data, int len);
    void (*start)(void *opaque, int dir);
    void (*stop)(void *opaque);
    void *opaque;
};

struct i2c_bus_s {
    struct i2c_slave_s *slave[0x80];
    uint8_t current;
    int dir;
};

/* I2C master - drives the clock signal on a bus.  There can be multiple
 * masters on one bus.  */
struct i2c_master_s {
    struct i2c_bus_s *bus;
    uint8_t message[I2C_MAX_MSG];
    int message_len;
    int ack;

    uint8_t data;
};

static inline int i2c_bus_start(struct i2c_bus_s *bus, uint8_t byte)
{
    struct i2c_slave_s *slave;

    bus->current = byte >> 1;
    bus->dir = byte & 1;
    slave = bus->slave[bus->current];

    return !slave;
}

static inline int i2c_start_submit(struct i2c_bus_s *bus)
{
    struct i2c_slave_s *slave = bus->slave[bus->current];
    if (!slave)
        return 1;

    if (slave->start)
        slave->start(slave->opaque, bus->dir);
    return 0;
}

static inline int i2c_stop_submit(struct i2c_bus_s *bus)
{
    struct i2c_slave_s *slave = bus->slave[bus->current];
    if (!slave)
        return 1;

    if (slave->stop)
        slave->stop(slave->opaque);
    return 0;
}

static inline int i2c_msg_submit(struct i2c_bus_s *bus,
                uint8_t message[], int len)
{
    struct i2c_slave_s *slave = bus->slave[bus->current];
    if (!slave)
        return 1;

    return slave->tx ? slave->tx(slave->opaque, message, len) : 1;
}

static inline void i2c_master_submit(struct i2c_master_s *master,
                int start, int stop)
{
    int ret = 0;

    if (!master->bus) {
        master->ack = 0;
        return;
    }

    if (start) {
        if (master->message_len)
            if (!master->bus->dir) {	/* Master --> Slave */
                i2c_start_submit(master->bus);
                ret = i2c_msg_submit(master->bus,
                                master->message, master->message_len);
                master->message_len = 0;
            }

        ret = i2c_bus_start(master->bus, master->data);
        master->message_len = 0;

        if (master->bus->dir) {		/* Master <-- Slave */
            i2c_start_submit(master->bus);
            master->message_len = 1;
            if (stop)
                i2c_msg_submit(master->bus, master->message, 0);
        }
    } else if (stop < 2) {
        if (!master->bus->dir) {	/* Master --> Slave */
            if (master->message_len < I2C_MAX_MSG)
                master->message[master->message_len ++] = master->data;
        } else {			/* Master <-- Slave */
            ret = i2c_msg_submit(master->bus,
                            master->message, master->message_len);
            master->data = master->message[0];
        }
    }

    if (stop) {
        if (!master->bus->dir) {	/* Master --> Slave */
            i2c_start_submit(master->bus);
            ret = i2c_msg_submit(master->bus,
                            master->message, master->message_len);
            master->message_len = 0;
        }

        i2c_stop_submit(master->bus);
    }

    master->ack = !ret;
}

/* Call with zero `addr' to detach.  */
static inline void i2c_slave_attach(struct i2c_bus_s *bus, uint8_t addr,
                struct i2c_slave_s *dev)
{
    if (addr >= 0x80)
        cpu_abort(cpu_single_env, "bad I2C address");

    if (dev->address)
        bus->slave[dev->address] = 0;

    dev->address = addr;

    if (dev->address)
        bus->slave[dev->address] = dev;
}

static inline void i2c_master_attach(struct i2c_bus_s *bus,
                struct i2c_master_s *dev)
{
    dev->bus = bus;
}

/* max7310.c */
struct i2c_slave_s *max7310_init(void);
void max7310_reset(struct i2c_slave_s *i2c);
void max7310_gpio_set(struct i2c_slave_s *i2c, int line, int level);
void max7310_gpio_handler_set(struct i2c_slave_s *i2c, int line,
                gpio_handler_t handler, void *opaque);

/* wm8750.c */
struct i2c_slave_s *wm8750_init(AudioState *audio);
void wm8750_reset(struct i2c_slave_s *i2c);
void wm8750_data_req_set(struct i2c_slave_s *i2c,
                void (*data_req)(void *, int, int), void *opaque);
void wm8750_dac_dat(void *opaque, uint32_t sample);
uint32_t wm8750_adc_dat(void *opaque);

/* wm8753.c */
struct i2c_slave_s *wm8753_init(AudioState *audio);
void wm8753_reset(struct i2c_slave_s *i2c);
void wm8753_data_req_set(struct i2c_slave_s *i2c,
                void (*data_req)(void *, int, int), void *opaque);
void wm8753_dac_dat(void *opaque, uint32_t sample);
uint32_t wm8753_adc_dat(void *opaque);
void wm8753_gpio_set(struct i2c_slave_s *i2c, int line, int level);
void wm8753_gpio_handler_set(struct i2c_slave_s *i2c, int line,
                gpio_handler_t handler, void *opaque);

/* pcf5060x.c */
struct i2c_slave_s *pcf5060x_init(void *pic, int irq, int tsc);
void pcf_reset(struct i2c_slave_s *i2c);
void pcf_gpo_handler_set(struct i2c_slave_s *i2c, int line,
                gpio_handler_t handler, void *opaque);
void pcf_onkey_set(struct i2c_slave_s *i2c, int level);
void pcf_exton_set(struct i2c_slave_s *i2c, int level);
