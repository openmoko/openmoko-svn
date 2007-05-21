/*
 * Samsung S3C24xx series LCD controller.
 *
 * Copyright (c) 2007 OpenMoko, Inc.
 * Author: Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This code is licenced under the GNU GPL v2.
 */
#include "vl.h"

typedef void (*s3c_drawfn_t)(uint32_t *, uint8_t *, const uint8_t *, int, int);

struct s3c_lcd_state_s {
    target_phys_addr_t base;
    void *irq;
    DisplayState *ds;
    s3c_drawfn_t *line_fn;

    uint32_t con[5];
    uint32_t saddr[3];
    uint32_t r;
    uint32_t g;
    uint16_t b;
    uint32_t dithmode;
    uint32_t tpal;
    uint8_t intpnd;
    uint8_t srcpnd;
    uint8_t intmsk;
    uint8_t lpcsel;

    uint16_t raw_pal[0x100];

    int width;
    int height;
    int bpp;
    int enable;
    int msb;
    int frm565;
    void *fb;
    uint32_t palette[0x100];
    int invalidate;
    int invalidatep;
    int src_width;
    int dest_width;
    s3c_drawfn_t fn;
};

static void s3c_lcd_update(struct s3c_lcd_state_s *s)
{
    s->intpnd |= s->srcpnd & ~s->intmsk;
    qemu_set_irq(s->irq, !!s->intpnd);
}

void s3c_lcd_reset(struct s3c_lcd_state_s *s)
{
    s->enable = 0;
    s->invalidate = 1;
    s->invalidatep = 1;
    s->width = -1;
    s->height = -1;

    s->con[0] = 0x00000000;
    s->con[1] = 0x00000000;
    s->con[2] = 0x00000000;
    s->con[3] = 0x00000000;
    s->con[4] = 0x00000000;
    s->saddr[0] = 0x00000000;
    s->saddr[1] = 0x00000000;
    s->saddr[2] = 0x00000000;
    s->r = 0x00000000;
    s->g = 0x00000000;
    s->b = 0x0000;
    s->dithmode = 0x00000;
    s->tpal = 0x00000000;
    s->intpnd = 0;
    s->srcpnd = 0;
    s->intmsk = 3;
    s->lpcsel = 4;
    s3c_lcd_update(s);
}

#define S3C_LCDCON1	0x00	/* LCD Control register 1 */
#define S3C_LCDCON2	0x04	/* LCD Control register 2 */
#define S3C_LCDCON3	0x08	/* LCD Control register 3 */
#define S3C_LCDCON4	0x0c	/* LCD Control register 4 */
#define S3C_LCDCON5	0x10	/* LCD Control register 5 */
#define S3C_LCDSADDR1	0x14	/* Framebuffer Start Address 1 register */
#define S3C_LCDSADDR2	0x18	/* Framebuffer Start Address 2 register */
#define S3C_LCDSADDR3	0x1c	/* Framebuffer Start Address 3 register */
#define S3C_REDLUT	0x20	/* Red Lookup Table register */
#define S3C_GREENLUT	0x24	/* Green Lookup Table register */
#define S3C_BLUELUT	0x28	/* Blue Lookup Table register */
#define S3C_DITHMODE	0x4c	/* Dithering Mode register */
#define S3C_TPAL	0x50	/* Temporary Palette register */
#define S3C_LCDINTPND	0x54	/* LCD Interrupt Pending register */
#define S3C_LCDSRCPND	0x58	/* LCD Interrupt Source Pending register */
#define S3C_LCDINTMSK	0x5c	/* LCD Interrupt Mask register */
#define S3C_LPCSEL	0x60	/* LPC3600 Control register */

#define S3C_PALETTE	0x400	/* Palette IO start offset */
#define S3C_PALETTEEND	0x5ff	/* Palette IO end offset */

