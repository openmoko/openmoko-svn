#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termios.h>

#include <sys/types.h>

#include <common/linux_list.h>
#include <gsmd/ts0707.h>

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
	} else {
		DEBUGP("llp->cur too big!!!\n");
		return -EFBIG;
	}
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
		if (byte == '\n') {
			/* re-set cursor to start of buffer */
			llp->cur = llp->buf;
			llp->state = LLPARSE_STATE_IDLE;
		}
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
			/* FIXME: what to do with return value ? */
			llp->cb(llp->buf, llp->cur - llp->buf, llp->ctx);
		}
	}

	return 0;
}

static int llparse_init(struct llparser *llp)
{
	llp->state = LLPARSE_STATE_IDLE;
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
	int rc, final = 0;

	DEBUGP("buf=`%s'(%d)\n", buf, len);
	
	/* responses come in order, so first response has to be for first
	 * command we sent, i.e. first entry in list */
	cmd = llist_entry(g->busy_atcmds.next, struct gsmd_atcmd, list);
	cmd->resp = buf;

	if (buf[0] == '+') {
		/* an extended response */
		const char *colon = strchr(buf, ':');
		if (!colon) {
			gsmd_log(GSMD_ERROR, "no colon in extd response `%s'\n",
				buf);
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
			rc = unsolicited_parse(g, buf, len, colon);
			/* if unsolicited parser didn't handle this 'reply', then we 
			 * need to continue and try harder and see what it is */
			if (rc != -ENOENT)
				return rc;
		}

		if (cmd->buf[2] != '+') {
			gsmd_log(GSMD_ERROR, "extd reply to non-extd command?\n");
			return -EINVAL;
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
	if (cmd->ret != 0)
		generate_event_from_cme(g, cmd->ret);
	if (final) {
		/* remove from list of currently executing cmds */
		llist_del(&cmd->list);

		/* if we're finished with current commands, but still have pending
		 * commands: we want to WRITE again */
		if (llist_empty(&g->busy_atcmds) && !llist_empty(&g->pending_atcmds))
			g->gfd_uart.when |= GSMD_FD_WRITE;

		if (!cmd->cb) {
			gsmd_log(GSMD_NOTICE, "command without cb!!!\n");
			return -EINVAL;
		}
		return cmd->cb(cmd, cmd->ctx);
	}

	return 0;
}	

/* callback to be called if [virtual] UART has some data for us */
static int atcmd_select_cb(int fd, unsigned int what, void *data)
{
	int len, rc;
	static char rxbuf[1024];
	struct gsmd *g = data;

	if (what & GSMD_FD_READ) {
		memset(rxbuf, 0, sizeof(rxbuf));
		while ((len = read(fd, rxbuf, sizeof(rxbuf)))) {
			if (len < 0) {
				if (errno == EAGAIN)
					return 0;
				gsmd_log(GSMD_NOTICE, "ERROR reading from fd %u: %d (%s)\n", fd, len,
					strerror(errno));
					return len;
			}
			rc = llparse_string(&g->llp, rxbuf, len);
			if (rc < 0) {
				gsmd_log(GSMD_ERROR, "ERROR during llparse_string: %d\n", rc);
				return rc;
			}
		}
	}

	/* write pending commands to UART */
	if (what & GSMD_FD_WRITE) {
		struct gsmd_atcmd *pos, *pos2;
		llist_for_each_entry_safe(pos, pos2, &g->pending_atcmds, list) {
			len = strlen(pos->buf);
			rc = write(fd, pos->buf, strlen(pos->buf));
			if (rc == 0) {
				gsmd_log(GSMD_ERROR, "write returns 0, aborting\n");
				break;
			} else if (rc < 0) {
				gsmd_log(GSMD_ERROR, "error during write to fd %d: %d\n",
					fd, rc);
				return rc;
			}
			if (rc < len) {
				gsmd_log(GSMD_FATAL, "short write!!! FIXME!\n");
				exit(3);
			}
			write(fd, "\r", 1);
			/* success: remove from global list of to-be-sent atcmds */
			llist_del(&pos->list);
			/* append to global list of executing atcmds */
			llist_add_tail(&pos->list, &g->busy_atcmds);

				/* we only send one cmd at the moment */
				g->gfd_uart.when &= ~GSMD_FD_WRITE;
				break;
		}
	}

#if 0
	if (llist_empty(&g->pending_atcmds))
		g->gfd_uart.when &= ~GSMD_FD_WRITE;
#endif
		

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

void atcmd_drain(int fd)
{
	int rc;
	struct termios t;
	rc = tcflush(fd, TCIOFLUSH);
	rc = tcgetattr(fd, &t);
	DEBUGP("c_iflag = 0x%08x, c_oflag = 0x%08x, c_cflag = 0x%08x, c_lflag = 0x%08x\n",
		t.c_iflag, t.c_oflag, t.c_cflag, t.c_lflag);
	t.c_iflag = t.c_oflag = 0;
	cfmakeraw(&t);
	rc = tcsetattr(fd, TCSANOW, &t);
}

/* init atcmd parser */
int atcmd_init(struct gsmd *g, int sockfd)
{
	g->gfd_uart.fd = sockfd;
	g->gfd_uart.when = GSMD_FD_READ;
	g->gfd_uart.data = g;
	g->gfd_uart.cb = &atcmd_select_cb;

	INIT_LLIST_HEAD(&g->pending_atcmds);
	INIT_LLIST_HEAD(&g->busy_atcmds);

	llparse_init (&g->llp);

	g->llp.cur = g->llp.buf;
	g->llp.len = sizeof(g->llp.buf);
	g->llp.cb = &ml_parse;
	g->llp.ctx = g;
	g->llp.flags = LGSM_ATCMD_F_EXTENDED;

	return gsmd_register_fd(&g->gfd_uart);
}	

