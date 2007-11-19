/*
 * Samsung S3C2410A RISC Microprocessor support (ARM920T based SoC).
 *
 * Copyright (c) 2007 OpenMoko, Inc.
 * Author: Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This code is licenced under the GNU GPL v2.
 */
#ifndef S3C_H
# define S3C_H	"s3c.h"

# include "qemu-common.h"
# include "flash.h"

/* Interrupt numbers */
# define S3C_PIC_EINT0	0
# define S3C_PIC_EINT1	1
# define S3C_PIC_EINT2	2
# define S3C_PIC_EINT3	3
# define S3C_PIC_EINT4	4
# define S3C_PIC_EINT8	5
# define S3C_PIC_WDT	9
# define S3C_PIC_TIMER0	10
# define S3C_PIC_TIMER1	11
# define S3C_PIC_TIMER2	12
# define S3C_PIC_TIMER3	13
# define S3C_PIC_TIMER4	14
# define S3C_PIC_UART2	15
# define S3C_PIC_LCD	16
# define S3C_PIC_DMA0	17
# define S3C_PIC_DMA1	18
# define S3C_PIC_DMA2	19
# define S3C_PIC_DMA3	20
# define S3C_PIC_SDI	21
# define S3C_PIC_SPI0	22
# define S3C_PIC_UART1	23
# define S3C_PIC_USBD	25
# define S3C_PIC_USBH	26
# define S3C_PIC_IIC	27
# define S3C_PIC_UART0	28
# define S3C_PIC_SPI1	29
# define S3C_PIC_RTC	30
# define S3C_PIC_ADC	31
/* "Sub source" interrupt numbers */
# define S3C_PICS_RXD0	32
# define S3C_PICS_TXD0	33
# define S3C_PICS_ERR0	34
# define S3C_PICS_RXD1	35
# define S3C_PICS_TXD1	36
# define S3C_PICS_ERR1	37
# define S3C_PICS_RXD2	38
# define S3C_PICS_TXD2	39
# define S3C_PICS_ERR2	40
# define S3C_PICS_TC	41
# define S3C_PICS_ADC	42

# define S3C_PIC_MAX	43
/* External interrupt numbers */
# define S3C_EINT(n)	((n >= 8) ? (6 << 5) | (n - 8) : (5 << 5) | n)

/* DMA requests */
# define S3C_RQ_nXDREQ0	0x00
# define S3C_RQ_nXDREQ1	0x10
# define S3C_RQ_I2SSDO	0x20
# define S3C_RQ_UART2	0x30
# define S3C_RQ_UART0	0x01
# define S3C_RQ_UART1	0x11
# define S3C_RQ_I2SSDI0	0x21
# define S3C_RQ_SDI0	0x31
# define S3C_RQ_SDI1	0x02
# define S3C_RQ_I2SSDI1	0x12
# define S3C_RQ_SDI2	0x22
# define S3C_RQ_SPI1	0x32
# define S3C_RQ_TIMER0	0x03
# define S3C_RQ_SPI0	0x13
# define S3C_RQ_TIMER1	0x23
# define S3C_RQ_TIMER2	0x33
# define S3C_RQ_USB_EP1	0x04
# define S3C_RQ_USB_EP2	0x14
# define S3C_RQ_USB_EP3	0x24
# define S3C_RQ_USB_EP4	0x34

# define S3C_RQ_MAX	0x35

/* I/O port numbers */
# define S3C_GP(b, n)	(((b) << 5) | n)
# define S3C_GPA(n)	S3C_GP(0, n)
# define S3C_GPB(n)	S3C_GP(1, n)
# define S3C_GPC(n)	S3C_GP(2, n)
# define S3C_GPD(n)	S3C_GP(3, n)
# define S3C_GPE(n)	S3C_GP(4, n)
# define S3C_GPF(n)	S3C_GP(5, n)
# define S3C_GPG(n)	S3C_GP(6, n)
# define S3C_GPH(n)	S3C_GP(7, n)
# define S3C_GP_MAX	S3C_GP(8, 0)

# define S3C_RAM_BASE	0x30000000
# define S3C_SRAM_BASE	0x40000000
# define S3C_SRAM_SIZE	0x00001000

# define S3C_PCLK_FREQ	66500000	/* Hz */
# define S3C_XTAL_FREQ	32768		/* Hz */

/* s3c2410.c */
struct s3c_pic_state_s;
struct s3c_pic_state_s *s3c_pic_init(target_phys_addr_t base,
                qemu_irq *arm_pic);
qemu_irq *s3c_pic_get(struct s3c_pic_state_s *s);

struct s3c_dma_state_s;
struct s3c_dma_state_s *s3c_dma_init(target_phys_addr_t base, qemu_irq *pic);
qemu_irq *s3c_dma_get(struct s3c_dma_state_s *s);

/* GPIO TODO: remove this out, replace with qemu_irq or sumpthin */
typedef void (*gpio_handler_t)(int line, int level, void *opaque);

struct s3c_timers_state_s;
struct s3c_timers_state_s *s3c_timers_init(target_phys_addr_t base,
                qemu_irq *pic, qemu_irq *dma);
void s3c_timers_cmp_handler_set(void *opaque, int line,
                gpio_handler_t handler, void *cmp_opaque);

struct s3c_uart_state_s;
struct s3c_uart_state_s *s3c_uart_init(target_phys_addr_t base,
                qemu_irq *irqs, qemu_irq *dma);
