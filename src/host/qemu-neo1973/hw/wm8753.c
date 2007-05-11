/*
 * WM8753 audio CODEC.
 *
 * Copyright (c) 2007 OpenMoko, Inc.
 * Author: Andrzej Zaborowski <andrew@openedhand.com>
 *
 * This file is licensed under GNU GPL v2.
 */

#include "vl.h"

#define IN_PORT_N	3
#define OUT_PORT_N	3

#define CODEC		"wm8753"

struct wm_rate_s;
struct wm8753_s {
    i2c_slave i2c;
    uint8_t i2c_data[2];
    int i2c_len;
    QEMUSoundCard card;
    SWVoiceIn *adc_voice[IN_PORT_N];
    SWVoiceOut *dac_voice[OUT_PORT_N];
    int enable;
    void (*data_req)(void *, int, int);
    void *opaque;
    uint8_t data_in[4096];
    uint8_t data_out[4096];
    int idx_in, req_in;
    int idx_out, req_out;

    SWVoiceOut **out[2];
    uint8_t outvol[7], outmute[2];
    SWVoiceIn **in[2];
    uint8_t invol[4], inmute[2];

    uint8_t adcin;
    uint8_t inctl[2];
    uint8_t recmix[2];
    uint8_t adc;

    uint8_t intpol;
    uint8_t intmask;
    uint8_t intlevel;
    uint8_t inten;
    uint8_t intinput;
    uint8_t inthigh;
    uint8_t intintr;
    uint8_t intprev;
    struct {
        qemu_irq handler;
        enum { input, low, high, intr } type;
    } line[8];
    qemu_irq *gpio_in;

    uint8_t response;

    uint32_t inmask, outmask;
    const struct wm_rate_s *rate;
    uint8_t path[4], mpath[2], format, alc, mute;
    uint16_t power[4];
};

static inline void wm8753_in_load(struct wm8753_s *s)
{
    int acquired;
    if (s->idx_in + s->req_in <= sizeof(s->data_in))
        return;
    s->idx_in = audio_MAX(0, (int) sizeof(s->data_in) - s->req_in);
    acquired = AUD_read(*s->in[0], s->data_in + s->idx_in,
                    sizeof(s->data_in) - s->idx_in);
}

static inline void wm8753_out_flush(struct wm8753_s *s)
{
    int sent;
    if (!s->idx_out)
        return;
    sent = AUD_write(*s->out[0], s->data_out, s->idx_out);
    s->idx_out = 0;
}

static void wm8753_audio_in_cb(void *opaque, int avail_b)
{
    struct wm8753_s *s = (struct wm8753_s *) opaque;
    s->req_in = avail_b;
    s->data_req(s->opaque, s->req_out >> 2, avail_b >> 2);
}

static void wm8753_audio_out_cb(void *opaque, int free_b)
{
    struct wm8753_s *s = (struct wm8753_s *) opaque;
    wm8753_out_flush(s);

    s->req_out = free_b;
    s->data_req(s->opaque, free_b >> 2, s->req_in >> 2);
}

struct wm_rate_s {
    int adc;
    int adc_hz;
    int dac;
    int dac_hz;
};

