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
	unsigned int num_events;
	struct timeval tv_first;
	struct timeval tv_last;
};

static struct stats st;
static char *filename;

static int handle_event(struct input_event *evt)
{
	if (st.num_events == 1)
		memcpy(&st.tv_first, &evt->time, sizeof(st.tv_first));

	memcpy(&st.tv_last, &evt->time, sizeof(st.tv_last));

	return 0;
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

static void print_stats(void)
{
	struct timeval tv;

	time_difference(&st.tv_first, &st.tv_last, &tv);

	printf("input statiststics for device %s:\n", filename);
	printf("\t%u reads, %u events in %u.%u seconds\n",
		st.num_reads, st.num_events, tv.tv_sec, tv.tv_usec);
	printf("\t%u events/read, %u events/second\n",
		st.num_events / st.num_reads,
		st.num_events / tv.tv_sec);
}

void sighand(int signal)
{
	fprintf(stderr, "received signal %d\n", signal);
	print_stats();
	exit(0);
}

int main(int argc, char **argv)
{
	int fd;
	char buf[READBUF_SIZE];

	if (argc < 2)
		exit(2);

	filename = argv[1];

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		perror("open");
		exit(1);
	}

	signal(SIGHUP, &sighand);
	signal(SIGINT, &sighand);
	signal(SIGTERM, &sighand);

	while (1) {
		int rc, i;
		struct input_event *evt;

		rc = read(fd, buf, sizeof(buf));
		if (rc < 0) {
			perror("read");
			break;
		}
		st.num_reads++;

		for (i = 0; i < rc; i += sizeof(struct input_event)) {
			evt = (struct input_event *) (buf + i);
			st.num_events++;
			handle_event(evt);
		}
	}
}
