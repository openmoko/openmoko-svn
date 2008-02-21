/* gsmd extended response parser
 *
 * (C) 2007 by OpenMoko, Inc.
 * Written by Harald Welte <laforge@openmoko.org>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */ 

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "gsmd.h"

#include <gsmd/gsmd.h>
#include <gsmd/atcmd.h>
#include <gsmd/extrsp.h>
#include <gsmd/talloc.h>

int extrsp_supports(const struct gsm_extrsp *er, int index, int value)
{
	int i;

	if (index >= er->num_tokens)
		return -EINVAL;
	if (er->tokens[index].type != GSMD_ECMD_RTT_RANGE)
		return -EINVAL;

	for (i = 0; i < er->tokens[index].u.range.num_items; i++) {
		struct gsm_extrsp_range_item *ri = 
			&er->tokens[index].u.range.item[i];
		if (value >= ri->min && value <= ri->max)
			return 1;
	}

	return 0;
}

enum parser_state {
	IDLE,
	TOKEN_STRING,
	TOKEN_STRING_LASTQUOTE,
	TOKEN_NUMERIC,
	TOKEN_RANGE,
};
		
/* parse a comma-separated list, possibly containing quote values */
struct gsm_extrsp *extrsp_parse(const void *ctx, const char *input)
{
	const char *cur = input;
	struct gsm_extrsp *er;
	enum parser_state state = IDLE;
	char buf[512];
	char *cur_buf = buf;
	struct gsm_extrsp_tok *cur_token;

	if (!input || strlen(input) == 0)
		return NULL;

	er = talloc(ctx, struct gsm_extrsp);
	if (!er)
		return NULL;
	memset(er, 0, sizeof(*er));

	while (*cur) {
		cur_token = &er->tokens[er->num_tokens];

		switch (state) {
		case IDLE:
			memset(buf, 0, sizeof(buf));
			cur_buf = buf;

			if (isblank(*cur))
				break;
			else if (*cur == '"') {
				cur_token->type = GSMD_ECMD_RTT_STRING;
				state = TOKEN_STRING;
			} else if (*cur == '(') {
				cur_token->type = GSMD_ECMD_RTT_RANGE;
				state = TOKEN_RANGE;
			} else if (isdigit(*cur)) {
				cur_token->type = GSMD_ECMD_RTT_NUMERIC;
				*cur_buf = *cur;
				cur_buf++;
				state = TOKEN_NUMERIC;
			} else if (*cur == ',') {
				cur_token->type = GSMD_ECMD_RTT_EMPTY;
				er->num_tokens++;
				state = IDLE;
			}
			break;
		case TOKEN_NUMERIC:
			if (*cur == ',') {
				/* end of number */
				cur_token->u.numeric = atoi(buf);
				er->num_tokens++;
				state = IDLE;
			} else if (isdigit(*cur)) {
				*cur_buf = *cur;
				cur_buf++;
			} else {
				/* ERORR */
			}
			break;
		case TOKEN_STRING:
			if (*cur == '"') {
				/* end of string token */
				strlcpy(cur_token->u.string, buf, GSM_EXTRSP_MAX_STRBUF);
				er->num_tokens++;
				state = TOKEN_STRING_LASTQUOTE;
			} else {
				*cur_buf = *cur;
				cur_buf++;
			}
			break;
		case TOKEN_STRING_LASTQUOTE:
			if (*cur == ',')
				state = IDLE;
			else {
				/* ERROR */
			}
			break;
		case TOKEN_RANGE:
			if (isdigit(*cur)) {
				*cur_buf = *cur;
				cur_buf++;
			} else if (*cur == '-') {
				/* previous number has completed */
				cur_token->u.range.item[cur_token->u.range.num_items].min = atoi(buf);
				memset(buf, 0, sizeof(buf));
				cur_buf = buf;
			} else if (*cur == ',') {
				/* previous number has completed */
				cur_token->u.range.item[cur_token->u.range.num_items].max = atoi(buf);
				cur_token->u.range.num_items++;
			} else if (*cur == ')') {
				/* previous number has completed */
				cur_token->u.range.item[cur_token->u.range.num_items].max = atoi(buf);
				cur_token->u.range.num_items++;
				state = TOKEN_STRING_LASTQUOTE;
				er->num_tokens++;
			} else {
				/* ERROR */
			}
			break;
		default:
			break;
		}
		cur++;
	}

	if (state == TOKEN_NUMERIC) {
		/* end of number */
		cur_token->u.numeric = atoi(buf);
		er->num_tokens++;
	}

	//extrsp_dump(er);
	return er;
}

static const char *er_tok_names[] = {
	[GSMD_ECMD_RTT_EMPTY]	= "EMPTY",
	[GSMD_ECMD_RTT_NUMERIC]	= "NUMERIC",
	[GSMD_ECMD_RTT_STRING]	= "STRING",
	[GSMD_ECMD_RTT_RANGE]	= "RANGE",
};

void extrsp_dump(const struct gsm_extrsp *er)
{
	int i;

	DEBUGP("entering(er=%p, num_tokens=%u)\n", er, er->num_tokens);
	for (i = 0; i < er->num_tokens; i++) {
		const struct gsm_extrsp_tok *tok = &er->tokens[i];
		DEBUGP("Token %u: %s: ", i, er_tok_names[tok->type]);
		switch (tok->type) {
		case GSMD_ECMD_RTT_EMPTY:
			DEBUGP("\n");
			break;
		case GSMD_ECMD_RTT_NUMERIC:
			DEBUGP("%d\n", tok->u.numeric);
			break;
		case GSMD_ECMD_RTT_STRING:
			DEBUGP("%s\n", tok->u.string);
			break;
		case GSMD_ECMD_RTT_NONE:
			break;
		case GSMD_ECMD_RTT_RANGE: {
			int j;
			for (j = 0; j < tok->u.range.num_items; j++)
				DEBUGP("%d-%d, ", tok->u.range.item[j].min,
					tok->u.range.item[j].max);
			}
			break;
		}
	}

}