static uint32_t s3c_lcd_read(void *opaque, target_phys_addr_t addr)
{
    struct s3c_lcd_state_s *s = (struct s3c_lcd_state_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case S3C_LCDCON1:
        return s->con[0];		/* XXX Return random LINECNT? */
    case S3C_LCDCON2:
        return s->con[1];
    case S3C_LCDCON3:
        return s->con[2];
    case S3C_LCDCON4:
        return s->con[3];
    case S3C_LCDCON5:
        return s->con[4];		/* XXX Return random STATUS? */
    case S3C_LCDSADDR1:
        return s->saddr[0];
    case S3C_LCDSADDR2:
        return s->saddr[1];
    case S3C_LCDSADDR3:
        return s->saddr[2];
    case S3C_REDLUT:
        return s->r;
    case S3C_GREENLUT:
        return s->g;
    case S3C_BLUELUT:
        return s->b;
    case S3C_DITHMODE:
        return s->dithmode;
    case S3C_TPAL:
        return s->tpal;
    case S3C_LCDINTPND:
        return s->intpnd;
    case S3C_LCDSRCPND:
        return s->srcpnd;
    case S3C_LCDINTMSK:
        return s->intmsk;
    case S3C_LPCSEL:
        return s->lpcsel;
    case S3C_PALETTE ... S3C_PALETTEEND:
        /* XXX assuming 16bit access */
        return s->raw_pal[(addr - S3C_PALETTE) >> 1];
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
        break;
    }
    return 0;
}

static void s3c_lcd_write(void *opaque, target_phys_addr_t addr,
                uint32_t value)
{
    struct s3c_lcd_state_s *s = (struct s3c_lcd_state_s *) opaque;
    addr -= s->base;

    switch (addr) {
    case S3C_LCDCON1:
        s->con[0] = value & 0x0003ffff;
        s->enable = value & 1;
        s->bpp = (value >> 1) & 0xf;
        s->invalidate = 1;
        s->invalidatep = 1;
        break;
    case S3C_LCDCON2:
        s->con[1] = value;
        s->invalidate = 1;
        break;
    case S3C_LCDCON3:
        s->con[2] = value;
        s->invalidate = 1;
        break;
    case S3C_LCDCON4:
        s->con[3] = value & 0xffff;
        break;
    case S3C_LCDCON5:
        s->con[4] = value & 0x1fff;
        s->frm565 = (value >> 11) & 1;
        s->msb = (value >> 12) & 1;
        s->invalidatep = 1;
        s->invalidate = 1;
        break;
    case S3C_LCDSADDR1:
        s->saddr[0] = value;
        s->fb = phys_ram_base +
                (((s->saddr[0] << 1) & 0x7ffffffe) - S3C_RAM_BASE);
        s->invalidate = 1;
        break;
    case S3C_LCDSADDR2:
        s->saddr[1] = value;
        s->invalidate = 1;
        break;
    case S3C_LCDSADDR3:
        s->saddr[2] = value;
        s->invalidate = 1;
        break;
    case S3C_REDLUT:
        s->r = value;
        s->invalidatep = 1;
        s->invalidate = 1;
        break;
    case S3C_GREENLUT:
        s->g = value;
        s->invalidatep = 1;
        s->invalidate = 1;
        break;
    case S3C_BLUELUT:
        s->b = value;
        s->invalidatep = 1;
        s->invalidate = 1;
        break;
    case S3C_DITHMODE:
        s->dithmode = value;
        break;
    case S3C_TPAL:
        s->tpal = value;
        s->invalidatep = 1;
        s->invalidate = 1;
        break;
    case S3C_LCDINTPND:
        s->intpnd = value & 3;
        break;
    case S3C_LCDSRCPND:
        s->srcpnd = value & 3;
        break;
    case S3C_LCDINTMSK:
        s->intmsk = value & 7;
        s3c_lcd_update(s);
        break;
    case S3C_LPCSEL:
        s->lpcsel = (value & 3) | 4;
        if (value & 1)
            printf("%s: attempt to enable LPC3600\n", __FUNCTION__);
        break;
    case S3C_PALETTE ... S3C_PALETTEEND:
        /* XXX assuming 16bit access */
        s->raw_pal[(addr - S3C_PALETTE) >> 1] = value;
        break;
    default:
        printf("%s: Bad register 0x%lx\n", __FUNCTION__, addr);
    }
}

