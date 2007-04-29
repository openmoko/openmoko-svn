/* select() FD handling for GSM Daemon.
 * (C) 2000-2006 by Harald Welte <laforge@gnumonks.org>
 *
 * Based on code originally written by myself for ulogd.
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

#include <fcntl.h>
#include <sys/select.h>
#include <common/linux_list.h>

#include "gsmd.h"

#include <gsmd/select.h>

static int maxfd = 0;
static LLIST_HEAD(gsmd_fds);

int gsmd_register_fd(struct gsmd_fd *fd)
{
	int flags;

	/* make FD nonblocking */
	flags = fcntl(fd->fd, F_GETFL);
	if (flags < 0)
		return -1;
	flags |= O_NONBLOCK;
	flags = fcntl(fd->fd, F_SETFL, flags);
	if (flags < 0)
		return -1;

	/* Register FD */
	if (fd->fd > maxfd)
		maxfd = fd->fd;

	llist_add_tail(&fd->list, &gsmd_fds);

	return 0;
}

void gsmd_unregister_fd(struct gsmd_fd *fd)
{
	llist_del(&fd->list);
}

int gsmd_select_main()
{
	struct gsmd_fd *ufd, *ufd2;
	fd_set readset, writeset, exceptset;
	int i;

	FD_ZERO(&readset);
	FD_ZERO(&writeset);
	FD_ZERO(&exceptset);

	/* prepare read and write fdsets */
	llist_for_each_entry(ufd, &gsmd_fds, list) {
		if (ufd->when & GSMD_FD_READ)
			FD_SET(ufd->fd, &readset);

		if (ufd->when & GSMD_FD_WRITE)
			FD_SET(ufd->fd, &writeset);

		if (ufd->when & GSMD_FD_EXCEPT)
			FD_SET(ufd->fd, &exceptset);
	}

	i = select(maxfd+1, &readset, &writeset, &exceptset, NULL);
	if (i > 0) {
		/* call registered callback functions */
		llist_for_each_entry_safe(ufd, ufd2, &gsmd_fds, list) {
			int flags = 0;

			if (FD_ISSET(ufd->fd, &readset))
				flags |= GSMD_FD_READ;

			if (FD_ISSET(ufd->fd, &writeset))
				flags |= GSMD_FD_WRITE;

			if (FD_ISSET(ufd->fd, &exceptset))
				flags |= GSMD_FD_EXCEPT;

			if (flags)
				ufd->cb(ufd->fd, flags, ufd->data);
		}
	}
	return i;
}
