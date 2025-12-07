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

	int rc = fork();
	if (rc < 0) {
		fprintf(stderr, "fork failed\n");
		exit(1);
	}
	else if (rc == 0) {
		close(fd[1]);

		char buf[1000];
		//read(fd[0], buf, 1000);
		dup2(fd[0], STDIN_FILENO);

        scanf("%s",buf);
        printf("Received: %s\n", buf);
	}
	else {
		close(fd[0]);

		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);

		printf("hello");
		fflush(stdout);
        close(STDOUT_FILENO);
		wait(NULL);
	}
	return 0;
}