void s3c_uart_attach(struct s3c_uart_state_s *s, CharDriverState *chr);

struct s3c_adc_state_s;
struct s3c_adc_state_s *s3c_adc_init(target_phys_addr_t base, qemu_irq irq,
                qemu_irq tcirq);
void s3c_adc_setscale(struct s3c_adc_state_s *adc, const int m[]);

struct s3c_i2c_state_s;
struct s3c_i2c_state_s *s3c_i2c_init(target_phys_addr_t base, qemu_irq irq);
i2c_bus *s3c_i2c_bus(struct s3c_i2c_state_s *s);

struct s3c_i2s_state_s;
struct s3c_i2s_state_s *s3c_i2s_init(target_phys_addr_t base, qemu_irq *dma);

struct s3c_wdt_state_s;
struct s3c_wdt_state_s *s3c_wdt_init(target_phys_addr_t base, qemu_irq irq);

/* s3c24xx_gpio.c */
struct s3c_gpio_state_s;
struct s3c_gpio_state_s *s3c_gpio_init(target_phys_addr_t base, qemu_irq *pic);
qemu_irq *s3c_gpio_in_get(struct s3c_gpio_state_s *s);
void s3c_gpio_out_set(struct s3c_gpio_state_s *s, int line, qemu_irq handler);
void s3c_gpio_setpwrstat(struct s3c_gpio_state_s *s, int stat);
void s3c_gpio_reset(struct s3c_gpio_state_s *s);

/* s3c24xx_lcd.c */
struct s3c_lcd_state_s;
struct s3c_lcd_state_s *s3c_lcd_init(target_phys_addr_t base, DisplayState *ds,
                qemu_irq irq);
void s3c_lcd_reset(struct s3c_lcd_state_s *s);

/* s3c24xx_mmci.c */
struct s3c_mmci_state_s;
struct s3c_mmci_state_s *s3c_mmci_init(target_phys_addr_t base,
                qemu_irq irq, qemu_irq *dma);
void s3c_mmci_handlers(struct s3c_mmci_state_s *s, void *opaque,
                void (*readonly_cb)(void *, int),
                void (*coverswitch_cb)(void *, int));
void s3c_mmci_reset(struct s3c_mmci_state_s *s);

/* s3c24xx_rtc.c */
struct s3c_rtc_state_s;
struct s3c_rtc_state_s *s3c_rtc_init(target_phys_addr_t base, qemu_irq irq);
void s3c_rtc_reset(struct s3c_rtc_state_s *s);

/* s3c24xx_udc.c */
struct s3c_udc_state_s;
struct s3c_udc_state_s *s3c_udc_init(target_phys_addr_t base, qemu_irq irq,
                qemu_irq *dma);
void s3c_udc_reset(struct s3c_udc_state_s *s);

/* s3c2410.c */
struct s3c_spi_state_s;
struct s3c_spi_state_s *s3c_spi_init(target_phys_addr_t base,
                qemu_irq irq0, qemu_irq drq0, qemu_irq irq1, qemu_irq drq1,
                struct s3c_gpio_state_s *gpio);
void s3c_spi_attach(struct s3c_spi_state_s *s, int ch,
                uint8_t (*txrx)(void *opaque, uint8_t value),
                uint8_t (*btxrx)(void *opaque, uint8_t value), void *opaque);

struct s3c_state_s {
    CPUState *env;
    qemu_irq *irq;
    qemu_irq *drq;
    struct s3c_pic_state_s *pic;
    struct s3c_dma_state_s *dma;
    struct s3c_gpio_state_s *io;
    struct s3c_lcd_state_s *lcd;
    struct s3c_timers_state_s *timers;
    struct s3c_uart_state_s *uart[3];
    struct s3c_mmci_state_s *mmci;
    struct s3c_adc_state_s *adc;
    struct s3c_i2c_state_s *i2c;
    struct s3c_i2s_state_s *i2s;
    struct s3c_rtc_state_s *rtc;
    struct s3c_spi_state_s *spi;
    struct s3c_udc_state_s *udc;
    struct s3c_wdt_state_s *wdt;

    /* Memory controller */
    target_phys_addr_t mc_base;
    uint32_t mc_regs[13];

    /* NAND Flash controller */
    target_phys_addr_t nand_base;
    struct nand_flash_s *nand;
    uint16_t nfconf;
    uint8_t nfcmd;
    uint8_t nfaddr;
    struct ecc_state_s nfecc;
    int nfwp;

    /* Clock & power management */
    target_phys_addr_t clkpwr_base;
    uint32_t clkpwr_regs[6];
};

/* s3c2410.c */
struct s3c_state_s *s3c2410_init(unsigned int sdram_size, DisplayState *ds);
void s3c_nand_register(struct s3c_state_s *s, struct nand_flash_s *chip);
void s3c_nand_setwp(struct s3c_state_s *s, int wp);

struct s3c_i2s_state_s { /* XXX move to .c */
    target_phys_addr_t base;
    qemu_irq *dma;
    void (*data_req)(void *, int, int);

    uint16_t control;
    uint16_t mode;
    uint16_t prescaler;
    uint16_t fcontrol;

    int tx_en;
    int rx_en;
    int tx_len;
    int rx_len;
    void (*codec_out)(void *, uint32_t);
    uint32_t (*codec_in)(void *);
    void *opaque;

    uint16_t buffer;
    int cycle;
};

#endif	/* S3C_H */
