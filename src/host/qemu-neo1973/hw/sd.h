/* 
 * SD Memory Card emulation.  Mostly correct for MMC too.
 *
 * Copyright (c) 2006 Andrzej Zaborowski  <balrog@zabor.org>
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

#include <vl.h>

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
#define CURRENT_STATE		(7 << 9)
#define READY_FOR_DATA		(1 << 8)
#define APP_CMD			(1 << 5)
#define AKE_SEQ_ERROR		(1 << 3)

typedef enum {
	sd_none = -1,
	sd_bc = 0,	/* broadcast -- no response */
	sd_bcr,		/* broadcast with response */
	sd_ac,		/* addressed -- no data transfer */
	sd_adtc,	/* addressed with data transfer */
} sd_cmd_type_t;

typedef enum {
	sd_nore = 0,	/* no response */
	sd_r1,		/* normal response command */
	sd_r2,		/* CID, CSD registers */
	sd_r3,		/* OCR register */
	sd_r6 = 6,	/* Published RCA response */
	sd_r1b = -1,
} sd_rsp_type_t;

struct sd_request_s {
	uint8_t cmd;
	uint32_t arg;
	uint8_t crc;
};

struct sd_response_none_s {
};

struct sd_response_r1_s {
	uint8_t cmd;
	uint32_t status;
	uint8_t crc;
};

struct sd_response_r1b_s {
	uint8_t cmd;
	uint32_t status;
	uint8_t crc;
};

struct sd_response_r2_s {
	uint16_t reg[8];
};

struct sd_response_r3_s {
	uint32_t ocr_reg;
};

struct sd_response_r6_s {
	uint8_t cmd;
	uint16_t arg;
	uint16_t status;
	uint8_t crc;
};

union sd_response_u {
	struct sd_response_none_s none;
	struct sd_response_r1_s r1;
	struct sd_response_r1b_s r1b;
	struct sd_response_r2_s r2;
	struct sd_response_r3_s r3;
	struct sd_response_r6_s r6;
};

struct sd_state_s;

struct sd_state_s *sd_init(void);
union sd_response_u sd_write_cmdline(struct sd_state_s *sd,
		struct sd_request_s req, int *rsplen);
void sd_write_datline(struct sd_state_s *sd, uint8_t value);
uint8_t sd_read_datline(struct sd_state_s *sd);
void sd_set_cb(struct sd_state_s *sd, void *opaque,
		void (*readonly_cb)(void *, int),
		void (*inserted_cb)(void *, int));

#endif	/* __hw_sd_h */
