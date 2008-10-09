/* /dev/input/event filtering tool
 *
 * (C) 2008 by Harald Welte <laforge@openmoko.org>
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
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#include <linux/input.h>

#define READBUF_SIZE (1024*sizeof(struct input_event))

struct stats {
	unsigned int num_reads;
	unsigned int num_events_in;
	unsigned int num_events_out;
	struct timeval tv_first;
	struct timeval tv_last;
};

static struct stats st;
static char *indev_filename;
static char *out_filename;
static int outfd;

static int handle_event(struct input_event *evt)
{
	int rc;

	/* statistics */
	st.num_events_in++;
	if (st.num_events_in == 1)
		memcpy(&st.tv_first, &evt->time, sizeof(st.tv_first));
	memcpy(&st.tv_last, &evt->time, sizeof(st.tv_last));

	switch (evt->type) {
		/* supress any absolute or relative movement events */
		case EV_ABS:
		case EV_REL:
			return sizeof(*evt);
		default:
			break;
	}

	rc = write(outfd, evt, sizeof(*evt));
	st.num_events_out++;

	return rc;
}

static void time_difference(const struct timeval *start,
			    struct timeval *end,
			    struct timeval *result)
{
	if (end->tv_usec < start->tv_usec) {
		end->tv_usec += 1000000;
		end->tv_sec -= 1;
	}

	result->tv_usec = end->tv_usec - start->tv_usec;
	result->tv_sec = end->tv_sec - start->tv_sec;
}

static void reset_stats(void)
{
	memset(&st, 0, sizeof(st));
}

static void print_stats(void)
{
	struct timeval tv;

	time_difference(&st.tv_first, &st.tv_last, &tv);

	printf("input statiststics for device %s:\n", indev_filename);
	printf("\t%u reads, %u events in, %u events out, %u.%u seconds\n",
		st.num_reads, st.num_events_in, st.num_events_out,
		tv.tv_sec, tv.tv_usec);
	printf("\t%u events/read, %u events/second in, %u events/secound out\n",
		st.num_events_in / st.num_reads,
		st.num_events_in / tv.tv_sec, st.num_events_out / tv.tv_sec);
}

void sighand(int signal)
{
	fprintf(stderr, "received signal %d\n", signal);
	print_stats();

	switch (signal) {
	case SIGHUP:
		break;
	case SIGUSR1:
		reset_stats();
	case SIGINT:
	case SIGTERM:
		exit(0);
		break;
	}
}

int main(int argc, char **argv)
{
	int infd;
	char buf[READBUF_SIZE];

	if (argc != 3) {
		fprintf(stderr, "this program needs two single arguments: "
			"the /dev/input/eventX device you want to open, "
			"and the name of the FIFO to be used for output\n");
		exit(2);
	}

	indev_filename = argv[1];
	out_filename = argv[2];

	infd = open(indev_filename, O_RDONLY);
	if (infd < 0) {
		perror("open input");
		exit(1);
	}

	outfd = open(out_filename, O_WRONLY);
	if (outfd < 0) {
		perror("open output");
		close (infd);
		exit(1);
	}

	signal(SIGHUP, &sighand);
	signal(SIGINT, &sighand);
	signal(SIGTERM, &sighand);
	signal(SIGUSR1, &sighand);
	signal(SIGPIPE, SIG_IGN);

	while (1) {
		int rc, i;
		struct input_event *evt;

		rc = read(infd, buf, sizeof(buf));
		if (rc < 0) {
			perror("read");
			break;
		}
		st.num_reads++;

		for (i = 0; i < rc; i += sizeof(struct input_event)) {
			evt = (struct input_event *) (buf + i);
			rc = handle_event(evt);
			if (rc < 0) {
				perror("handle_event");
				exit(3);
			}
			if (rc < sizeof(struct input_event)) {
				fprintf(stderr, "short write\n");
				close(outfd);
				/* we do a blocking reopen */
				outfd = open(out_filename, O_WRONLY);
				if (outfd < 0) {
					perror("reopen output");
					close (infd);
					exit(4);
				}
			}
		}
	}
}