static const struct wm_rate_s wm_rate_table[] = {
    {  256, 48000,  256, 48000 },	/* SR: 00000 */
    {  384, 48000,  384, 48000 },	/* SR: 00001 */
    {  256, 48000, 1536,  8000 },	/* SR: 00010 */
    {  384, 48000, 2304,  8000 },	/* SR: 00011 */
    { 1536,  8000,  256, 48000 },	/* SR: 00100 */
    { 2304,  8000,  384, 48000 },	/* SR: 00101 */
    { 1536,  8000, 1536,  8000 },	/* SR: 00110 */
    { 2304,  8000, 2304,  8000 },	/* SR: 00111 */
    { 1024, 12000, 1024, 12000 },	/* SR: 01000 */
    { 1526, 12000, 1536, 12000 },	/* SR: 01001 */
    {  768, 16000,  768, 16000 },	/* SR: 01010 */
    { 1152, 16000, 1152, 16000 },	/* SR: 01011 */
    {  384, 32000,  384, 32000 },	/* SR: 01100 */
    {  576, 32000,  576, 32000 },	/* SR: 01101 */
    {  128, 96000,  128, 96000 },	/* SR: 01110 */
    {  192, 96000,  192, 96000 },	/* SR: 01111 */
    {  256, 44100,  256, 44100 },	/* SR: 10000 */
    {  384, 44100,  384, 44100 },	/* SR: 10001 */
    {  256, 44100, 1408,  8018 },	/* SR: 10010 */
    {  384, 44100, 2112,  8018 },	/* SR: 10011 */
    { 1408,  8018,  256, 44100 },	/* SR: 10100 */
    { 2112,  8018,  384, 44100 },	/* SR: 10101 */
    { 1408,  8018, 1408,  8018 },	/* SR: 10110 */
    { 2112,  8018, 2112,  8018 },	/* SR: 10111 */
    { 1024, 11025, 1024, 11025 },	/* SR: 11000 */
    { 1536, 11025, 1536, 11025 },	/* SR: 11001 */
    {  512, 22050,  512, 22050 },	/* SR: 11010 */
    {  768, 22050,  768, 22050 },	/* SR: 11011 */
    {  512, 24000,  512, 24000 },	/* SR: 11100 */
    {  768, 24000,  768, 24000 },	/* SR: 11101 */
    {  128, 88200,  128, 88200 },	/* SR: 11110 */
    {  192, 88200,  192, 88200 },	/* SR: 11111 */
};

static void wm8753_set_format(struct wm8753_s *s)
{
    int i;
    audsettings_t in_fmt;
    audsettings_t out_fmt;
    audsettings_t monoout_fmt;

    wm8753_out_flush(s);

    if (s->in[0] && *s->in[0])
        AUD_set_active_in(*s->in[0], 0);
    if (s->out[0] && *s->out[0])
        AUD_set_active_out(*s->out[0], 0);

    for (i = 0; i < IN_PORT_N; i ++)
        if (s->adc_voice[i]) {
            AUD_close_in(&s->card, s->adc_voice[i]);
            s->adc_voice[i] = 0;
        }
    for (i = 0; i < OUT_PORT_N; i ++)
        if (s->dac_voice[i]) {
            AUD_close_out(&s->card, s->dac_voice[i]);
            s->dac_voice[i] = 0;
        }

    if (!s->enable)
        return;

    /* Setup input */
    in_fmt.endianness = 0;
    in_fmt.nchannels = 2;
    in_fmt.freq = s->rate->adc_hz;
    in_fmt.fmt = AUD_FMT_S16;

    s->adc_voice[0] = AUD_open_in(&s->card, s->adc_voice[0],
                    CODEC ".linein", s, wm8753_audio_in_cb, &in_fmt);
    s->adc_voice[1] = AUD_open_in(&s->card, s->adc_voice[1],
                    CODEC ".rx", s, wm8753_audio_in_cb, &in_fmt);
    s->adc_voice[2] = AUD_open_in(&s->card, s->adc_voice[2],
                    CODEC ".mic", s, wm8753_audio_in_cb, &in_fmt);

    /* Setup output */
    out_fmt.endianness = 0;
    out_fmt.nchannels = 2;
    out_fmt.freq = s->rate->dac_hz;
    out_fmt.fmt = AUD_FMT_S16;
    monoout_fmt.endianness = 0;
    monoout_fmt.nchannels = 1;
    monoout_fmt.freq = s->rate->dac_hz;
    monoout_fmt.fmt = AUD_FMT_S16;

    s->dac_voice[0] = AUD_open_out(&s->card, s->dac_voice[0],
                    CODEC ".speaker", s, wm8753_audio_out_cb, &out_fmt);
    s->dac_voice[1] = AUD_open_out(&s->card, s->dac_voice[1],
                    CODEC ".headphone", s, wm8753_audio_out_cb, &out_fmt);
    /* MONOOUT is also in stereo for simplicity */
    s->dac_voice[2] = AUD_open_out(&s->card, s->dac_voice[2],
                    CODEC ".monoout", s, wm8753_audio_out_cb, &out_fmt);

    /* We should connect the left and right channels to their
     * respective inputs/outputs but we have completely no need
     * for mixing or combining paths to different ports, so we
     * connect both channels to where the left channel is routed.  */
    if (s->in[0] && *s->in[0])
        AUD_set_active_in(*s->in[0], 1);
    if (s->out[0] && *s->out[0])
        AUD_set_active_out(*s->out[0], 1);
}

