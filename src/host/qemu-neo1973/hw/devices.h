#ifndef QEMU_DEVICES_H
#define QEMU_DEVICES_H

/* Devices that have nowhere better to go.  */

/* smc91c111.c */
void smc91c111_init(NICInfo *, uint32_t, qemu_irq);

/* ssd0323.c */
int ssd0323_xfer_ssi(void *opaque, int data);
void *ssd0323_init(DisplayState *ds, qemu_irq *cmd_p);

/* ads7846.c */
struct ads7846_state_s;
uint32_t ads7846_read(void *opaque);
void ads7846_write(void *opaque, uint32_t value);
struct ads7846_state_s *ads7846_init(qemu_irq penirq);

/* stellaris_input.c */
void stellaris_gamepad_init(int n, qemu_irq *irq, const int *keycode);

/* jbt6k74.c */
uint8_t jbt6k74_txrx(void *opaque, uint8_t value);
uint8_t jbt6k74_btxrx(void *opaque, uint8_t value);
void *jbt6k74_init();

/* modem.c */
CharDriverState *modem_init();
void modem_enable(CharDriverState *chr, int enable);

/* gps.c */
CharDriverState *gps_antaris_serial_init();
void gps_enable(CharDriverState *chr, int enable);

#ifdef NEED_CPU_H

/* usb-ohci.c */
void usb_ohci_init_memio(target_phys_addr_t base, int num_ports, int devfn,
                qemu_irq irq);

#endif

#endif
