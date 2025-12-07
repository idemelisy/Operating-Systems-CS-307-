#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int main(int argc, char *argv[])
{
    printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    printf("hello world (pid:%d)\n", (int) getpid());
    int rc = fork();
    if (rc < 0) {
        // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // child (new process)
        printf("hello, I am child (pid:%d)\n", (int) getpid());
        rc = fork();
        if (rc > 0) {
            // still the same child.
            wait(NULL);
            printf("I am still the same child after fork and parent of %d (pid:%d)\n", rc, (int) getpid());
        } else if (!rc) { // rc==0
            // grand-child
            printf("hello, I am grand-child (pid:%d)\n", (int) getpid());
        } else {
            fprintf(stderr, "fork failed\n");
            exit(1);
        }
    } else {
        // parent goes down this path (original process)
        wait(NULL);
        printf("hello, I am parent of %d (pid:%d)\n",
	       rc, (int) getpid());

    }
    printf("Everybody prints this at the end (pid:%d)\n",(int) getpid());
    sleep(1);

    return 0;
}