static void wm8753_int_update(struct wm8753_s *s)
{
    int line;
    uint8_t level = s->inthigh;
    if (s->inten && (s->intinput & (s->intlevel ^ s->intpol) & s->intmask))
        level |= s->intintr;

    level ^= s->intprev;
    s->intprev ^= level;

    while ((line = ffs(level))) {
        line --;
        if (s->line[line].handler)
            qemu_set_irq(s->line[line].handler, (s->intprev >> line) & 1);
        level ^= 1 << line;
    }
}

static inline void wm8753_input_update(struct wm8753_s *s, int line)
{
    if (s->line[line].type == input)
        s->intinput |= 1 << line;
    else
        s->intinput &= ~(1 << line);
    if (s->line[line].type == high)
        s->inthigh |= 1 << line;
    else
        s->inthigh &= ~(1 << line);
    if (s->line[line].type == intr)
        s->intintr |= 1 << line;
    else
        s->intintr &= ~(1 << line);
}

static inline void wm8753_mask_update(struct wm8753_s *s)
{
#define R_ONLY	0x0000ffff
#define L_ONLY	0xffff0000
#define BOTH	(R_ONLY | L_ONLY)
#define NONE	(R_ONLY & L_ONLY)
    s->inmask =
            (s->inmute[0] ? R_ONLY : BOTH) &
            (s->inmute[1] ? L_ONLY : BOTH) &
            (s->mute ? NONE : BOTH);
    s->outmask =
            (s->outmute[0] ? R_ONLY : BOTH) &
            (s->outmute[1] ? L_ONLY : BOTH) &
            (s->mute ? NONE : BOTH);
}

static uint8_t wm8753_read(struct wm8753_s *s, int readsel)
{
    switch (readsel) {
    case 0:/* Device ID high */
        return 0x87;
    case 1:/* Device ID low */
        return 0x53;
    case 2:/* Device revision */
        return 0x01;
    case 3:/* Device capabilities */
        return 0x00;
    case 4:/* Device status */
        return (s->intinput & s->intlevel) | s->inthigh;
    case 5:/* Interrupt status */
        return (s->inten ? 0xff : 0x00) & s->intinput &
                (s->intlevel ^ s->intpol) & s->intmask;
#ifdef VERBOSE
    default:
        printf("%s: unknown register %02x\n", __FUNCTION__, readsel);
#endif
    }

    return 0;
}

void wm8753_reset(i2c_slave *i2c)
{
    int i;
    struct wm8753_s *s = (struct wm8753_s *) i2c;
    s->enable = 0;
    wm8753_set_format(s);

    s->idx_in = sizeof(s->data_in);
    s->req_in = 0;
    s->idx_out = 0;
    s->req_out = 0;

    s->in[0] = &s->adc_voice[0];
    s->invol[0] = 0x17;
    s->invol[1] = 0x17;
    s->invol[2] = 0xc3;
    s->invol[3] = 0xc3;
    s->out[0] = &s->dac_voice[0];
    s->outvol[0] = 0xff;
    s->outvol[1] = 0xff;
    s->outvol[2] = 0x79;
    s->outvol[3] = 0x79;
    s->outvol[4] = 0x79;
    s->outvol[5] = 0x79;
    s->inmute[0] = 0;
    s->inmute[1] = 0;
    s->outmute[0] = 0;
    s->outmute[1] = 0;

    s->adcin = 0;
    s->inctl[0] = 0;
    s->inctl[1] = 0;
    s->recmix[0] = 0x55;
    s->recmix[1] = 0x05;
    s->adc = 0;

    s->intpol = 0x00;
    s->intmask = 0x0;
    s->inten = 0;
    s->intinput = 0xff;
    s->inthigh = 0x00;
    s->intintr = 0x00;
    for (i = 0; i < 8; i ++)
        s->line[i].type = input;

    s->power[0] = 0x000;
    s->power[1] = 0x000;
    s->power[2] = 0x000;
    s->power[3] = 0x000;

    s->alc = 0;
    s->mute = 1;
    s->path[0] = 0;
    s->path[1] = 0;
    s->path[2] = 0;
    s->path[3] = 0;
    s->mpath[0] = 0;
    s->mpath[1] = 0;
    s->format = 0x0a;
    wm8753_int_update(s);
    wm8753_mask_update(s);
}

