#ifndef _GSMD_EXTRSP_H
#define _GSMD_EXTRSP_H

/* how many tokens (CSV items) can an extended response have, max */
#define GSM_EXTRSP_MAX_TOKENS	16

/* how many individual sub-ranges can one range contain */
#define GSM_EXTRSP_MAX_RANGES	16

/* how many character we are going to store in string buffer */
#define GSM_EXTRSP_MAX_STRBUF	64

struct gsm_extrsp_range_item {
	int min;
	int max;
};

enum gsm_extrsp_tok_type {
	GSMD_ECMD_RTT_NONE,
	GSMD_ECMD_RTT_EMPTY,
	GSMD_ECMD_RTT_NUMERIC,
	GSMD_ECMD_RTT_STRING,
	GSMD_ECMD_RTT_RANGE,
};

struct gsm_extrsp_tok {
	enum gsm_extrsp_tok_type type;
	union {
		struct {
			struct gsm_extrsp_range_item item[GSM_EXTRSP_MAX_RANGES];
			int num_items;
		} range;
		char string[GSM_EXTRSP_MAX_STRBUF];
		int numeric;
	} u;
};

struct gsm_extrsp {
	unsigned int num_tokens;
	struct gsm_extrsp_tok tokens[GSM_EXTRSP_MAX_TOKENS];
};

extern int extrsp_supports(const struct gsm_extrsp *er, int index, int value);
extern struct gsm_extrsp *extrsp_parse(const void *ctx, const char *input);
extern void extrsp_dump(const struct gsm_extrsp *er);

#endif