static CPUReadMemoryFunc *s3c_lcd_readfn[] = {
    s3c_lcd_read,
    s3c_lcd_read,
    s3c_lcd_read,
};

static CPUWriteMemoryFunc *s3c_lcd_writefn[] = {
    s3c_lcd_write,
    s3c_lcd_write,
    s3c_lcd_write,
};

static inline void s3c_lcd_resize(struct s3c_lcd_state_s *s)
{
    int new_width, new_height;
    new_height = ((s->con[1] >> 14) & 0x3ff) + 1;
    new_width = ((s->con[2] >> 8) & 0x7ff) + 1;
    if (s->width != new_width || s->height != new_height) {
        s->width = new_width;
        s->height = new_height;
        dpy_resize(s->ds, s->width, s->height);
        s->invalidate = 1;
    }
}

static inline
uint32_t s3c_rgb_to_pixel8(unsigned int r, unsigned int g, unsigned b)
{
    return ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6);
}

static inline
uint32_t s3c_rgb_to_pixel15(unsigned int r, unsigned int g, unsigned b)
{
    return ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
}

static inline
uint32_t s3c_rgb_to_pixel16(unsigned int r, unsigned int g, unsigned b)
{
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

static inline
uint32_t s3c_rgb_to_pixel24(unsigned int r, unsigned int g, unsigned b)
{
    return (r << 16) | (g << 8) | b;
}

static inline
uint32_t s3c_rgb_to_pixel32(unsigned int r, unsigned int g, unsigned b)
{
    return (r << 16) | (g << 8) | b;
}

static inline uint32_t s3c_rgb(struct s3c_lcd_state_s *s,
                unsigned int r, unsigned int g, unsigned b)
{
    switch (s->ds->depth) {
    case 8:
        return s3c_rgb_to_pixel32(r << 2, g << 2, b << 2);
    case 15:
        return s3c_rgb_to_pixel15(r << 2, g << 2, b << 2);
    case 16:
        return s3c_rgb_to_pixel16(r << 2, g << 2, b << 2);
    case 24:
        return s3c_rgb_to_pixel24(r << 2, g << 2, b << 2);
    case 32:
        return s3c_rgb_to_pixel32(r << 2, g << 2, b << 2);
    default:
        fprintf(stderr, "%s: Bad color depth\n", __FUNCTION__);
        exit(1);
    }
}

static void s3c_lcd_palette_load(struct s3c_lcd_state_s *s)
{
    int i, n;
    switch (s->bpp) {
    case 0:
    case 8:
        n = 2;
        s->src_width = s->width >> 3;
        s->fn = s->line_fn[0];
        break;
    case 1:
    case 9:
        n = 4;
        s->src_width = s->width >> 2;
        s->fn = s->line_fn[1];
        break;
    case 2:
    case 10:
        n = 16;
        s->src_width = s->width >> 1;
        s->fn = s->line_fn[2];
        break;
    case 3:
    case 11:
        n = 256;
        s->src_width = s->width >> 0;
        s->fn = s->line_fn[3];
        break;
    case 6:
        s->src_width = (s->width * 3) >> 1;
        s->fn = s->line_fn[4];
        return;
    case 12:
        s->src_width = s->width << 1;
        if (s->frm565)
            s->fn = s->line_fn[5];
        else
            s->fn = s->line_fn[6];
        return;
    case 13:
        s->src_width = s->width << 2;
        s->fn = s->line_fn[7];
        return;
    default:
        return;
    }
    if (s->bpp & 8) {
        for (i = 0; i < n; i ++)
            if (s->frm565)
                s->palette[i] = s3c_rgb(s,
                        (s->raw_pal[i] >> 10) & 0x3e,
                        (s->raw_pal[i] >> 5) & 0x3f,
                        (s->raw_pal[i] << 1) & 0x3e);
            else
                s->palette[i] = s3c_rgb(s,
                        ((s->raw_pal[i] >> 10) & 0x3e) | (s->raw_pal[i] & 1),
                        ((s->raw_pal[i] >> 6) & 0x3e) | (s->raw_pal[i] & 1),
                        s->raw_pal[i] & 0x3f);
    } else {
        for (i = 0; i < n; i ++)
            if (n < 256)
                s->palette[i] = s3c_rgb(s,
                        ((s->r >> (i * 4)) & 0xf) << 2,
                        ((s->g >> (i * 4)) & 0xf) << 2,
                        ((s->b >> (i * 4)) & 0xf) << 2);
            else
                s->palette[i] = s3c_rgb(s,
                        ((s->r >> (((i >> 5) & 7) * 4)) & 0xf) << 2,
                        ((s->g >> (((i >> 2) & 7) * 4)) & 0xf) << 2,
                        ((s->b >> ((i & 3) * 4)) & 0xf) << 2);
    }
}

static void s3c_update_display(void *opaque)
{
    struct s3c_lcd_state_s *s = (struct s3c_lcd_state_s *) opaque;
    int y, src_width, dest_width, dirty[2], miny, maxy;
    ram_addr_t x, addr, new_addr, start, end;
    uint8_t *src, *dest;
    if (!s->enable || !s->dest_width)
        return;

    s3c_lcd_resize(s);

    if (s->invalidatep) {
        s3c_lcd_palette_load(s);
        s->invalidatep = 0;
    }

    src = s->fb;
    src_width = s->src_width;

    dest = s->ds->data;
    dest_width = s->width * s->dest_width;

    addr = (ram_addr_t) (s->fb - (void *) phys_ram_base);
    start = addr + s->height * src_width;
    end = addr;
    dirty[0] = dirty[1] = cpu_physical_memory_get_dirty(start, VGA_DIRTY_FLAG);
    miny = s->height;
    maxy = 0;
    for (y = 0; y < s->height; y ++) {
        new_addr = addr + src_width;
        for (x = addr + TARGET_PAGE_SIZE; x < new_addr;
                        x += TARGET_PAGE_SIZE) {
            dirty[1] = cpu_physical_memory_get_dirty(x, VGA_DIRTY_FLAG);
            dirty[0] |= dirty[1];
        }
        if (dirty[0] || s->invalidate) {
            s->fn(s->palette, dest, src, s->width, s->dest_width);
            maxy = y;
            end = new_addr;
            if (y < miny) {
                miny = y;
                start = addr;
            }
        }
        addr = new_addr;
        dirty[0] = dirty[1];
        src += src_width;
        dest += dest_width;
    }

    s->invalidate = 0;
    if (end > start)
        cpu_physical_memory_reset_dirty(start, end, VGA_DIRTY_FLAG);
    s->srcpnd |= (1 << 1);			/* INT_FrSyn */
    s3c_lcd_update(s);
    dpy_update(s->ds, 0, miny, s->width, maxy);
}

static void s3c_invalidate_display(void *opaque)
{
    struct s3c_lcd_state_s *s = (struct s3c_lcd_state_s *) opaque;
    s->invalidate = 1;
}

static void s3c_screen_dump(void *opaque, const char *filename)
{
    /* TODO */
}

#define BITS 8
#include "s3c24xx_template.h"
#define BITS 15
#include "s3c24xx_template.h"
#define BITS 16
#include "s3c24xx_template.h"
#define BITS 24
#include "s3c24xx_template.h"
#define BITS 32
#include "s3c24xx_template.h"

static void s3c_lcd_save(QEMUFile *f, void *opaque)
{
    struct s3c_lcd_state_s *s = (struct s3c_lcd_state_s *) opaque;
    int i;
    for (i = 0; i < 5; i ++)
        qemu_put_be32s(f, &s->con[i]);
    for (i = 0; i < 3; i ++)
        qemu_put_be32s(f, &s->saddr[i]);
    qemu_put_be32s(f, &s->r);
    qemu_put_be32s(f, &s->g);
    qemu_put_be16s(f, &s->b);
    qemu_put_be32s(f, &s->dithmode);
    qemu_put_be32s(f, &s->tpal);
    qemu_put_8s(f, &s->intpnd);
    qemu_put_8s(f, &s->srcpnd);
    qemu_put_8s(f, &s->intmsk);
    qemu_put_8s(f, &s->lpcsel);
    for (i = 0; i < 0x100; i ++)
        qemu_put_be16s(f, &s->raw_pal[i]);
}

static int s3c_lcd_load(QEMUFile *f, void *opaque, int version_id)
{
    struct s3c_lcd_state_s *s = (struct s3c_lcd_state_s *) opaque;
    int i;
    for (i = 0; i < 5; i ++)
        qemu_get_be32s(f, &s->con[i]);
    for (i = 0; i < 3; i ++)
        qemu_get_be32s(f, &s->saddr[i]);
    qemu_get_be32s(f, &s->r);
    qemu_get_be32s(f, &s->g);
    qemu_get_be16s(f, &s->b);
    qemu_get_be32s(f, &s->dithmode);
    qemu_get_be32s(f, &s->tpal);
    qemu_get_8s(f, &s->intpnd);
    qemu_get_8s(f, &s->srcpnd);
    qemu_get_8s(f, &s->intmsk);
    qemu_get_8s(f, &s->lpcsel);

    s->invalidate = 1;
    s->invalidatep = 1;
    s->width = -1;
    s->height = -1;
    s->bpp = (s->con[0] >> 1) & 0xf;
    s->enable = s->con[0] & 1;
    s->msb = (s->con[4] >> 12) & 1;
    s->frm565 = (s->con[4] >> 11) & 1;
    s->fb = phys_ram_base + (((s->saddr[0] << 1) & 0x7ffffffe) - S3C_RAM_BASE);

    for (i = 0; i < 0x100; i ++)
        qemu_get_be16s(f, &s->raw_pal[i]);

    return 0;
}

struct s3c_lcd_state_s *s3c_lcd_init(target_phys_addr_t base, DisplayState *ds,
                qemu_irq irq)
{
    int iomemtype;
    struct s3c_lcd_state_s *s = (struct s3c_lcd_state_s *)
            qemu_mallocz(sizeof(struct s3c_lcd_state_s));

    s->base = base;
    s->irq = irq;
    s->ds = ds;

    s3c_lcd_reset(s);

    graphic_console_init(ds, s3c_update_display,
                    s3c_invalidate_display, s3c_screen_dump, s);

    iomemtype = cpu_register_io_memory(0, s3c_lcd_readfn,
                    s3c_lcd_writefn, s);
    cpu_register_physical_memory(s->base, 0xffffff, iomemtype);

    register_savevm("s3c24xx_lcd", 0, 0, s3c_lcd_save, s3c_lcd_load, s);

    switch (s->ds->depth) {
    case 0:
        s->dest_width = 0;
        break;
    case 8:
        s->line_fn = s3c_draw_fn_8;
        s->dest_width = 1;
        break;
    case 15:
        s->line_fn = s3c_draw_fn_15;
        s->dest_width = 2;
        break;
    case 16:
        s->line_fn = s3c_draw_fn_16;
        s->dest_width = 2;
        break;
    case 24:
        s->line_fn = s3c_draw_fn_24;
        s->dest_width = 3;
        break;
    case 32:
        s->line_fn = s3c_draw_fn_32;
        s->dest_width = 4;
        break;
    default:
        fprintf(stderr, "%s: Bad color depth\n", __FUNCTION__);
        exit(1);
    }
    return s;
}