static void wm8753_event(i2c_slave *i2c, enum i2c_event event)
{
    struct wm8753_s *s = (struct wm8753_s *) i2c;

    switch (event) {
    case I2C_START_SEND:
        s->i2c_len = 0;
        break;
    case I2C_FINISH:
#ifdef VERBOSE
        if (s->i2c_len < 2)
            printf("%s: message too short (%i bytes)\n",
                            __FUNCTION__, s->i2c_len);
#endif
        break;
    default:
        break;
    }
}

#define WM8753_DAC	0x01
#define WM8753_ADC	0x02
#define WM8753_PCM	0x03
#define WM8753_HIFI	0x04
#define WM8753_IOCTL	0x05
#define WM8753_SRATE1	0x06
#define WM8753_SRATE2	0x07
#define WM8753_LDAC	0x08
#define WM8753_RDAC	0x09
#define WM8753_BASS	0x0a
#define WM8753_TREBLE	0x0b
#define WM8753_ALC1	0x0c
#define WM8753_ALC2	0x0d
#define WM8753_ALC3	0x0e
#define WM8753_NGATE	0x0f
#define WM8753_LADC	0x10
#define WM8753_RADC	0x11
#define WM8753_ADCTL1	0x12
#define WM8753_3D	0x13
#define WM8753_PWR1	0x14
#define WM8753_PWR2	0x15
#define WM8753_PWR3	0x16
#define WM8753_PWR4	0x17
#define WM8753_ID	0x18
#define WM8753_INTPOL	0x19
#define WM8753_INTEN	0x1a
#define WM8753_GPIO1	0x1b
#define WM8753_GPIO2	0x1c
#define WM8753_RESET	0x1f
#define WM8753_RECMIX1	0x20
#define WM8753_RECMIX2	0x21
#define WM8753_LOUTM1	0x22
#define WM8753_LOUTM2	0x23
#define WM8753_ROUTM1	0x24
#define WM8753_ROUTM2	0x25
#define WM8753_MOUTM1	0x26
#define WM8753_MOUTM2	0x27
#define WM8753_LOUT1V	0x28
#define WM8753_ROUT1V	0x29
#define WM8753_LOUT2V	0x2a
#define WM8753_ROUT2V	0x2b
#define WM8753_MOUTV	0x2c
#define WM8753_OUTCTL	0x2d
#define WM8753_ADCIN	0x2e
#define WM8753_INCTL1	0x2f
#define WM8753_INCTL2	0x30
#define WM8753_LINVOL	0x31
#define WM8753_RINVOL	0x32
#define WM8753_MICBIAS	0x33
#define WM8753_CLOCK	0x34
#define WM8753_PLL1CTL1	0x35
#define WM8753_PLL1CTL2	0x36
#define WM8753_PLL1CTL3	0x37
#define WM8753_PLL1CTL4	0x38
#define WM8753_PLL2CTL1	0x39
#define WM8753_PLL2CTL2	0x3a
#define WM8753_PLL2CTL3	0x3b
#define WM8753_PLL2CTL4	0x3c
#define WM8753_BIASCTL	0x3d
#define WM8753_ADCTL2	0x3f

