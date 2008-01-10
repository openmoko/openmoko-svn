/* gsmd timer code
 *
 * (C) 2000-2005 by Harald Welte <laforge@gnumonks.org>
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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include <common/linux_list.h>

#include "gsmd.h"

#include <gsmd/gsmd.h>
#include <gsmd/talloc.h>

static LLIST_HEAD(gsmd_timers);
static void *__tmr_ctx;

#if 0
static void tv_normalize(struct timeval *out)
{
	out->tv_sec += (out->tv_usec / 1000000);
	out->tv_usec = (out->tv_usec % 1000000);
}
#endif
/* subtract two struct timevals */
static int tv_sub(struct timeval *res, const struct timeval *from,
		  const struct timeval *sub)
{
	res->tv_sec = from->tv_sec - sub->tv_sec;
	res->tv_usec = from->tv_usec - sub->tv_usec;

	while (res->tv_usec < 0) {
		res->tv_sec -= 1;
		res->tv_usec += 1000000;
	}

	return 0;
}
#if 0
static int tv_add(struct timeval *res, const struct timeval *a1,
		  const struct timeval *a2)
{
	unsigned int carry;

	res->tv_sec = a1->tv_sec + a2->tv_sec;
	res->tv_usec = a1->tv_usec + a2->tv_usec;

	tv_normalize(res);
}
#endif
static int tv_later(const struct timeval *expires, const struct timeval *now)
{
	if (expires->tv_sec < now->tv_sec)
		return 0;
	else if (expires->tv_sec > now->tv_sec)
		return 1;
	else /* if (expires->tv_sec == now->tv_sec */ {
		if (expires->tv_usec >= now->tv_usec)
			return 1;
	}

	return 0;
}

static int tv_smaller(const struct timeval *t1, const struct timeval *t2)
{
	return tv_later(t2, t1);
}

static int calc_next_expiration(void)
{
	struct gsmd_timer *cur;
	struct timeval min, now, diff;
	struct itimerval iti;
	int ret;

	gettimeofday(&now, NULL);

retry:
	if (llist_empty(&gsmd_timers))
		return 0;

	llist_for_each_entry(cur, &gsmd_timers, list) {
		if (gsmd_timers.next == &cur->list)
			min = cur->expires;

		if (tv_smaller(&cur->expires, &min))
			min = cur->expires;
	}

	if (tv_sub(&diff, &min, &now) < 0) {
		/* FIXME: run expired timer callbacks */
		/* we cannot run timers from here since we might be
		 * called from register_timer() within check_n_run() */

		/* FIXME: restart with next minimum timer */
		goto retry;
	}

	/* re-set kernel timer */
	memset(&iti, 0, sizeof(iti));
	memcpy(&iti.it_value, &diff, sizeof(iti.it_value));
	ret = setitimer(ITIMER_REAL, &iti, NULL);
	if (ret < 0)
		return ret;

	return 0;
}

void gsmd_timer_check_n_run(void)
{
	struct gsmd_timer *cur, *cur2;
	struct timeval now;

	if (gettimeofday(&now, NULL) < 0)
		return;

	llist_for_each_entry_safe(cur, cur2, &gsmd_timers, list) {
		if (tv_later(&now, &cur->expires)) {
			/* fist delete it from the list of timers */
			llist_del(&cur->list);
			/* then call.  called function can re-add it */
			(cur->cb)(cur, cur->data);
		}
	}

	calc_next_expiration();
}

int gsmd_timer_init(void)
{
	__tmr_ctx = talloc_named_const(gsmd_tallocs, 1, "timers");

	return 0;
}

struct gsmd_timer *gsmd_timer_alloc(void)
{
	struct gsmd_timer *tmr;

	tmr = talloc_size(__tmr_ctx, sizeof(*tmr));
	return tmr;
}

int gsmd_timer_register(struct gsmd_timer *timer)
{
	int ret;
	struct timeval tv;

	ret = gettimeofday(&tv, NULL);
	if (ret < 0)
		return ret;

	/* convert expiration time into absoulte time */
	timer->expires.tv_sec += tv.tv_sec;
	timer->expires.tv_usec += tv.tv_usec;

	llist_add_tail(&timer->list, &gsmd_timers);

	/* re-calculate next expiration */
	calc_next_expiration();

	return 0;
}

struct gsmd_timer *gsmd_timer_create(struct timeval *expires,
				     void (*cb)(struct gsmd_timer *tmr,
						void *data),
				     void *data)
{
	struct gsmd_timer *tmr = gsmd_timer_alloc();
	int rc;

	if (!tmr)
		return NULL;

	memcpy(&tmr->expires, expires, sizeof(tmr->expires));
	tmr->cb = cb;
	tmr->data = data;

	rc = gsmd_timer_register(tmr);
	if (rc < 0) {
		talloc_free(tmr);
		return NULL;
	}

	return tmr;
}

void gsmd_timer_unregister(struct gsmd_timer *timer)
{
	llist_del(&timer->list);

	/* re-calculate next expiration */
	calc_next_expiration();
}
