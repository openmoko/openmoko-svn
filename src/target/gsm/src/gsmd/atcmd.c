#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>

#include <common/linux_list.h>
#include "gsmd.h"
#include "atcmd.h"
#include "unsolicited.h"

/* libgsmd / gsmd AT command interpreter / parser / constructor
 * (C) 2006 by Harald Welte <hwelte@hmw-consulting.de>
 *
 * Written for First International Computer, Inc., Taiwan
 */ 

enum final_result_codes {
	GSMD_RESULT_OK = 0,
	GSMD_RESULT_ERR = 1,
	NUM_FINAL_RESULTS,
};

static const char *final_results[] = {
	"OK",
	"ERROR",
	"+CME ERROR:"
};

/* we basically implement a parse that can deal with
 * - receiving and queueing commands from higher level of libgmsd
 * - optionally combining them into one larger command (; appending)
 * - sending those commands to the TA, receiving and parsing responses
 * - calling back application on completion, or waiting synchronously
 *   for a response
 * - dealing with intermediate and unsolicited resultcodes by calling
 *   back into the application / higher levels
 */

static inline int llparse_append(struct llparser *llp, char byte)
{
	if (llp->cur < llp->buf + llp->len) {
		*(llp->cur++) = byte;
		return 0;
	} else
		return -EFBIG;
}

static int llparse_byte(struct llparser *llp, char byte)
{
	int ret = 0;

	switch (llp->state) {
	case LLPARSE_STATE_IDLE:
		if (llp->flags & LGSM_ATCMD_F_EXTENDED) {
			if (byte == '\r')
				llp->state = LLPARSE_STATE_IDLE_CR;
			else {
#ifdef STRICT
				llp->state = LLPARSE_STATE_ERROR;
#else
				llp->state = LLPARSE_STATE_RESULT;
				ret = llparse_append(llp, byte);
#endif
			}
		} else {
			llp->state = LLPARSE_STATE_RESULT;
			ret = llparse_append(llp, byte);
		}
		break;
	case LLPARSE_STATE_IDLE_CR:
		if (byte == '\n')
			llp->state = LLPARSE_STATE_IDLE_LF;
		else
			llp->state = LLPARSE_STATE_ERROR;
		break;
	case LLPARSE_STATE_IDLE_LF:
		/* can we really go directly into result_cr ? */
		if (byte == '\r')
			llp->state = LLPARSE_STATE_RESULT_CR;
		else {
			llp->state = LLPARSE_STATE_RESULT;
			ret = llparse_append(llp, byte);
		}
		break;
	case LLPARSE_STATE_RESULT:
		if (byte == '\r')
			llp->state = LLPARSE_STATE_RESULT_CR;
		else
			ret = llparse_append(llp, byte);
		break;
	case LLPARSE_STATE_RESULT_CR:
		if (byte == '\n')
			llp->state = LLPARSE_STATE_IDLE;
		break;
	case LLPARSE_STATE_ERROR:
		break;
	}

	return ret;
}

static int llparse_string(struct llparser *llp, char *buf, unsigned int len)
{
	while (len--) {
		int rc = llparse_byte(llp, *(buf++));
		if (rc < 0)
			return rc;

		/* if _after_ parsing the current byte we have finished,
		 * let the caller know that there is something to handle */
		if (llp->state == LLPARSE_STATE_RESULT_CR) {
			llp->cb(llp->buf, llp->cur - llp->buf, llp->ctx);
			/* re-set cursor to start of buffer */
			llp->cur = llp->buf;
		}
	}

	return 0;
}

/* mid-level parser */

static int parse_final_result(const char *res)
{
	int i;
	for (i = 0; i < NUM_FINAL_RESULTS; i++) {
		if (!strcmp(res, final_results[i]))
			return i;
	}
	
	return -1;
}