static int wm8753_tx(i2c_slave *i2c, uint8_t data)
{
    struct wm8753_s *s = (struct wm8753_s *) i2c;
    uint8_t cmd;
    uint16_t value;

    if (s->i2c_len >= 2) {
        printf("%s: long message (%i bytes)\n", __FUNCTION__, s->i2c_len);
#ifdef VERBOSE
        return 1;
#endif
    }
    s->i2c_data[s->i2c_len ++] = data;
    if (s->i2c_len != 2)
        return 0;

    cmd = s->i2c_data[0] >> 1;
    value = ((s->i2c_data[0] << 8) | s->i2c_data[1]) & 0x1ff;

    switch (cmd) {
    case WM8753_ADCIN:		/* ADC Input Mode */
        s->adcin = value;
        wm8753_set_format(s);
        break;

    case WM8753_INCTL1:		/* ADC Input Control (1) */
        s->inctl[0] = value;
        wm8753_set_format(s);
        break;
    case WM8753_INCTL2:		/* ADC Input Control (2) */
        s->inctl[1] = value;
        wm8753_set_format(s);
        break;

    case WM8753_RECMIX1:	/* Record Mix (1) */
        s->recmix[0] = value;
        wm8753_set_format(s);
        break;
    case WM8753_RECMIX2:	/* Record Mix (2) */
        s->recmix[0] = value;
        wm8753_set_format(s);
        break;

    case WM8753_ADC:		/* Additional Control (1) */
        s->adc = value;
        wm8753_set_format(s);
        break;

    case WM8753_PWR1:		/* Power Management (1) */
        s->power[0] = value & 0x1ff;
        s->enable = ((value >> 6) & 7) == 3;	/* VMIDSEL, VREF */
        wm8753_set_format(s);
        break;
    case WM8753_PWR2:		/* Power Management (2) */
        s->power[1] = value & 0x1ff;
        wm8753_set_format(s);
        break;
    case WM8753_PWR3:		/* Power Management (2) */
        s->power[2] = value & 0x1ff;
        wm8753_set_format(s);
        break;
    case WM8753_PWR4:		/* Power Management (2) */
        s->power[3] = value & 0x1ff;
        wm8753_set_format(s);
        break;

    case WM8753_MICBIAS:
        break;

    case WM8753_LINVOL:		/* Left Channel PGA */
        s->invol[0] = value & 0x3f;		/* LINVOL */
        s->inmute[0] = (value >> 7) & 1;	/* LINMUTE */
        wm8753_mask_update(s);
        break;

    case WM8753_RINVOL:		/* Right Channel PGA */
        s->invol[1] = value & 0x3f;		/* RINVOL */
        s->inmute[1] = (value >> 7) & 1;	/* RINMUTE */
        wm8753_mask_update(s);
        break;

    case WM8753_ADCTL1:		/* Additional Control (1) */
    case WM8753_ADCTL2:		/* Additional Control (2) */
        break;

    case WM8753_LADC:		/* Left ADC Digital Volume */
        s->invol[2] = value & 0xff;		/* LADCVOL */
        break;

    case WM8753_RADC:		/* Right ADC Digital Volume */
        s->invol[3] = value & 0xff;		/* RADCVOL */
        break;

    case WM8753_ALC1:		/* ALC Control (1) */
        s->alc = (value >> 7) & 3;		/* ALCSEL */
        break;

    case WM8753_ALC2:		/* ALC Control (2) */
    case WM8753_ALC3:		/* ALC Control (3) */
    case WM8753_NGATE:		/* Noise Gate Control */
    case WM8753_3D:		/* 3D enhance */
    case WM8753_BASS:		/* Bass Control */
    case WM8753_TREBLE:		/* Treble Control */
    case WM8753_CLOCK:		/* Clock Control */
    case WM8753_BIASCTL:	/* Bias Control */
        break;

    case WM8753_LDAC:		/* Left Channel Digital Volume */
        s->outvol[0] = value & 0xff;		/* LDACVOL */
        break;

    case WM8753_RDAC:		/* Right Channel Digital Volume */
        s->outvol[1] = value & 0xff;		/* RDACVOL */
        break;

    case WM8753_DAC:		/* DAC Control */
        s->mute = (value >> 3) & 1;		/* DACMU */
        wm8753_mask_update(s);
        break;

    case WM8753_LOUTM1:		/* Left Mixer Control (1) */
        s->path[0] = (value >> 8) & 1;		/* LD2LO */
        wm8753_set_format(s);
        break;

    case WM8753_LOUTM2:		/* Left Mixer Control (2) */
        s->path[1] = (value >> 8) & 1;		/* RD2LO */
        wm8753_set_format(s);
        break;

    case WM8753_ROUTM1:		/* Right Mixer Control (1) */
        s->path[2] = (value >> 8) & 1;		/* LD2RO */
        wm8753_set_format(s);
        break;

    case WM8753_ROUTM2:		/* Right Mixer Control (2) */
        s->path[3] = (value >> 8) & 1;		/* RD2RO */
        wm8753_set_format(s);
        break;

    case WM8753_MOUTM1:		/* Mono Mixer Control (1) */
        s->mpath[0] = (value >> 8) & 1;		/* LD2MO */
        wm8753_set_format(s);
        break;

    case WM8753_MOUTM2:		/* Mono Mixer Control (2) */
        s->mpath[1] = (value >> 8) & 1;		/* RD2MO */
        wm8753_set_format(s);
        break;

    case WM8753_LOUT1V:		/* LOUT1 Volume */
        s->outvol[2] = value & 0x7f;		/* LOUT2VOL */
        break;

    case WM8753_LOUT2V:		/* LOUT2 Volume */
        s->outvol[4] = value & 0x7f;		/* LOUT2VOL */
        break;

    case WM8753_ROUT1V:		/* ROUT1 Volume */
        s->outvol[3] = value & 0x7f;		/* ROUT2VOL */
        break;

    case WM8753_ROUT2V:		/* ROUT2 Volume */
        s->outvol[5] = value & 0x7f;		/* ROUT2VOL */
        break;

    case WM8753_MOUTV:		/* MONOOUT Volume */
        s->outvol[6] = value & 0x7f;		/* MONOOUTVOL */
        break;

    case WM8753_OUTCTL:		/* Output Control */
        break;

    case WM8753_INTPOL:		/* Interrupt Polarity */
        s->intpol = value & 0xff;
        wm8753_int_update(s);
        break;

    case WM8753_INTEN:		/* Interrupt Mask */
        s->intmask = value & 0xff;
        wm8753_int_update(s);
        break;

    case WM8753_GPIO1:		/* GPIO Control (1) */
        s->inten = !!(value & 3);
        switch ((value >> 3) & 3) {
        case 0:  s->line[5].type = input; break;
        case 1:  s->line[5].type = intr; break;
        case 2:  s->line[5].type = low; break;
        case 3:  s->line[5].type = high; break;
        }
        wm8753_input_update(s, 5);
        switch ((value >> 0) & 7) {
        case 4:  s->line[4].type = low; break;
        case 5:  s->line[4].type = high; break;
        case 7:  s->line[4].type = intr; break;
        default: s->line[4].type = input; break;
        }
        wm8753_input_update(s, 4);
        wm8753_int_update(s);
        break;

    case WM8753_GPIO2:		/* GPIO Control (2) */
        switch ((value >> 6) & 7) {
        case 4:  s->line[3].type = low; break;
        case 5:  s->line[3].type = high; break;
        case 7:  s->line[3].type = intr; break;
        default: s->line[3].type = input; break;
        }
        wm8753_input_update(s, 3);
        switch ((value >> 3) & 7) {
        case 0:  s->line[2].type = low; break;
        case 1:  s->line[2].type = high; break;
        case 3:  s->line[2].type = intr; break;
        default: s->line[2].type = input; break;
        }
        wm8753_input_update(s, 2);
        switch ((value >> 0) & 7) {
        case 0:  s->line[1].type = low; break;
        case 1:  s->line[1].type = high; break;
        case 3:  s->line[1].type = intr; break;
        default: s->line[1].type = input; break;
        }
        wm8753_input_update(s, 1);
        wm8753_int_update(s);
        break;

    case WM8753_HIFI:		/* Digital Hi-Fi Audio Interface Format */
    case WM8753_IOCTL:		/* Digital Audio Interface Control */
        break;

    case WM8753_PCM:		/* Digital Voice Audio Interface Format */
        s->format = value;
        wm8753_set_format(s);
        break;

    case WM8753_ID:		/* Read Control */
        s->response = wm8753_read(s, (value >> 1) & 7);
        break;
    case 0x00:			/* Dummy */
        break;

    case WM8753_PLL1CTL1:	/* PLL1 Control (1) */
    case WM8753_PLL1CTL2:	/* PLL1 Control (2) */
    case WM8753_PLL1CTL3:	/* PLL1 Control (3) */
    case WM8753_PLL1CTL4:	/* PLL1 Control (4) */
    case WM8753_PLL2CTL1:	/* PLL2 Control (1) */
    case WM8753_PLL2CTL2:	/* PLL2 Control (2) */
    case WM8753_PLL2CTL3:	/* PLL2 Control (3) */
    case WM8753_PLL2CTL4:	/* PLL2 Control (4) */
        break;

    case WM8753_SRATE1:		/* Sample Rate Control (1) */
        s->rate = &wm_rate_table[(value >> 1) & 0x1f];
        wm8753_set_format(s);
        break;
    case WM8753_SRATE2:		/* Sample Rate Control (2) */
        break;

    case WM8753_RESET:		/* Reset */
        if (value == 0)
            wm8753_reset(&s->i2c);
        break;

#ifdef VERBOSE
    default:
        printf("%s: unknown register %02x\n", __FUNCTION__, cmd);
#endif
    }

    return 0;
}

