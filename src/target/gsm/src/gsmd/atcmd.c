/* gsmd AT command interpreter / parser / constructor
 *
 * (C) 2006-2007 by OpenMoko, Inc.
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termios.h>

#include <sys/types.h>

#include <common/linux_list.h>

#include "gsmd.h"

#include <gsmd/ts0705.h>
#include <gsmd/ts0707.h>
#include <gsmd/gsmd.h>
#include <gsmd/atcmd.h>
#include <gsmd/talloc.h>
#include <gsmd/unsolicited.h>

static void *__atcmd_ctx;
static int remove_timer(struct gsmd_atcmd * cmd);

enum final_result_codes {
	GSMD_RESULT_OK = 0,
	GSMD_RESULT_ERR = 1,
	NUM_FINAL_RESULTS,
};

#if 0
static const char *final_results[] = {
	"OK",
	"ERROR",
	"+CME ERROR:",
	"+CMS ERROR:",
};
#endif

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

static inline void llparse_endline(struct llparser *llp)
{
	/* re-set cursor to start of buffer */
	llp->cur = llp->buf;
	llp->state = LLPARSE_STATE_IDLE;
	memset(llp->buf, 0, LLPARSE_BUF_SIZE);
}

static int llparse_byte(struct llparser *llp, char byte)
{
	int ret = 0;

	switch (llp->state) {
	case LLPARSE_STATE_IDLE:
	case LLPARSE_STATE_PROMPT_SPC:
		if (llp->flags & LGSM_ATCMD_F_EXTENDED) {
			if (byte == '\r')
				llp->state = LLPARSE_STATE_IDLE_CR;
			else if (byte == '>')
				llp->state = LLPARSE_STATE_PROMPT;
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
		else if (byte != '\r')
			llp->state = LLPARSE_STATE_ERROR;
		break;
	case LLPARSE_STATE_IDLE_LF:
		/* can we really go directly into result_cr ? */
		if (byte == '\r')
			llp->state = LLPARSE_STATE_RESULT_CR;
		else if (byte == '>')
			llp->state = LLPARSE_STATE_PROMPT;
		else {
			llp->state = LLPARSE_STATE_RESULT;
			ret = llparse_append(llp, byte);
		}
		break;
	case LLPARSE_STATE_RESULT:
		if (byte == '\r')
			llp->state = LLPARSE_STATE_RESULT_CR;
		else if ((llp->flags & LGSM_ATCMD_F_LFCR) && byte == '\n')
			llp->state = LLPARSE_STATE_RESULT_LF;
		else
			ret = llparse_append(llp, byte);
		break;
	case LLPARSE_STATE_RESULT_CR:
		if (byte == '\n')
			llparse_endline(llp);
		break;
	case LLPARSE_STATE_RESULT_LF:
		if (byte == '\r')
			llparse_endline(llp);
		break;
	case LLPARSE_STATE_PROMPT:
		if (byte == ' ')
			llp->state = LLPARSE_STATE_PROMPT_SPC;
		else {
			/* this was not a real "> " prompt */
			llparse_append(llp, '>');
			ret = llparse_append(llp, byte);
			llp->state = LLPARSE_STATE_RESULT;
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

		/* if a full SMS-style prompt was received, poke the select */
		if (llp->state == LLPARSE_STATE_PROMPT_SPC)
			llp->prompt_cb(llp->ctx);
	}

	return 0;
}

static int llparse_init(struct llparser *llp)
{
	llp->state = LLPARSE_STATE_IDLE;
	return 0;
}

#if 0
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
#endif

void atcmd_wake_pending_queue (struct gsmd *g) 
{
        g->gfd_uart.when |= GSMD_FD_WRITE;
}

void atcmd_wait_pending_queue (struct gsmd *g) 
{
        g->gfd_uart.when &= ~GSMD_FD_WRITE;
}


static int atcmd_done(struct gsmd *g, struct gsmd_atcmd *cmd, const char *buf)
{
        int rc = 0;
	/* remove timer if get respond before timeout */
	remove_timer(cmd);
        if (!cmd->cb) {
                gsmd_log(GSMD_NOTICE, "command without cb!!!\n");
        } else {
                DEBUGP("Calling final cmd->cb()\n");
                /* send final result code if there is no information
                * response in mlbuf */
                if (g->mlbuf_len) {
                        cmd->resp = (char *) g->mlbuf;
                        g->mlbuf[g->mlbuf_len] = 0;
                } else {
                        cmd->resp = (char *) buf;
                }
                rc = cmd->cb(cmd, cmd->ctx, cmd->resp);
                DEBUGP("Clearing mlbuf\n");
                memset(g->mlbuf, 0, MLPARSE_BUF_SIZE);
                g->mlbuf_len = 0;
        }
        
        /* remove from list of currently executing cmds */
        llist_del(&cmd->list);
        talloc_free(cmd);
        
        /* if we're finished with current commands, but still have pending
        * commands: we want to WRITE again */
        if (llist_empty(&g->busy_atcmds)) {
                //g->clear_to_send = 1;
                if (!llist_empty(&g->pending_atcmds)) {
                        atcmd_wake_pending_queue(g);
                }
        }
        return rc;
}

static int ml_parse(const char *buf, int len, void *ctx)
{
	struct gsmd *g = ctx;
	struct gsmd_atcmd *cmd = NULL;
	int rc = 0;
	int cme_error = 0;
	int cms_error = 0;

	DEBUGP("buf=`%s'(%d)\n", buf, len);

	/* FIXME: This needs to be part of the vendor plugin. If we receive
	 * an empty string or that 'ready' string, we need to init the modem */
	if (strlen(buf) == 0 ||
	    !strcmp(buf, "AT-Command Interpreter ready")) {
		g->interpreter_ready = 1;
		gsmd_initsettings(g);
		gsmd_alive_start(g);
		return 0;
	}

	/* responses come in order, so first response has to be for first
	 * command we sent, i.e. first entry in list */
	if (!llist_empty(&g->busy_atcmds))
		cmd = llist_entry(g->busy_atcmds.next,
				  struct gsmd_atcmd, list);

	if (cmd && !strcmp(buf, cmd->buf)) {
		DEBUGP("ignoring echo\n");
		return 0;
	}

	/* we have to differentiate between the following cases:
	 *
	 * A) an information response ("+whatever: ...")
	 *    we just pass it up the callback
	 * B) an unsolicited message ("+whateverelse: ... ")
	 *    we call the unsolicited.c handlers
	 * C) a final response ("OK", "+CME ERROR", ...)
	 *    in this case, we need to check whether we already sent some
	 *    previous data to the callback (information response).  If yes,
	 *    we do nothing.  If no, we need to call the callback.
	 * D) an intermediate response ("CONNECTED", "BUSY", "NO DIALTONE")
	 *    TBD
	 */

	if (buf[0] == '+' || strchr(g->vendorpl->ext_chars, buf[0])) {
		/* an extended response */
		const char *colon = strchr(buf, ':');
		if (!colon) {
			gsmd_log(GSMD_ERROR, "no colon in extd response `%s'\n",
				buf);
			return -EINVAL;
		}
		if (!strncmp(buf+1, "CME ERROR", 9)) {
			/* Part of Case 'C' */
			unsigned long err_nr;
			err_nr = strtoul(colon+1, NULL, 10);
			DEBUGP("error number %lu\n", err_nr);
			if (cmd)
				cmd->ret = err_nr;
			cme_error = 1;
			goto final_cb;
		}
		if (!strncmp(buf+1, "CMS ERROR", 9)) {
			/* Part of Case 'C' */
			unsigned long err_nr;
			err_nr = strtoul(colon+1, NULL, 10);
			DEBUGP("error number %lu\n", err_nr);
			if (cmd)
				cmd->ret = err_nr;
			cms_error = 1;
			goto final_cb;
		}

		if (!cmd || strncmp(buf, &cmd->buf[2], colon-buf)) {
			/* Assuming Case 'B' */
			DEBUGP("extd reply `%s' to cmd `%s', must be "
			       "unsolicited\n", buf, cmd ? &cmd->buf[2] : "NONE");
			colon++;
			if (colon > buf+len)
				colon = NULL;
			rc = unsolicited_parse(g, buf, len, colon);
			if (rc == -EAGAIN) {
				/* The parser wants one more line of
				 * input.  Wait for the next line, concatenate
				 * and resumbit to unsolicited_parse().  */
				DEBUGP("Multiline unsolicited code\n");
				g->mlbuf_len = len;
				memcpy(g->mlbuf, buf, len);
				g->mlunsolicited = 1;
				return 0;
			}
			/* if unsolicited parser didn't handle this 'reply', then we 
			 * need to continue and try harder and see what it is */
			if (rc != -ENOENT) {
				/* Case 'B' finished */
				return rc;
			}
			/* contine, not 'B' */
		}

		if (cmd) {
			if (cmd->buf[2] != '+' && strchr(g->vendorpl->ext_chars, cmd->buf[2]) == NULL) {
				gsmd_log(GSMD_ERROR, "extd reply to non-extd command?\n");
				return -EINVAL;
			}

			/* if we survive till here, it's a valid extd response
			 * to an extended command and thus Case 'A' */

			/* it might be a multiline response, so if there's a previous
			   response, send out mlbuf and start afresh with an empty buffer */
			if (g->mlbuf_len) {
				if (!cmd->cb) {
					gsmd_log(GSMD_NOTICE, "command without cb!!!\n");
				} else {
					DEBUGP("Calling cmd->cb()\n");
					cmd->resp = (char *) g->mlbuf;
					rc = cmd->cb(cmd, cmd->ctx, cmd->resp);
					DEBUGP("Clearing mlbuf\n");
					memset(g->mlbuf, 0, MLPARSE_BUF_SIZE);
				}
				g->mlbuf_len = 0;
			}

			/* the current buf will be appended to mlbuf below */
		}
	} else {
		if (!strcmp(buf, "RING") ||
		    ((g->flags & GSMD_FLAG_V0) && buf[0] == '2')) {
			/* this is the only non-extended unsolicited return
			 * code, part of Case 'B' */
			return unsolicited_parse(g, buf, len, NULL);
		}

		if (!strcmp(buf, "ERROR") ||
		    ((g->flags & GSMD_FLAG_V0) && buf[0] == '4')) {
			/* Part of Case 'C' */
			DEBUGP("unspecified error\n");
			if (cmd)
				cmd->ret = 4;
			goto final_cb;
		}

		if (!strncmp(buf, "OK", 2) ||
		    ((g->flags & GSMD_FLAG_V0) && buf[0] == '0')) {
			/* Part of Case 'C' */
			if (cmd)
				cmd->ret = 0;
			goto final_cb;
		}

		/* FIXME: handling of those special commands in response to
		 * ATD / ATA */
		if (!strncmp(buf, "NO CARRIER", 10) ||
		    ((g->flags & GSMD_FLAG_V0) && buf[0] == '3')) {
			/* Part of Case 'D' */
			goto final_cb;
		}

		if (!strncmp(buf, "BUSY", 4) ||
		    ((g->flags & GSMD_FLAG_V0) && buf[0] == '7')) {
			/* Part of Case 'D' */
			goto final_cb;
		}
	}

	/* we reach here, if we are at an information response that needs to be
	 * passed on */

	if (g->mlbuf_len)
		g->mlbuf[g->mlbuf_len ++] = '\n';
	DEBUGP("Appending buf to mlbuf\n");
	if (len > MLPARSE_BUF_SIZE - g->mlbuf_len)
		len = MLPARSE_BUF_SIZE - g->mlbuf_len;
	memcpy(g->mlbuf + g->mlbuf_len, buf, len);
	g->mlbuf_len += len;

	if (g->mlunsolicited) {
		rc = unsolicited_parse(g, (const char*) g->mlbuf, (int) g->mlbuf_len,
				strchr((const char*)g->mlbuf, ':') + 1);
		if (rc == -EAGAIN) {
			/* The parser wants one more line of
			 * input.  Wait for the next line, concatenate
			 * and resumbit to unsolicited_parse().  */
			DEBUGP("Multiline unsolicited code\n");
			return 0;
		}
		g->mlunsolicited = 0;
		g->mlbuf_len = 0;
	}
	return rc;

final_cb:
	/* if reach here, the final result code of a command has been reached */

	if (!cmd)
		return rc;

	if (cmd && cme_error)
		generate_event_from_cme(g, cmd->ret);
	
	if (cmd && cms_error)
		generate_event_from_cms(g, cmd->ret);

	return atcmd_done(g, cmd, buf);
}

/* called when the modem asked for a new line of a multiline atcmd */
static int atcmd_prompt(void *data)
{
	struct gsmd *g = data;

        atcmd_wake_pending_queue(g);
	return 0;
}

/* callback to be called if [virtual] UART has some data for us */
static int atcmd_select_cb(int fd, unsigned int what, void *data)
{
	int len, rc;
	static char rxbuf[1024];
	struct gsmd *g = data;
	char *cr;

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
	if ((what & GSMD_FD_WRITE) && g->interpreter_ready) {
		struct gsmd_atcmd *pos, *pos2;
		llist_for_each_entry_safe(pos, pos2, &g->pending_atcmds, list) {
			cr = strchr(pos->cur, '\n');
			if (cr)
				len = cr - pos->cur;
			else
				len = pos->buflen - 1;  /* assuming zero-terminated strings */
			rc = write(fd, pos->cur, len);
			if (rc == 0) {
				gsmd_log(GSMD_ERROR, "write returns 0, aborting\n");
				break;
			} else if (rc < 0) {
				gsmd_log(GSMD_ERROR, "error during write to fd %d: %d\n",
					fd, rc);
				return rc;
			}
			if (!cr || rc == len)
				rc ++;	/* Skip the \n or \0 */
			pos->buflen -= rc;
			pos->cur += rc;
			write(fd, "\r", 1);

			if (!pos->buflen) {
				/* success: create atcommand timeout timer */
				pos->timeout = pos->create_timer_func(g);  
				/* success: remove from global list of
				 * to-be-sent atcmds */
				llist_del(&pos->list);
				/* append to global list of executing atcmds */
				llist_add_tail(&pos->list, &g->busy_atcmds);

				/* we only send one cmd at the moment */
				break;
			} else {
				/* The write was short or the atcmd has more
				 * lines to send after a "> ".  */
				if (rc < len)
					return 0;
				break;
			}
		}

		/* Either pending_atcmds is empty or a command has to wait */
                atcmd_wait_pending_queue(g);
	}

	return 0;
}

static void discard_timeout(struct gsmd_timer *tmr, void *data) 
{
        struct gsmd *g=data;
        struct gsmd_atcmd *cmd=NULL;
        DEBUGP("discard time out!!\n");
        if (!llist_empty(&g->busy_atcmds)) {
                cmd = llist_entry(g->busy_atcmds.next,struct gsmd_atcmd, list);
        }
        if (!cmd) { 
                DEBUGP("ERROR!! busy_atcmds is NULL\n");
                return;
        }
        if (cmd->timeout != tmr) {
                DEBUGP("ERROR!! cmd->timeout != tmr\n");
                return;
        }

        gsmd_timer_free(cmd->timeout);
        cmd->timeout = NULL;

	if (cmd->cb) {
		cmd->resp = "Timeout";
                cmd->cb(cmd, cmd->ctx, cmd->resp);
	}
	
	// discard the timeout at command
	llist_del(&cmd->list);
	talloc_free(cmd);
	
	// pass the next pending at command
	if (llist_empty(&g->busy_atcmds) && !llist_empty(&g->pending_atcmds)) {
		atcmd_wake_pending_queue(g);
	}
}

static struct gsmd_timer * discard_timer(struct gsmd *g)
{
        
	struct timeval tv;
	tv.tv_sec = GSMD_ATCMD_TIMEOUT;
	tv.tv_usec = 0;
         
	DEBUGP("Create discard timer\n");
	
	return gsmd_timer_create(&tv, &discard_timeout, g);
}

struct gsmd_atcmd *atcmd_fill(const char *cmd, int rlen,
			      atcmd_cb_t cb, void *ctx, u_int16_t id,
			      create_timer_t ct)
{
	int buflen = strlen(cmd);
	struct gsmd_atcmd *atcmd;

	if (rlen > buflen)
		buflen = rlen;

	atcmd = talloc_size(__atcmd_ctx, sizeof(*atcmd)+ buflen);
	if (!atcmd)
		return NULL;

	atcmd->ctx = ctx;
	atcmd->id = id;
	atcmd->flags = 0;
	atcmd->ret = -255;
	atcmd->buflen = buflen;
	atcmd->buf[buflen-1] = '\0';
	atcmd->cur = atcmd->buf;
	atcmd->cb = cb;
	atcmd->resp = NULL;
	atcmd->timeout = NULL;
	strncpy(atcmd->buf, cmd, buflen-1);
	if (!ct)
		atcmd->create_timer_func = discard_timer; 
	else
		atcmd->create_timer_func = ct;

	return atcmd;
}

static int remove_timer(struct gsmd_atcmd * cmd)
{
	if (cmd->timeout) {
		DEBUGP("Get respond before timeout, remove timer!\n");
		gsmd_timer_unregister(cmd->timeout);
		gsmd_timer_free(cmd->timeout);
		cmd->timeout = NULL;
	} else {
		DEBUGP("ERROR!! The %s response comes too late!!\n", cmd->buf);
	}

	return 0;
}


/* submit an atcmd in the global queue of pending atcmds */
int atcmd_submit(struct gsmd *g, struct gsmd_atcmd *cmd)
{

	if (g->machinepl->ex_submit) {
		DEBUGP("extra-submiting command\n");
		g->machinepl->ex_submit(g);
	}
	DEBUGP("submitting command `%s'\n", cmd->buf);

	llist_add_tail(&cmd->list, &g->pending_atcmds);
	if (llist_empty(&g->busy_atcmds) && !llist_empty(&g->pending_atcmds)) {
		atcmd_wake_pending_queue(g);
	}

	return 0;
}

/* cancel a currently executing atcmd by issuing the command given as
 * parameter, usually AT ot ATH.  */
int cancel_atcmd(struct gsmd *g, struct gsmd_atcmd *cmd)
{
        struct gsmd_atcmd *cur;
        if (llist_empty(&g->busy_atcmds)) {
                return atcmd_submit(g, cmd);
        }
        cur = llist_entry(g->busy_atcmds.next, struct gsmd_atcmd, list);
        DEBUGP("cancelling command `%s' with an `%s'\n", cur->buf, cmd->buf);
        
        if (g->mlbuf_len) {
                DEBUGP("Discarding mlbuf: %.*s\n", g->mlbuf_len, g->mlbuf);
                g->mlbuf_len = 0;
        }
        
        llist_add(&cmd->list, &g->pending_atcmds);
        return atcmd_done(g, cur, "ERROR");
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
	__atcmd_ctx = talloc_named_const(gsmd_tallocs, 1, "atcmds");

	g->gfd_uart.fd = sockfd;
	g->gfd_uart.when = GSMD_FD_READ;
	g->gfd_uart.data = g;
	g->gfd_uart.cb = &atcmd_select_cb;

	INIT_LLIST_HEAD(&g->pending_atcmds);
	INIT_LLIST_HEAD(&g->busy_atcmds);

	llparse_init (&g->llp);

	g->mlbuf_len = 0;
	g->mlunsolicited = 0;
	g->alive_responded = 0;

	g->llp.cur = g->llp.buf;
	g->llp.len = sizeof(g->llp.buf);
	g->llp.cb = &ml_parse;
	g->llp.prompt_cb = &atcmd_prompt;
	g->llp.ctx = g;
	g->llp.flags = LGSM_ATCMD_F_EXTENDED;

	return gsmd_register_fd(&g->gfd_uart);
}

/* remove from the queues any command whose .ctx matches given */
int atcmd_terminate_matching(struct gsmd *g, void *ctx)
{
	int num = 0;
	struct gsmd_atcmd *cmd, *pos;

	llist_for_each_entry_safe(cmd, pos, &g->busy_atcmds, list)
		if (cmd->ctx == ctx) {
			cmd->ret = -ESHUTDOWN;
			cmd->cb(cmd, cmd->ctx, "ERROR");
			cmd->cb = NULL;
			cmd->ctx = NULL;
			num ++;
		}

	llist_for_each_entry_safe(cmd, pos, &g->pending_atcmds, list)
		if (cmd->ctx == ctx) {
			llist_del(&cmd->list);
			cmd->ret = -ESHUTDOWN;
			cmd->cb(cmd, cmd->ctx, "ERROR");
			talloc_free(cmd);
			num ++;
		}

	return num;
}
