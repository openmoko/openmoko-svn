#include <stdio.h>
#include <string.h>

#include <libgsmd/libgsmd.h>

#define STDIN_BUF_SIZE	1024

int shell_main(struct lgsmd_handle *lgsmh)
{
	int rc;
	char buf[STDIN_BUF_SIZE+1];

	while (1) {
		rc = fscanf(stdin, "%s", buf);
		if (rc == EOF) {
			printf("EOF\n");
			return -1;
		}
		if (rc <= 0) {
			printf("NULL\n");
			continue;
		}
		printf("STR=`%s'\n", buf);
	}
}