static int wm8753_rx(i2c_slave *i2c)
{
    return 0x00;
}

static void wm8753_gpio_set(void *opaque, int line, int level)
{
    struct wm8753_s *s = (struct wm8753_s *) opaque;
    if (level)
        s->intlevel |= 1 << line;
    else
        s->intlevel &= ~(1 << line);
    wm8753_int_update(s);
}

i2c_slave *wm8753_init(i2c_bus *bus, AudioState *audio)
{
    struct wm8753_s *s = (struct wm8753_s *)
            i2c_slave_init(bus, 0, sizeof(struct wm8753_s));
    s->i2c.event = wm8753_event;
    s->i2c.recv = wm8753_rx;
    s->i2c.send = wm8753_tx;
    s->gpio_in = qemu_allocate_irqs(wm8753_gpio_set, s, 8);

    AUD_register_card(audio, CODEC, &s->card);
    wm8753_reset(&s->i2c);
    return &s->i2c;
}

void wm8753_fini(i2c_slave *i2c)
{
    struct wm8753_s *s = (struct wm8753_s *) i2c;
    wm8753_reset(&s->i2c);
    AUD_remove_card(&s->card);
    qemu_free(s);
}

qemu_irq *wm8753_gpio_in_get(i2c_slave *i2c)
{
    struct wm8753_s *s = (struct wm8753_s *) i2c;
    return s->gpio_in;
}

