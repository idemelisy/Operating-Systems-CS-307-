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
	pipe(fd);       //to use between parent and child

    int fd2[2];     //to use between parent and grandchild
    pipe(fd2);

	int rc = fork();
	if (rc < 0) {
		fprintf(stderr, "fork failed\n");
		exit(1);
	}
	else if (rc == 0) {

        int rc2 = fork();
        if (rc2 < 0) {
            fprintf(stderr, "fork failed\n");
            exit(1);
        }
        else if (rc2 == 0 ) {
            //grandchild process
            close(fd[0]);
            close(fd[1]);
            close(fd2[1]);

            char buf[1000];
            while(read(fd2[0], buf, 1000)) {
                printf("Grandchild recieved message: %s\n", buf);
            }
            printf("grandchild terminates\n");
        }
        else{
            //child process
            close(fd[0]);
            close(fd2[0]);
            close(fd2[1]);
            char* msgC = "Child sending message!";
            write(fd[1], msgC, strlen(msgC));
            printf("child terminates\n");
        }
	}
	else {
        //parent process
		close(fd[1]);
        close(fd2[0]);

        char buf[1000];
        while(read(fd[0], buf, 1000)) {
            printf("Parent recieved message: %s\n", buf);
        }
		char* msg = "Parent sending message!";

		write(fd2[1], msg, strlen(msg));

		wait(NULL);
        wait(NULL);
        printf("parent terminates\n");
	}
	return 0;
}