static int ml_parse(const char *buf, int len, void *ctx)
{
	struct gsmd *g = ctx;
	struct gsmd_atcmd *cmd;
	int final = 0;
	
	/* responses come in order, so first response has to be for first
	 * command we sent, i.e. first entry in list */
	cmd = llist_entry(g->busy_atcmds.next, struct gsmd_atcmd, list);

	if (buf[0] == '+') {
		/* an extended response */
		const char *colon = strchr(buf, ':');
		if (!colon) {
			fprintf(stderr, "no colon in extd response `%s'\n",
				buf);
			return -EINVAL;
		}
		if (cmd->buf[2] != '+') {
			fprintf(stderr, "extd reply to non-extd command?\n");
			return -EINVAL;
		}

		if (!strncmp(buf+1, "CME ERROR", 9)) {
			unsigned long err_nr;
			err_nr = strtoul(colon+1, NULL, 10);
			DEBUGP("error number %lu\n", err_nr);
			cmd->ret = err_nr;
			final = 1;
			goto final_cb;
		}

		if (strncmp(buf, &cmd->buf[2], colon-buf)) {
			DEBUGP("extd reply `%s' to cmd `%s', must be "
			       "unsolicited\n", buf, &cmd->buf[2]);
			colon++;
			if (colon > buf+len)
				colon = NULL;
			return unsolicited_parse(g, buf, len, colon);
		}

		/* if we survive till here, it's a valid extd response
		 * to an extended command */

		/* FIXME: solve multi-line responses ! */
		if (cmd->buflen < len)
			len = cmd->buflen;

		memcpy(cmd->buf, buf, len);
	} else {

		/* this is the only non-extended unsolicited return code */
		if (!strcmp(buf, "RING"))
			return unsolicited_parse(g, buf, len, NULL);

		if (!strcmp(buf, "ERROR") ||
		    ((g->flags & GSMD_FLAG_V0) && cmd->buf[0] == '4')){
			DEBUGP("unspecified error\n");
			cmd->ret = 4;
			final = 1;
			goto final_cb;
		}

		if (!strncmp(buf, "OK", 2)
		    || ((g->flags & GSMD_FLAG_V0) && cmd->buf[0] == '0')) {
			cmd->ret = 0;
			final = 1;
			goto final_cb;
		}

		/* FIXME: handling of those special commands in response to
		 * ATD / ATA */
		if (!strncmp(buf, "NO CARRIER", 10)) {
		}

		if (!strncmp(buf, "BUSY", 4)) {
		}
	}

final_cb:
	if (final) {
		/* remove from list of currently executing cmds */
		llist_del(&cmd->list);

		if (cmd->cb) {
			fprintf(stderr, "command without cb!!!\n");
			return -EINVAL;
		}
		return cmd->cb(cmd, cmd->ctx);

	}

	return 0;
}

/* callback to be called if [virtual] UART has some data for us */
static int atcmd_select_cb(int fd, unsigned int what, void *data)
{
	int len;
	static char rxbuf[1024];
	struct gsmd *g = data;

	if (what & GSMD_FD_READ) {
		while ((len = read(fd, rxbuf, sizeof(rxbuf)))) {
			int rc; 
	
			if (len < 0) {
				DEBUGP("ERROR reading from fd %u: %d (%s)\n", fd, len,
					strerror(errno));
					return len;
			}
			rc = llparse_string(&g->llp, rxbuf, len);
			if (rc < 0) {
				DEBUGP("ERROR during llparse_string: %d\n", rc);
				return rc;
			}
		}
	}

	/* write pending commands to UART */
	if (what & GSMD_FD_WRITE) {
		struct gsmd_atcmd *pos, *pos2;
		llist_for_each_entry_safe(pos, pos2, &g->pending_atcmds, list) {
			int rc = write(fd, pos->buf, strlen(pos->buf));
			if (rc == 0) {
				DEBUGP("write returns 0, aborting\n");
				break;
			} else if (rc < 0) {
				DEBUGP("error during write to fd %d: %d\n",
					fd, rc);
				return rc;
			}
			if (rc < len) {
				fprintf(stderr, "short write!!! FIXME!\n");
				exit(3);
			}
			/* success: remove from global list of to-be-sent atcmds */
			llist_del(&pos->list);
			/* append to global list of executing atcmds */
			llist_add_tail(&pos->list, &g->busy_atcmds);
		}
	}

	return 0;
}


struct gsmd_atcmd *atcmd_fill(const char *cmd, int rlen,
			      atcmd_cb_t cb, void *ctx)
{
	int buflen = strlen(cmd);
	struct gsmd_atcmd *atcmd;
	
	if (rlen > buflen)
		buflen = rlen;
	
	atcmd = malloc(sizeof(*atcmd)+ buflen);
	if (!atcmd)
		return NULL;

	atcmd->ctx = ctx;
	atcmd->flags = 0;
	atcmd->ret = -255;
	atcmd->buflen = buflen;
	atcmd->buf[buflen-1] = '\0';
	atcmd->cb = cb;
	strncpy(atcmd->buf, cmd, buflen-1);

	return atcmd;
}

/* submit an atcmd in the global queue of pending atcmds */
int atcmd_submit(struct gsmd *g, struct gsmd_atcmd *cmd)
{
	llist_add_tail(&cmd->list, &g->pending_atcmds);
	g->gfd_uart.when |= GSMD_FD_WRITE;

	return 0;
}

/* init atcmd parser */
int atcmd_init(struct gsmd *g, int sockfd)
{
	g->gfd_uart.fd = sockfd;
	g->gfd_uart.when = GSMD_FD_READ;
	g->gfd_uart.data = g;
	g->gfd_uart.cb = &atcmd_select_cb;

	g->llp.cur = g->llp.buf;
	g->llp.cb = &ml_parse;
	g->llp.ctx = g;
	g->llp.flags = LGSM_ATCMD_F_EXTENDED;

	return gsmd_register_fd(&g->gfd_uart);
}	

