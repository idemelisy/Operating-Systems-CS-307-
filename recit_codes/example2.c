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
		//close(fd[1]);

		char buf[1000];
		read(fd[0], buf, 1000);
		printf("Child recieved message: %s\n", buf);
	}
	else {
		//close(fd[0]);

		char* msg = "Parent sending hello!";
		write(fd[1], msg, strlen(msg));
		wait(NULL);
        
		//close(fd[1]);
	}
	return 0;
}
