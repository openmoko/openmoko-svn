/*
 * MMC bus cards emulation.  Used for MMC/SD/SDIO.
 *
 * Copyright (c) 2006-2007 Andrzej Zaborowski  <balrog@zabor.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __hw_sd_h
#define __hw_sd_h		1

#define OUT_OF_RANGE		(1 << 31)
#define ADDRESS_ERROR		(1 << 30)
#define BLOCK_LEN_ERROR		(1 << 29)
#define ERASE_SEQ_ERROR		(1 << 28)
#define ERASE_PARAM		(1 << 27)
#define WP_VIOLATION		(1 << 26)
#define CARD_IS_LOCKED		(1 << 25)
#define LOCK_UNLOCK_FAILED	(1 << 24)
#define COM_CRC_ERROR		(1 << 23)
#define ILLEGAL_COMMAND		(1 << 22)
#define CARD_ECC_FAILED		(1 << 21)
#define CC_ERROR		(1 << 20)
#define SD_ERROR		(1 << 19)
#define CID_CSD_OVERWRITE	(1 << 16)
#define WP_ERASE_SKIP		(1 << 15)
#define CARD_ECC_DISABLED	(1 << 14)
#define ERASE_RESET		(1 << 13)
#define CURRENT_STATE		(15 << 9)
#define READY_FOR_DATA		(1 << 8)
#define APP_CMD			(1 << 5)
#define AKE_SEQ_ERROR		(1 << 3)

#define CARD_STATUS_A		0x02004100
#define CARD_STATUS_B		0x00c01e00
#define CARD_STATUS_C		0xfd39a028

#define CARD_STATUS_SDIO_MASK	0x80c81e04

typedef enum {
    sd_none = -1,
    sd_bc = 0,	/* broadcast -- no response */
    sd_bcr,	/* broadcast with response */
    sd_ac,	/* addressed -- no data transfer */
    sd_adtc,	/* addressed with data transfer */
} sd_cmd_type_t;

struct sd_request_s {
    uint8_t cmd;
    uint32_t arg;
    uint8_t crc;
};

enum sd_state_e {
    sd_inactive_state = -1,	/* No-exit state */
    sd_idle_state = 0,
    sd_ready_state,
    sd_identification_state,
    sd_standby_state,
    sd_transfer_state,
    sd_sendingdata_state,
    sd_receivingdata_state,
    sd_programming_state,
    sd_disconnect_state,
    /* SDIO only */
    sd_command_state,
    sd_initialization_state = sd_idle_state,
};

typedef struct sd_card_s {
    void *opaque;
    int (*do_command)(void *opaque, struct sd_request_s *req,
                    uint8_t *response);
    void (*write_data)(void *opaque, uint8_t value);
    uint8_t (*read_data)(void *opaque);
    int (*data_ready)(void *opaque);
    qemu_irq irq;
} sd_card;

static inline int sd_do_command(struct sd_card_s *sd, struct sd_request_s *req,
                uint8_t *response)
{
    return sd->do_command(sd->opaque, req, response);
}

static inline void sd_write_data(struct sd_card_s *sd, uint8_t value)
{
    sd->write_data(sd->opaque, value);
}

static inline uint8_t sd_read_data(struct sd_card_s *sd)
{
    return sd->read_data(sd->opaque);
}

static inline int sd_data_ready(struct sd_card_s *sd)
{
    return sd->data_ready(sd->opaque);
}

/* sd.c */
struct sd_card_s *sd_init(BlockDriverState *bs, int is_spi);
void sd_set_cb(struct sd_card_s *sd, qemu_irq readonly, qemu_irq insert);

/* ssi-sd.c */
int ssi_sd_xfer(void *opaque, int val);
void *ssi_sd_init(BlockDriverState *bs);

/* ar6000.c */
struct sd_card_s *ar6k_init(NICInfo *nd);

#endif	/* __hw_sd_h */