void wm8753_gpio_out_set(i2c_slave *i2c, int line, qemu_irq handler)
{
    struct wm8753_s *s = (struct wm8753_s *) i2c;
    if (line >= 8) {
        printf("%s: No GPIO pin %i\n", __FUNCTION__, line);
        return;
    }

    s->line[line].handler = handler;
}

void wm8753_data_req_set(i2c_slave *i2c,
                void (*data_req)(void *, int, int), void *opaque)
{
    struct wm8753_s *s = (struct wm8753_s *) i2c;
    s->data_req = data_req;
    s->opaque = opaque;
}

void wm8753_dac_dat(void *opaque, uint32_t sample)
{
    struct wm8753_s *s = (struct wm8753_s *) opaque;
    uint32_t *data = (uint32_t *) &s->data_out[s->idx_out];
    *data = sample & s->outmask;
    s->req_out -= 4;
    s->idx_out += 4;
    if (s->idx_out >= sizeof(s->data_out) || s->req_out <= 0)
        wm8753_out_flush(s);
}

uint32_t wm8753_adc_dat(void *opaque)
{
    struct wm8753_s *s = (struct wm8753_s *) opaque;
    uint32_t *data;
    if (s->idx_in >= sizeof(s->data_in))
        wm8753_in_load(s);
    data = (uint32_t *) &s->data_in[s->idx_in];
    s->req_in -= 4;
    s->idx_in += 4;
    return *data & s->inmask;
}
