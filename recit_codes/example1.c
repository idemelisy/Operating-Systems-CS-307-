#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	int fd[2];
	pipe(fd);

	// fd[0]: read
	// fd[1]: write

	char* msg = "hello";
	write(fd[1], msg, strlen(msg));

	char buf[1000];
	read(fd[0], buf, 1000);
	printf("Message: %s\n", buf);

	return 0;
}
